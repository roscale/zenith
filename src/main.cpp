#include <getopt.h>
#include "server.hpp"

extern "C" {
#define static
#include <wlr/util/log.h>
#undef static
}

int main(int argc, char* argv[]) {
#ifdef DEBUG
	wlr_log_init(WLR_DEBUG, nullptr);
#endif

	while ((getopt(argc, argv, "")) != -1);
	if (optind == argc) {
		std::cout << "Usage: " << argv[0] << " COMMAND\n";
		exit(1);
	}
	char* startup_cmd = argv[optind];

	ZenithServer::instance()->run(startup_cmd);
}