TARGET_EXEC := zenith
SRC_DIRS := ./src

DEBUG_BUILD_DIR := ./build/$(TARGET_EXEC)/debug
PROFILE_BUILD_DIR := ./build/$(TARGET_EXEC)/profile
RELEASE_BUILD_DIR := ./build/$(TARGET_EXEC)/release

# Find all the C and C++ files we want to compile
# Note the single quotes around the * expressions. Make will incorrectly expand these otherwise.
SRCS := $(shell find $(SRC_DIRS) -name '*.cpp' -or -name '*.cc' -or -name '*.c' -or -name '*.s')

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
INC_DIRS := $(shell find $(SRC_DIRS) -type d) ./ third_party /usr/include/pixman-1
# Add a prefix to INC_DIRS. So moduleA would become -ImoduleA. GCC understands this -I flag
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

# The -MMD and -MP flags together generate Makefiles for us!
# These files will have .d instead of .o as the output.
ASAN := -g -fsanitize=address -fno-omit-frame-pointer -lasan -lrt
WARNINGS := -Wall -Wextra -Wno-unused-parameter -Werror

DEBUG_CPPFLAGS := $(INC_FLAGS) -MMD -MP -DWLR_USE_UNSTABLE -DDEBUG $(WARNINGS) $(ASAN)
PROFILE_CPPFLAGS := $(INC_FLAGS) -MMD -MP -DWLR_USE_UNSTABLE -DPROFILE $(WARNINGS)
RELEASE_CPPFLAGS := $(INC_FLAGS) -MMD -MP -DWLR_USE_UNSTABLE -O2 $(WARNINGS)

DEBUG_LDFLAGS := -lGL -lEGL -lGLESv2 -lpthread -lwlroots -lwayland-server -lxkbcommon -L ./ -lflutter_engine_debug $(ASAN)
PROFILE_LDFLAGS := -lGL -lEGL -lGLESv2 -lpthread -lwlroots -lwayland-server -lxkbcommon -L ./ -lflutter_engine_profile
RELEASE_LDFLAGS := -lGL -lEGL -lGLESv2 -lpthread -lwlroots -lwayland-server -lxkbcommon -L ./ -lflutter_engine_release

all: debug_bundle profile_bundle release_bundle

debug_bundle: $(DEBUG_BUILD_DIR)/bundle/$(TARGET_EXEC)
	mkdir -p $(dir $<)/lib/
	cp libflutter_engine_debug.so $(dir $<)/lib/libflutter_engine.so
	cp -r build/linux/x64/debug/bundle/data $(dir $<)

profile_bundle: $(PROFILE_BUILD_DIR)/bundle/$(TARGET_EXEC)
	mkdir -p $(dir $<)/lib/
	cp libflutter_engine_profile.so $(dir $<)/lib/libflutter_engine.so
	cp build/linux/x64/profile/bundle/lib/libapp.so $(dir $<)/lib
	cp -r build/linux/x64/profile/bundle/data $(dir $<)

release_bundle: $(RELEASE_BUILD_DIR)/bundle/$(TARGET_EXEC)
	mkdir -p $(dir $<)/lib/
	cp libflutter_engine_release.so $(dir $<)/lib/libflutter_engine.so
	cp build/linux/x64/release/bundle/lib/libapp.so $(dir $<)/lib
	cp -r build/linux/x64/release/bundle/data $(dir $<)

attach_debugger:
	flutter attach --debug-uri=http://127.0.0.1:12345/

$(DEBUG_BUILD_DIR)/bundle/$(TARGET_EXEC): $(DEBUG_OBJS)
	mkdir -p $(dir $@)
	$(CXX) $(DEBUG_OBJS) -o $@ -Wl,-rpath='$$ORIGIN/lib' $(DEBUG_LDFLAGS)

$(PROFILE_BUILD_DIR)/bundle/$(TARGET_EXEC): $(PROFILE_OBJS)
	mkdir -p $(dir $@)
	$(CXX) $(PROFILE_OBJS) -o $@ -Wl,-rpath='$$ORIGIN/lib' $(PROFILE_LDFLAGS)

$(RELEASE_BUILD_DIR)/bundle/$(TARGET_EXEC): $(RELEASE_OBJS)
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

.PHONY: clean all debug_bundle profile_bundle release_bundle attach_debugger

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