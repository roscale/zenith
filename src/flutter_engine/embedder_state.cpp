#include "embedder_state.hpp"
#include "embedder_callbacks.hpp"
#include "platform_api.hpp"
#include "standard_method_codec.h"
#include "json_message_codec.h"
#include <filesystem>
#include <thread>
#include <unistd.h>
#include "server.hpp"
#include "cursor_image_mapping.hpp"
#include "json_method_codec.h"
#include "keyboard_helpers.hpp"

using namespace flutter;

EmbedderState::EmbedderState(wlr_egl* flutter_gl_context, wlr_egl* flutter_resource_gl_context)
	  : message_dispatcher(&messenger),
	    flutter_gl_context(flutter_gl_context),
	    flutter_resource_gl_context(flutter_resource_gl_context),
	    event_loop(wl_event_loop_create()) {

	messenger.SetMessageDispatcher(&message_dispatcher);
}

void EmbedderState::run_engine() {
	embedder_thread = std::thread([this]() {
		auto callable_queue_fn = [](int fd, uint32_t mask, void* data) {
			auto* queue = static_cast<CallableQueue*>(data);
			return (int) queue->execute();
		};

		wl_event_loop_add_fd(event_loop, callable_queue.get_fd(), WL_EVENT_READABLE, callable_queue_fn,
		                     &callable_queue);

		while (true) {
			wl_event_loop_dispatch(event_loop, -1);
		}
	});

	callable_queue.enqueue([this] {
		configure_and_run_engine();

		platform_task_runner.set_engine(engine);

		wl_event_loop_add_fd(event_loop, platform_task_runner.timer_fd, WL_EVENT_READABLE,
		                     [](int fd, uint32_t mask, void* data) -> int {
			                     auto* state = static_cast<EmbedderState*>(data);

			                     uint64_t expiration_count;
			                     read(state->platform_task_runner.timer_fd, &expiration_count, sizeof(uint64_t));

			                     state->platform_task_runner.execute_expired_tasks();
			                     return 0;
		                     }, this);

		register_platform_api();
	});
}

void EmbedderState::configure_and_run_engine() {
	/*
	 * Configure renderer, task runners, and project args.
	 */
	FlutterRendererConfig config = {};
	config.type = kOpenGL;
	config.open_gl.struct_size = sizeof(FlutterOpenGLRendererConfig);
	config.open_gl.make_current = flutter_make_current;
	config.open_gl.clear_current = flutter_clear_current;
	config.open_gl.present_with_info = flutter_present;
	config.open_gl.fbo_callback = flutter_fbo_callback;
	config.open_gl.gl_external_texture_frame_callback = flutter_gl_external_texture_frame_callback;
	config.open_gl.make_resource_current = flutter_make_resource_current;
	// Force Flutter to ask for another framebuffer any time it wants to draw.
	// We have a swapchain, so when Flutter finishes a frame, it's going to be shown on the screen.
	// If Flutter begins to render the next frame in the same framebuffer, visual glitches are going
	// to appear on the screen.
	// This makes sure that when Flutter asks for another framebuffer, we are providing one that is
	// unused.
	config.open_gl.fbo_reset_after_present = true;
	config.open_gl.surface_transformation = flutter_surface_transformation;
	config.open_gl.populate_existing_damage = flutter_populate_existing_damage;

	FlutterTaskRunnerDescription platform_task_runner_description{};
	platform_task_runner_description.struct_size = sizeof(FlutterTaskRunnerDescription);
	platform_task_runner_description.user_data = this;
	platform_task_runner_description.identifier = 1;
	platform_task_runner_description.runs_task_on_current_thread_callback = [](void* userdata) {
		auto* flutter_engine_state = static_cast<EmbedderState*>(userdata);
		return std::this_thread::get_id() == flutter_engine_state->embedder_thread.get_id();
	};
	platform_task_runner_description.post_task_callback = [](FlutterTask task, uint64_t target_time, void* userdata) {
		auto* flutter_engine_state = static_cast<EmbedderState*>(userdata);
		flutter_engine_state->platform_task_runner.add_task(target_time, task);
	};

	FlutterCustomTaskRunners task_runners{};
	task_runners.struct_size = sizeof(FlutterCustomTaskRunners);
	task_runners.platform_task_runner = &platform_task_runner_description;

	std::filesystem::path executable_path = std::filesystem::canonical("/proc/self/exe");
	// Normally, the observatory is started on a random port with some random auth code, but we will
	// set up a fixed URL in order to automate attaching a debugger which requires observatory's URL.
	std::array command_line_argv = {
		  executable_path.c_str(),
		  "--observatory-port=12345",
		  "--disable-service-auth-codes",
	};

	std::filesystem::path executable_directory = executable_path.parent_path();

	FlutterProjectArgs args = {};
	args.struct_size = sizeof(FlutterProjectArgs);
	std::filesystem::path assets_path = executable_directory / "data" / "flutter_assets";
	args.assets_path = assets_path.c_str();
	std::filesystem::path icu_data_path = executable_directory / "data" / "icudtl.dat";
	args.icu_data_path = icu_data_path.c_str();
	args.platform_message_callback = flutter_platform_message_callback;
	args.vsync_callback = flutter_vsync_callback;
	args.custom_task_runners = &task_runners;
	args.command_line_argc = command_line_argv.size();
	args.command_line_argv = command_line_argv.data();

#ifndef DEBUG
	/*
	 * Profile and release modes have to run in AOT mode.
	 */
	FlutterEngineAOTDataSource data_source = {};
	data_source.type = kFlutterEngineAOTDataSourceTypeElfPath;
	std::filesystem::path elf_path = executable_directory / "lib" / "libapp.so";
	data_source.elf_path = elf_path.c_str();

	FlutterEngineAOTData aot_data;
	FlutterEngineCreateAOTData(&data_source, &aot_data);

	args.aot_data = aot_data;
#endif

	int result = FlutterEngineRun(FLUTTER_ENGINE_VERSION, &config, &args, this, &engine);
	assert(result == kSuccess && engine != nullptr);
}

void EmbedderState::register_platform_api() {
	messenger.SetEngine(engine);

	key_event_channel = std::make_unique<flutter::BasicMessageChannel<rapidjson::Document>>(
		  &messenger,
		  "flutter/keyevent",
		  &flutter::JsonMessageCodec::GetInstance()
	);

	auto& standard_codec = flutter::StandardMethodCodec::GetInstance();
	platform_method_channel = std::make_unique<flutter::MethodChannel<>>(
		  &messenger,
		  "platform",
		  &standard_codec
	);

	ZenithServer* server = ZenithServer::instance();
	platform_method_channel->SetMethodCallHandler(
		  [server](const flutter::MethodCall<>& call, std::unique_ptr<flutter::MethodResult<>> result) {
			  const std::string& method_name = call.method_name();
			  if (method_name == "startup_complete") {
				  startup_complete(server, call, std::move(result));
			  } else if (method_name == "activate_window") {
				  activate_window(server, call, std::move(result));
			  } else if (method_name == "pointer_hover") {
				  pointer_hover(server, call, std::move(result));
			  } else if (method_name == "pointer_exit") {
				  pointer_exit(server, call, std::move(result));
			  } else if (method_name == "close_window") {
				  close_window(server, call, std::move(result));
			  } else if (method_name == "resize_window") {
				  resize_window(server, call, std::move(result));
			  } else if (method_name == "maximize_window") {
				  maximize_window(server, call, std::move(result));
			  } else if (method_name == "unregister_view_texture") {
				  unregister_view_texture(server, call, std::move(result));
			  } else if (method_name == "mouse_button_event") {
				  mouse_button_event(server, call, std::move(result));
			  } else if (method_name == "change_window_visibility") {
				  change_window_visibility(server, call, std::move(result));
			  } else if (method_name == "touch_down") {
				  touch_down(server, call, std::move(result));
			  } else if (method_name == "touch_motion") {
				  touch_motion(server, call, std::move(result));
			  } else if (method_name == "touch_up") {
				  touch_up(server, call, std::move(result));
			  } else if (method_name == "touch_cancel") {
				  touch_cancel(server, call, std::move(result));
			  } else if (method_name == "insert_text") {
				  insert_text(server, call, std::move(result));
			  } else if (method_name == "emulate_keycode") {
				  emulate_keycode(server, call, std::move(result));
			  } else if (method_name == "open_windows_maximized") {
				  open_windows_maximized(server, call, std::move(result));
			  } else if (method_name == "maximized_window_size") {
				  maximized_window_size(server, call, std::move(result));
			  } else if (method_name == "unlock_session") {
				  unlock_session(server, call, std::move(result));
			  } else if (method_name == "enable_display") {
				  enable_display(server, call, std::move(result));
			  } else if (method_name == "hide_keyboard") {
				  hide_keyboard(server, call, std::move(result));
			  } else {
				  result->Error("method_does_not_exist", "Method " + method_name + " does not exist");
			  }
		  }
	);

	// https://api.flutter.dev/flutter/services/SystemChannels/mouseCursor-constant.html
	mouse_cursor_method_channel = std::make_unique<flutter::MethodChannel<>>(
		  &messenger,
		  "flutter/mousecursor",
		  &standard_codec
	);

	server = ZenithServer::instance();
	mouse_cursor_method_channel->SetMethodCallHandler(
		  [server](const flutter::MethodCall<>& call, std::unique_ptr<flutter::MethodResult<>> result) {
			  const std::string& method_name = call.method_name();
			  if (method_name == "activateSystemCursor") {
				  flutter::EncodableMap args = std::get<flutter::EncodableMap>(call.arguments()[0]);
				  [[maybe_unused]] auto device = std::get<int32_t>(args[flutter::EncodableValue("device")]);
				  auto kind = std::get<std::string>(args[flutter::EncodableValue("kind")]);

				  server->callable_queue.enqueue([server, kind] {
					  if (server->pointer != nullptr) {
						  auto iter = g_cursor_image_mapping.find(kind);
						  const char* cursor = "default";
						  if (iter != g_cursor_image_mapping.end()) {
							  cursor = iter->second;
						  }
						  wlr_xcursor_manager_set_cursor_image(server->pointer->cursor_mgr, cursor,
						                                       server->pointer->cursor);
					  }
				  });
				  result->Success();
			  } else {
				  result->Error("method_does_not_exist", "Method " + method_name + " does not exist");
			  }
		  }
	);

	text_input_method_channel = std::make_unique<flutter::MethodChannel<rapidjson::Document>>(
		  &messenger,
		  "flutter/textinput",
		  &JsonMethodCodec::GetInstance()
	);
	text_input_method_channel->SetMethodCallHandler(
		  [this](const flutter::MethodCall<rapidjson::Document>& call,
		         std::unique_ptr<flutter::MethodResult<rapidjson::Document>> result) {
			  const std::string& method_name = call.method_name();

			  if (method_name == "TextInput.setClient") {
				  auto array = call.arguments()->GetArray();
				  int64_t transaction = array[0].GetInt64();

				  auto config = array[1].GetObject();
				  std::string input_action = {config["inputAction"].GetString(),
				                              config["inputAction"].GetStringLength()};

				  current_text_input_client.emplace(*text_input_method_channel, transaction, input_action);

			  } else if (method_name == "TextInput.setEditingState") {
				  auto object = call.arguments()->GetObject();

				  std::string text = {object["text"].GetString(), object["text"].GetStringLength()};
				  flutter::TextRange composing_range(
					    object["composingBase"].GetInt64(),
					    object["composingExtent"].GetInt64()
				  );
				  flutter::TextRange selection(
					    object["selectionBase"].GetInt64(),
					    object["selectionExtent"].GetInt64()
				  );
				  current_text_input_client->set_editing_state(text, composing_range, selection);

			  } else if (method_name == "TextInput.clearClient") {
				  current_text_input_client = std::nullopt;

			  } else if (method_name == "TextInput.show") {
				  send_text_input_event(0, TextInputEventType::enable);

			  } else if (method_name == "TextInput.hide") {
				  send_text_input_event(0, TextInputEventType::disable);
			  }

			  result->Success();
		  }
	);
}

void EmbedderState::send_window_metrics(FlutterWindowMetricsEvent metrics) {
	callable_queue.enqueue([this, metrics] {
		FlutterEngineSendWindowMetricsEvent(engine, &metrics);
	});
}

void
EmbedderState::on_vsync(intptr_t baton, uint64_t frame_start_time_nanos, uint64_t frame_target_time_nanos) {
	callable_queue.enqueue([this, baton, frame_start_time_nanos, frame_target_time_nanos] {
		FlutterEngineOnVsync(engine, baton, frame_start_time_nanos, frame_target_time_nanos);
	});
}

void EmbedderState::send_pointer_event(FlutterPointerEvent pointer_event) {
	callable_queue.enqueue([this, pointer_event] {
		FlutterEngineSendPointerEvent(engine, &pointer_event, 1);
	});
}

void EmbedderState::send_key_event(const KeyboardKeyEventMessage& message) {
	callable_queue.enqueue([message, this] {
		rapidjson::Document json;
		json.SetObject();
		json.AddMember("keymap", "linux", json.GetAllocator());
		// Even thought Flutter only understands GTK keycodes, these are essentially the same as
		// xkbcommon keycodes.
		// https://gitlab.gnome.org/GNOME/gtk/-/blob/gtk-3-24/gdk/wayland/gdkkeys-wayland.c#L179
		json.AddMember("toolkit", "gtk", json.GetAllocator());
		json.AddMember("scanCode", message.scan_code, json.GetAllocator());
		json.AddMember("keyCode", message.keysym, json.GetAllocator());
		// https://github.com/flutter/engine/blob/2a8ac1e0ca2535a6af17dde3530d277ecd601543/shell/platform/linux/fl_keyboard_manager.cc#L96
		if (message.keysym < 128) {
			json.AddMember("specifiedLogicalKey", message.keysym, json.GetAllocator());
		}
		json.AddMember("modifiers", message.modifiers, json.GetAllocator());
		// Normally I would also set `unicodeScalarValues`, but I don't anticipate using this feature.
		// https://github.com/flutter/flutter/blob/b8f7f1f986/packages/flutter/lib/src/services/raw_keyboard_linux.dart#L55

		switch (message.state) {
			case KeyboardKeyState::press: {
				json.AddMember("type", "keydown", json.GetAllocator());
				key_repeater.schedule_repeat(message);
				break;
			}
			case KeyboardKeyState::repeat: {
				// Multiple keydown events means repeated key.
				json.AddMember("type", "keydown", json.GetAllocator());
				break;
			}
			case KeyboardKeyState::release: {
				json.AddMember("type", "keyup", json.GetAllocator());
				key_repeater.cancel_repeat();
				break;
			}
		}

		key_event_channel->Send(json, [this, message](const uint8_t* reply, size_t reply_size) {
			auto obj = flutter::JsonMessageCodec::GetInstance().DecodeMessage(reply, reply_size);
			if (obj == nullptr) {
				return;
			}
			auto& handled_object = obj->GetObject()["handled"];
			bool handled = handled_object.GetBool();
			if (handled) {
				return;
			}

			if (!current_text_input_client.has_value() && message.state != KeyboardKeyState::repeat) {
				ZenithServer::instance()->callable_queue.enqueue([message] {
					uint32_t wlr_state = message.state == KeyboardKeyState::press ? WL_KEYBOARD_KEY_STATE_PRESSED
					                                                              : WL_KEYBOARD_KEY_STATE_RELEASED;

					wlr_seat_keyboard_notify_key(ZenithServer::instance()->seat, current_time_milliseconds(),
					                             message.keycode,
					                             wlr_state);
				});
				return;
			}

			if (!current_text_input_client.has_value() || message.state == KeyboardKeyState::release) {
				return;
			}

			switch (message.keysym) {
				case XKB_KEY_Return:
				case XKB_KEY_KP_Enter: {
					current_text_input_client->enter();
					break;
				}
				default: {
					static char buffer[64];
					size_t bytes_written = xkb_keysym_to_utf8(message.keysym, buffer, sizeof(buffer));
					if (bytes_written != 0) {
						current_text_input_client->add_text(buffer);
					}
				}
			}
		});
	});
}

void EmbedderState::register_external_texture(int64_t id) {
	callable_queue.enqueue([this, id] {
		FlutterEngineRegisterExternalTexture(engine, id);
	});
}

void EmbedderState::unregister_external_texture(int64_t id) {
	callable_queue.enqueue([this, id] {
		FlutterEngineUnregisterExternalTexture(engine, id);
	});
}

void EmbedderState::mark_external_texture_frame_available(int64_t id) {
	callable_queue.enqueue([this, id] {
		FlutterEngineMarkExternalTextureFrameAvailable(engine, id);
	});
}

std::optional<intptr_t> EmbedderState::get_baton() {
	std::scoped_lock lock(baton_mutex);
	if (new_baton) {
		new_baton = false;
		return baton;
	}
	return std::nullopt;
}

void EmbedderState::set_baton(intptr_t p_baton) {
	std::scoped_lock lock(baton_mutex);
	assert(new_baton == false);
	baton = p_baton;
	new_baton = true;
}

FlutterEngine EmbedderState::get_engine() const {
	return engine;
}

void EmbedderState::commit_surface(const SurfaceCommitMessage& message) {
	callable_queue.enqueue([message, this] {
		EncodableList subsurfaces_below{};
		for (const SubsurfaceParentState& state: message.subsurfaces_below) {
			subsurfaces_below.emplace_back(EncodableMap{
				  {EncodableValue("id"), EncodableValue(state.id)},
				  {EncodableValue("x"),  EncodableValue(state.x)},
				  {EncodableValue("y"),  EncodableValue(state.y)},
			});
		}
		EncodableList subsurfaces_above{};
		for (const SubsurfaceParentState& state: message.subsurfaces_above) {
			subsurfaces_above.emplace_back(EncodableMap{
				  {EncodableValue("id"), EncodableValue(state.id)},
				  {EncodableValue("x"),  EncodableValue(state.x)},
				  {EncodableValue("y"),  EncodableValue(state.y)},
			});
		}

		auto map = EncodableMap{
			  {EncodableValue("view_id"), EncodableValue((int64_t) message.view_id)},
			  {EncodableValue("surface"), EncodableValue(EncodableMap{
					{EncodableValue("role"),              EncodableValue((int64_t) message.surface.role)},
					{EncodableValue("textureId"),         EncodableValue(message.surface.texture_id)},
					{EncodableValue("x"),                 EncodableValue(message.surface.x)},
					{EncodableValue("y"),                 EncodableValue(message.surface.y)},
					{EncodableValue("width"),             EncodableValue(message.surface.width)},
					{EncodableValue("height"),            EncodableValue(message.surface.height)},
					{EncodableValue("scale"),             EncodableValue(message.surface.scale)},
					{EncodableValue("subsurfaces_below"), EncodableValue(std::move(subsurfaces_below))},
					{EncodableValue("subsurfaces_above"), EncodableValue(std::move(subsurfaces_above))},
					{EncodableValue("input_region"),      EncodableValue(EncodableMap{
						  {EncodableValue("x1"), EncodableValue((int64_t) message.surface.input_region.x1)},
						  {EncodableValue("y1"), EncodableValue((int64_t) message.surface.input_region.y1)},
						  {EncodableValue("x2"), EncodableValue((int64_t) message.surface.input_region.x2)},
						  {EncodableValue("y2"), EncodableValue((int64_t) message.surface.input_region.y2)},
					})},
			  })},
		};
		if (message.xdg_surface.has_value()) {
			map.insert({EncodableValue("has_xdg_surface"), EncodableValue(true)});
			map.insert({EncodableValue("xdg_surface"), EncodableValue(EncodableMap{
				  {EncodableValue("role"),   EncodableValue((int64_t) message.xdg_surface->role)},
				  {EncodableValue("x"),      EncodableValue(message.xdg_surface->x)},
				  {EncodableValue("mapped"), EncodableValue(message.xdg_surface->mapped)},
				  {EncodableValue("y"),      EncodableValue(message.xdg_surface->y)},
				  {EncodableValue("width"),  EncodableValue(message.xdg_surface->width)},
				  {EncodableValue("height"), EncodableValue(message.xdg_surface->height)},
			})});
		} else {
			map.insert({EncodableValue("has_xdg_surface"), EncodableValue(false)});
		}

		if (message.xdg_popup.has_value()) {
			map.insert({EncodableValue("has_xdg_popup"), EncodableValue(true)});
			map.insert({EncodableValue("xdg_popup"), EncodableValue(EncodableMap{
				  {EncodableValue("parent_id"), EncodableValue(message.xdg_popup->parent_id)},
				  {EncodableValue("x"),         EncodableValue(message.xdg_popup->x)},
				  {EncodableValue("y"),         EncodableValue(message.xdg_popup->y)},
				  {EncodableValue("width"),     EncodableValue(message.xdg_popup->width)},
				  {EncodableValue("height"),    EncodableValue(message.xdg_popup->height)},
			})});
		} else {
			map.insert({EncodableValue("has_xdg_popup"), EncodableValue(false)});
		}

		if (message.toplevel_decoration.has_value()) {
			map.insert({EncodableValue("has_toplevel_decoration"), EncodableValue(true)});
			map.insert({EncodableValue("toplevel_decoration"), EncodableValue((int64_t) *message.toplevel_decoration)});
		} else {
			map.insert({EncodableValue("has_toplevel_decoration"), EncodableValue(false)});
		}

		if (message.toplevel_title.has_value()) {
			map.insert({EncodableValue("has_toplevel_title"), EncodableValue(true)});
			map.insert({EncodableValue("toplevel_title"), EncodableValue(*message.toplevel_title)});
		} else {
			map.insert({EncodableValue("has_toplevel_title"), EncodableValue(false)});
		}

		if (message.toplevel_app_id.has_value()) {
			map.insert({EncodableValue("has_toplevel_app_id"), EncodableValue(true)});
			map.insert({EncodableValue("toplevel_app_id"), EncodableValue(*message.toplevel_app_id)});
		} else {
			map.insert({EncodableValue("has_toplevel_app_id"), EncodableValue(false)});
		}

		auto value = std::make_unique<EncodableValue>(map);
		platform_method_channel->InvokeMethod("commit_surface", std::move(value));
	});
}

void EmbedderState::map_xdg_surface(size_t view_id) {
	callable_queue.enqueue([view_id, this] {
		auto value = std::make_unique<EncodableValue>(EncodableMap{
			  {EncodableValue("view_id"), EncodableValue((int64_t) view_id)},
		});
		platform_method_channel->InvokeMethod("map_xdg_surface", std::move(value));
	});
}

void EmbedderState::unmap_xdg_surface(size_t view_id) {
	callable_queue.enqueue([view_id, this] {
		auto value = std::make_unique<EncodableValue>(EncodableMap{
			  {EncodableValue("view_id"), EncodableValue((int64_t) view_id)},
		});
		platform_method_channel->InvokeMethod("unmap_xdg_surface", std::move(value));
	});
}

void EmbedderState::map_subsurface(size_t view_id) {
	callable_queue.enqueue([view_id, this] {
		auto value = std::make_unique<EncodableValue>(EncodableMap{
			  {EncodableValue("view_id"), EncodableValue((int64_t) view_id)},
		});
		platform_method_channel->InvokeMethod("map_subsurface", std::move(value));
	});
}

void EmbedderState::unmap_subsurface(size_t view_id) {
	callable_queue.enqueue([view_id, this] {
		auto value = std::make_unique<EncodableValue>(EncodableMap{
			  {EncodableValue("view_id"), EncodableValue((int64_t) view_id)},
		});
		platform_method_channel->InvokeMethod("unmap_subsurface", std::move(value));
	});
}

void EmbedderState::send_text_input_event(size_t view_id, TextInputEventType event) {
	callable_queue.enqueue([event, view_id, this] {
		const char* type;
		switch (event) {
			case TextInputEventType::enable:
				type = "enable";
				break;
			case TextInputEventType::disable:
				type = "disable";
				break;
			case TextInputEventType::commit:
				type = "commit";
				break;
		}
		auto value = std::make_unique<EncodableValue>(EncodableMap{
			  {EncodableValue("view_id"), EncodableValue((int64_t) view_id)},
			  {EncodableValue("type"),    EncodableValue(type)},
		});
		platform_method_channel->InvokeMethod("send_text_input_event", std::move(value));
	});
}

void EmbedderState::interactive_move(size_t view_id) {
	callable_queue.enqueue([view_id, this] {
		auto value = std::make_unique<EncodableValue>(EncodableMap{
			  {EncodableValue("view_id"), EncodableValue((int64_t) view_id)},
		});
		platform_method_channel->InvokeMethod("interactive_move", std::move(value));
	});
}

void EmbedderState::interactive_resize(size_t view_id, xdg_toplevel_resize_edge edge) {
	callable_queue.enqueue([view_id, edge, this] {
		auto value = std::make_unique<EncodableValue>(EncodableMap{
			  {EncodableValue("view_id"), EncodableValue((int64_t) view_id)},
			  {EncodableValue("edge"),    EncodableValue((int64_t) edge)},
		});
		platform_method_channel->InvokeMethod("interactive_resize", std::move(value));
	});
}

void EmbedderState::set_window_title(size_t view_id, const std::string& title) {
	callable_queue.enqueue([view_id, title, this] {
		auto value = std::make_unique<EncodableValue>(EncodableMap{
			  {EncodableValue("view_id"), EncodableValue((int64_t) view_id)},
			  {EncodableValue("title"),   EncodableValue(title)},
		});
		platform_method_channel->InvokeMethod("set_title", std::move(value));
	});
}

void EmbedderState::set_app_id(size_t view_id, const std::string& app_id) {
	callable_queue.enqueue([view_id, app_id, this] {
		auto value = std::make_unique<EncodableValue>(EncodableMap{
			  {EncodableValue("view_id"), EncodableValue((int64_t) view_id)},
			  {EncodableValue("app_id"),  EncodableValue(app_id)},
		});
		platform_method_channel->InvokeMethod("set_app_id", std::move(value));
	});
}

void EmbedderState::update_text_editing_state() {
	callable_queue.enqueue([this] {
		assert(current_text_input_client.has_value());
		current_text_input_client->update_editing_state();
	});
}

void EmbedderState::destroy_surface(size_t view_id) {
	callable_queue.enqueue([view_id, this] {
		auto value = std::make_unique<EncodableValue>(EncodableMap{
			  {EncodableValue("view_id"), EncodableValue((int64_t) view_id)},
		});
		platform_method_channel->InvokeMethod("destroy_surface", std::move(value));
	});
}
