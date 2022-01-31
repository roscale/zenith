#include "view.hpp"
#include "server.hpp"
#include "util.hpp"
#include "wayland_view.hpp"


extern "C" {
#define static
#include "wlr/render/wlr_texture.h"
#include "wlr/types/wlr_xdg_shell.h"
#include "wlr/render/gles2.h"
#undef static
}

using namespace flutter;

static size_t next_view_id = 1;

ZenithView::ZenithView(ZenithServer* server, wlr_surface* surface)
	  : server(server), surface(surface), id(next_view_id++) {
}

