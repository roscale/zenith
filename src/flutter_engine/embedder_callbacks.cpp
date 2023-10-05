#include "embedder_callbacks.hpp"
#include "embedder_state.hpp"
#include "server.hpp"
#include "rect.hpp"

extern "C" {
#include <GLES3/gl3.h>
#define static
#include <wlr/render/egl.h>
#include <wlr/render/gles2.h>
#undef static
}

#include <cassert>
#include <iostream>
#include <sys/eventfd.h>
#include <unistd.h>

auto flutter_make_current(void* userdata) -> bool {
	auto* state = static_cast<EmbedderState*>(userdata);
	return wlr_egl_make_current(state->flutter_gl_context);
}

auto flutter_clear_current(void* userdata) -> bool {
	auto* state = static_cast<EmbedderState*>(userdata);
	return wlr_egl_unset_current(state->flutter_gl_context);
}

auto flutter_fbo_callback(void* userdata) -> uint32_t {
	ZenithServer* server = ZenithServer::instance();
	wlr_gles2_buffer* gles2_buffer = server->output->swap_chain->start_write();
	return gles2_buffer->fbo;
}

auto flutter_present(void* userdata, const FlutterPresentInfo* present_info) -> bool {
	// Wait for the buffer to finish rendering before we commit it to the screen.
	glFinish();

	ZenithServer* server = ZenithServer::instance();
	array_view<FlutterRect> frame_damage(present_info->frame_damage.damage, present_info->frame_damage.num_rects);
	server->output->swap_chain->end_write(frame_damage);
	return true;
}

void flutter_vsync_callback(void* userdata, intptr_t baton) {
	auto* state = static_cast<EmbedderState*>(userdata);
	state->set_baton(baton);
}

auto flutter_gl_external_texture_frame_callback(void* userdata, int64_t texture_id, size_t width, size_t height,
                                                FlutterOpenGLTexture* texture_out) -> bool {
	ZenithServer* server = ZenithServer::instance();
	channel<wlr_gles2_texture_attribs> texture_attribs{};

	server->callable_queue.enqueue([texture_id, &texture_attribs]() {
		ZenithServer* server = ZenithServer::instance();

		auto find_client_chain = [&]() -> std::shared_ptr<SurfaceBufferChain<wlr_buffer>> {
			auto it = server->surface_buffer_chains.find(texture_id);
			if (it != server->surface_buffer_chains.end()) {
				size_t view_id = server->surface_id_per_texture_id.at(texture_id);
				auto& texture_ids = server->texture_ids_per_surface_id.at(view_id);
				// Always keep 2 textures alive for tiling animations.
				// The current one and the previous one.
				while (!texture_ids.empty()) {
					if (texture_ids.at(0) == (size_t) texture_id) {
						break;
					}
					if (texture_ids.size() >= 2 && texture_ids.at(1) == (size_t) texture_id) {
						break;
					}

					size_t first = *texture_ids.begin();
					texture_ids.erase(texture_ids.begin());
					assert(server->surface_buffer_chains.count(first) == 1);
					server->surface_buffer_chains.erase(first);
					assert(server->surface_id_per_texture_id.count(first) == 1);
					server->surface_id_per_texture_id.erase(first);

					server->embedder_state->unregister_external_texture(first);
				}
				return it->second;
			}
			return nullptr;
		};

		const auto& client_chain = find_client_chain();

		if (client_chain == nullptr) {
			texture_attribs.write({});
			return;
		}

		wlr_buffer* buffer = client_chain->start_read();
		assert(buffer != nullptr); // TODO: crashes here

		wlr_texture* texture = wlr_client_buffer_get(buffer)->texture;
		assert(texture != nullptr);

		wlr_gles2_texture_attribs attribs{};
		wlr_gles2_texture_get_attribs(texture, &attribs);
		texture_attribs.write(attribs);
		return;
	});

	wlr_gles2_texture_attribs attribs = texture_attribs.read();
	if (attribs.tex == 0) {
		return false;
	}

	texture_out->target = attribs.target;
	texture_out->format = GL_RGBA8;
	texture_out->name = attribs.tex;
	texture_out->user_data = reinterpret_cast<void*>(texture_id);

	texture_out->destruction_callback = [](void* user_data) {
		auto texture_id = reinterpret_cast<int64_t>(user_data);

		auto* server = ZenithServer::instance();
		server->callable_queue.enqueue([server, texture_id]() {
			auto it = server->surface_buffer_chains.find(texture_id);
			if (it != server->surface_buffer_chains.end()) {
				it->second->end_read();
			}
		});
	};

	return true;
}

void flutter_platform_message_callback(const FlutterPlatformMessage* message, void* userdata) {
	auto* state = static_cast<EmbedderState*>(userdata);

	if (message->struct_size != sizeof(FlutterPlatformMessage)) {
		std::cerr << "ERROR: Invalid message size received. Expected: "
		          << sizeof(FlutterPlatformMessage) << " but received "
		          << message->struct_size << std::endl;
		return;
	}

	state->message_dispatcher.HandleMessage(*message, [] {}, [] {});
}

auto flutter_make_resource_current(void* userdata) -> bool {
	auto* state = static_cast<EmbedderState*>(userdata);
	return wlr_egl_make_current(state->flutter_resource_gl_context);
}

/*
 * The default rendering is done upside down for some reason.
 * This flips the rendering on the x-axis.
 */
auto flutter_surface_transformation(void* data) -> FlutterTransformation {
	channel<double> height_chan = {};
	auto* server = ZenithServer::instance();
	server->callable_queue.enqueue([server, &height_chan] {
		height_chan.write(server->output->wlr_output->height);
	});
	double height = height_chan.read();

	return FlutterTransformation{
		  1.0, 0.0, 0.0, 0.0, -1.0, height, 0.0, 0.0, 1.0,
	};
}

void flutter_populate_existing_damage(void* user_data, intptr_t fbo_id, FlutterDamage* existing_damage) {
	ZenithServer* server = ZenithServer::instance();
	array_view<FlutterRect> damage_regions = server->output->swap_chain->get_damage_regions();

	// FIXME: I think Flutter's partial repaint mechanism is not completely implemented.
	// It only works with one rectangle. If I give it more than one, it just ignores them.
	// For this reason we just combine all damage regions into one rectangle.
	static auto union_region = FlutterRect{};
	if (damage_regions.size() > 0) {
		union_region = damage_regions[0];
		for (size_t i = 1; i < damage_regions.size(); i++) {
			union_region = rect_union(union_region, damage_regions[i]);
		}
	}

	existing_damage->struct_size = sizeof(FlutterDamage);
	existing_damage->num_rects = 1;
	existing_damage->damage = &union_region;
}
