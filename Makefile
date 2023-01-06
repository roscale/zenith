CXX := clang++ -std=c++17

uname_m = $(shell uname -m)
ifeq ($(uname_m),x86_64)
ARCH += x64
ARCH_DEB += amd64
else
ARCH += arm64
ARCH_DEB += arm64
endif

TARGET_EXEC := zenith
SRC_DIRS := src
DEPS_DIR := deps

DEBUG_BUILD_DIR := build/$(TARGET_EXEC)/debug
PROFILE_BUILD_DIR := build/$(TARGET_EXEC)/profile
RELEASE_BUILD_DIR := build/$(TARGET_EXEC)/release

DEBUG_BUNDLE_DIR := $(DEBUG_BUILD_DIR)/bundle
PROFILE_BUNDLE_DIR := $(PROFILE_BUILD_DIR)/bundle
RELEASE_BUNDLE_DIR := $(RELEASE_BUILD_DIR)/bundle

# Find all the C and C++ files we want to compile
# Note the single quotes around the * expressions. Make will incorrectly expand these otherwise.
SRCS := $(shell find $(SRC_DIRS) -name '*.cpp' -or -name '*.cc' -or -name '*.c')

# String substitution for every C/C++ file.
# As an example, hello.cpp turns into ./build/hello.cpp.o
DEBUG_OBJS := $(SRCS:%=$(DEBUG_BUILD_DIR)/%.o)
PROFILE_OBJS := $(SRCS:%=$(PROFILE_BUILD_DIR)/%.o)
RELEASE_OBJS := $(SRCS:%=$(RELEASE_BUILD_DIR)/%.o)

# String substitution (suffix version without %).
# As an example, ./build/hello.cpp.o turns into ./build/hello.cpp.d
DEBUG_DEPS := $(DEBUG_OBJS:.o=.d)
PROFILE_DEPS := $(PROFILE_OBJS:.o=.d)
RELEASE_DEPS := $(RELEASE_OBJS:.o=.d)

# Every folder in ./src will need to be passed to GCC so that it can find header files
INC_DIRS := $(shell find $(SRC_DIRS) -type d)
# Add a prefix to INC_DIRS. So moduleA would become -ImoduleA. GCC understands this -I flag
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

ASAN := -g -fno-omit-frame-pointer -fsanitize=address
WARNINGS := -Wall -Wextra -Werror \
			-Wno-unused-parameter -Wno-unused-variable -Wno-invalid-offsetof -Wno-unknown-pragmas

# The -MMD and -MP flags together generate Makefiles for us!
# These files will have .d instead of .o as the output.
COMMON_CPPFLAGS := $(INC_FLAGS) \
	 $(WARNINGS) \
	`pkg-config --cflags pixman-1` \
	`pkg-config --cflags libdrm` \
	 -MMD -MP -DWLR_USE_UNSTABLE

DEBUG_CPPFLAGS := $(COMMON_CPPFLAGS) -DDEBUG $(ASAN)
PROFILE_CPPFLAGS := $(COMMON_CPPFLAGS) -DPROFILE
RELEASE_CPPFLAGS := $(COMMON_CPPFLAGS) -O2

COMMON_LDFLAGS := -lwayland-server -lxkbcommon -lpixman-1 -linput -lwlroots -lepoxy -lGLESv2 -lEGL -lGL -lpam -ldrm -L. -L$(DEPS_DIR)
DEBUG_LDFLAGS := $(COMMON_LDFLAGS) -lflutter_engine_debug $(ASAN)
PROFILE_LDFLAGS := $(COMMON_LDFLAGS) -lflutter_engine_profile
RELEASE_LDFLAGS := $(COMMON_LDFLAGS) -lflutter_engine_release

ENGINE_REVISION := $(shell flutter --version | grep Engine | awk '{print $$NF}')

$(DEPS_DIR)/libflutter_engine_debug.so:
	curl -L https://github.com/sony/flutter-embedded-linux/releases/download/$(ENGINE_REVISION)/elinux-$(ARCH)-debug.zip >/tmp/elinux-$(ARCH)-debug.zip
	unzip -o /tmp/elinux-$(ARCH)-debug.zip -d /tmp || exit
	mkdir -p deps
	mv /tmp/libflutter_engine.so deps/libflutter_engine_debug.so

$(DEPS_DIR)/libflutter_engine_profile.so:
	curl -L https://github.com/sony/flutter-embedded-linux/releases/download/$(ENGINE_REVISION)/elinux-$(ARCH)-profile.zip >/tmp/elinux-$(ARCH)-profile.zip
	unzip -o /tmp/elinux-$(ARCH)-profile.zip -d /tmp || exit
	mkdir -p deps
	mv /tmp/libflutter_engine.so deps/libflutter_engine_profile.so

$(DEPS_DIR)/libflutter_engine_release.so:
	curl -L https://github.com/sony/flutter-embedded-linux/releases/download/$(ENGINE_REVISION)/elinux-$(ARCH)-release.zip >/tmp/elinux-$(ARCH)-release.zip
	unzip -o /tmp/elinux-$(ARCH)-release.zip -d /tmp || exit
	mkdir -p deps
	mv /tmp/libflutter_engine.so deps/libflutter_engine_release.so

$(DEBUG_BUNDLE_DIR)/$(TARGET_EXEC): $(DEBUG_OBJS) $(DEPS_DIR)/libflutter_engine_debug.so
	mkdir -p $(dir $@)
	$(CXX) $(DEBUG_OBJS) -o $@ -Wl,-rpath='$$ORIGIN/lib' $(DEBUG_LDFLAGS)

$(PROFILE_BUNDLE_DIR)/$(TARGET_EXEC): $(PROFILE_OBJS) $(DEPS_DIR)/libflutter_engine_profile.so
	mkdir -p $(dir $@)
	$(CXX) $(PROFILE_OBJS) -o $@ -Wl,-rpath='$$ORIGIN/lib' $(PROFILE_LDFLAGS)

$(RELEASE_BUNDLE_DIR)/$(TARGET_EXEC): $(RELEASE_OBJS) $(DEPS_DIR)/libflutter_engine_release.so
	mkdir -p $(dir $@)
	$(CXX) $(RELEASE_OBJS) -o $@ -Wl,-rpath='$$ORIGIN/lib' $(RELEASE_LDFLAGS)

# Build step for C source
$(DEBUG_BUILD_DIR)/%.c.o: %.c Makefile
	mkdir -p $(dir $@)
	$(CC) $(DEBUG_CPPFLAGS) $(CFLAGS) -c $< -o $@

$(PROFILE_BUILD_DIR)/%.c.o: %.c Makefile
	mkdir -p $(dir $@)
	$(CC) $(PROFILE_CPPFLAGS) $(CFLAGS) -c $< -o $@

$(RELEASE_BUILD_DIR)/%.c.o: %.c Makefile
	mkdir -p $(dir $@)
	$(CC) $(RELEASE_CPPFLAGS) $(CFLAGS) -c $< -o $@

# Build step for C++ source
$(DEBUG_BUILD_DIR)/%.cpp.o: %.cpp Makefile
	mkdir -p $(dir $@)
	$(CXX) $(DEBUG_CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(PROFILE_BUILD_DIR)/%.cpp.o: %.cpp Makefile
	mkdir -p $(dir $@)
	$(CXX) $(PROFILE_CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(RELEASE_BUILD_DIR)/%.cpp.o: %.cpp Makefile
	mkdir -p $(dir $@)
	$(CXX) $(RELEASE_CPPFLAGS) $(CXXFLAGS) -c $< -o $@

# Build step for C++ source
$(DEBUG_BUILD_DIR)/%.cc.o: %.cc Makefile
	mkdir -p $(dir $@)
	$(CXX) $(DEBUG_CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(PROFILE_BUILD_DIR)/%.cc.o: %.cc Makefile
	mkdir -p $(dir $@)
	$(CXX) $(PROFILE_CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(RELEASE_BUILD_DIR)/%.cc.o: %.cc Makefile
	mkdir -p $(dir $@)
	$(CXX) $(RELEASE_CPPFLAGS) $(CXXFLAGS) -c $< -o $@

.PHONY: clean all \
		flutter_debug flutter_profile flutter_release \
 		debug_bundle profile_bundle release_bundle \
 		deb_package attach_debugger

flutter_debug:
	flutter build linux --debug

flutter_profile:
	flutter build linux --profile

flutter_release:
	flutter build linux --release

debug_bundle: flutter_debug $(DEBUG_BUNDLE_DIR)/$(TARGET_EXEC)
	mkdir -p $(DEBUG_BUNDLE_DIR)/lib/
	cp $(DEPS_DIR)/libflutter_engine_debug.so $(DEBUG_BUNDLE_DIR)/lib/libflutter_engine.so
	cp -r build/linux/$(ARCH)/debug/bundle/data $(DEBUG_BUNDLE_DIR)
	cp lsan_suppressions.txt $(DEBUG_BUNDLE_DIR)

profile_bundle: flutter_profile $(PROFILE_BUNDLE_DIR)/$(TARGET_EXEC)
	mkdir -p $(PROFILE_BUNDLE_DIR)/lib/
	cp $(DEPS_DIR)/libflutter_engine_profile.so $(PROFILE_BUNDLE_DIR)/lib/libflutter_engine.so
	cp build/linux/$(ARCH)/profile/bundle/lib/libapp.so $(PROFILE_BUNDLE_DIR)/lib
	cp -r build/linux/$(ARCH)/profile/bundle/data $(PROFILE_BUNDLE_DIR)

release_bundle: flutter_release $(RELEASE_BUNDLE_DIR)/$(TARGET_EXEC)
	mkdir -p $(RELEASE_BUNDLE_DIR)/lib/
	cp $(DEPS_DIR)/libflutter_engine_release.so $(RELEASE_BUNDLE_DIR)/lib/libflutter_engine.so
	cp build/linux/$(ARCH)/release/bundle/lib/libapp.so $(RELEASE_BUNDLE_DIR)/lib
	cp -r build/linux/$(ARCH)/release/bundle/data $(RELEASE_BUNDLE_DIR)

# Usage: make deb_package VERSION=0.2
deb_package: release_bundle
	mkdir -p build/zenith/release/deb/debpkg
	cp -r dpkg/* build/zenith/release/deb/debpkg

	sed -i 's/$$VERSION/$(VERSION)/g' build/zenith/release/deb/debpkg/DEBIAN/control
	sed -i 's/$$ARCH/$(ARCH_DEB)/g' build/zenith/release/deb/debpkg/DEBIAN/control
	cp -r build/zenith/release/bundle/* build/zenith/release/deb/debpkg/opt/zenith

	dpkg-deb -Zxz --root-owner-group --build build/zenith/release/deb/debpkg build/zenith/release/deb

attach_debugger:
	flutter attach --debug-uri=http://127.0.0.1:12345/

all: debug_bundle profile_bundle release_bundle

clean:
	-rm -r $(DEBUG_BUILD_DIR)
	-rm -r $(PROFILE_BUILD_DIR)
	-rm -r $(RELEASE_BUILD_DIR)

# Include the .d makefiles. The - at the front suppresses the errors of missing
# Makefiles. Initially, all the .d files will be missing, and we don't want those
# errors to show up.
-include $(DEBUG_DEPS)
-include $(PROFILE_DEPS)
-include $(RELEASE_DEPS)
