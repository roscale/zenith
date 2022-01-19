#include "server.hpp"

extern "C" {
#define static
#include <wlr/util/log.h>
#undef static
}

int main() {
#ifdef DEBUG
	wlr_log_init(WLR_DEBUG, nullptr);
#endif

	ZenithServer::instance()->run();
}