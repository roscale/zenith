TARGET_EXEC := zenith
SRC_DIRS := ./src

DBG_BUILD_DIR := ./build/$(TARGET_EXEC)/debug
REL_BUILD_DIR := ./build/$(TARGET_EXEC)/release

# Find all the C and C++ files we want to compile
# Note the single quotes around the * expressions. Make will incorrectly expand these otherwise.
SRCS := $(shell find $(SRC_DIRS) -name '*.cpp' -or -name '*.cc' -or -name '*.c' -or -name '*.s')

# String substitution for every C/C++ file.
# As an example, hello.cpp turns into ./build/hello.cpp.o
DBG_OBJS := $(SRCS:%=$(DBG_BUILD_DIR)/%.o)
REL_OBJS := $(SRCS:%=$(REL_BUILD_DIR)/%.o)

# String substitution (suffix version without %).
# As an example, ./build/hello.cpp.o turns into ./build/hello.cpp.d
DBG_DEPS := $(DBG_OBJS:.o=.d)
REL_DEPS := $(REL_OBJS:.o=.d)

# Every folder in ./src will need to be passed to GCC so that it can find header files
INC_DIRS := $(shell find $(SRC_DIRS) -type d) ./ third_party /usr/include/pixman-1
# Add a prefix to INC_DIRS. So moduleA would become -ImoduleA. GCC understands this -I flag
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

# The -MMD and -MP flags together generate Makefiles for us!
# These files will have .d instead of .o as the output.
ASAN := -g -fsanitize=address -fno-omit-frame-pointer -lasan -lrt
WARNINGS := -Wall -Wextra -Wno-unused-parameter -Werror

DBG_CPPFLAGS := $(INC_FLAGS) -MMD -MP -DWLR_USE_UNSTABLE -DDEBUG $(WARNINGS) $(ASAN)
REL_CPPFLAGS := $(INC_FLAGS) -MMD -MP -DWLR_USE_UNSTABLE -O2 $(WARNINGS)

DBG_LDFLAGS := -lGL -lEGL -lGLESv2 -lpthread -lwlroots -lwayland-server -lxkbcommon -L ./ -lflutter_engine_debug $(ASAN)
REL_LDFLAGS := -lGL -lEGL -lGLESv2 -lpthread -lwlroots -lwayland-server -lxkbcommon -L ./ -lflutter_engine_release

all: debug_bundle release_bundle

debug_bundle: $(DBG_BUILD_DIR)/bundle/$(TARGET_EXEC)
	mkdir -p $(dir $<)/lib/
	cp libflutter_engine_debug.so $(dir $<)/lib/libflutter_engine.so
	cp -r build/linux/x64/debug/bundle/data $(dir $<)

release_bundle: $(REL_BUILD_DIR)/bundle/$(TARGET_EXEC)
	mkdir -p $(dir $<)/lib/
	cp libflutter_engine_release.so $(dir $<)/lib/libflutter_engine.so
	cp build/linux/x64/release/bundle/lib/libapp.so $(dir $<)/lib
	cp -r build/linux/x64/release/bundle/data $(dir $<)

$(DBG_BUILD_DIR)/bundle/$(TARGET_EXEC): $(DBG_OBJS)
	mkdir -p $(dir $@)
	$(CXX) $(DBG_OBJS) -o $@ -Wl,-rpath='$$ORIGIN/lib' $(DBG_LDFLAGS)

$(REL_BUILD_DIR)/bundle/$(TARGET_EXEC): $(REL_OBJS)
	mkdir -p $(dir $@)
	$(CXX) $(REL_OBJS) -o $@ -Wl,-rpath='$$ORIGIN/lib' $(REL_LDFLAGS)

# Build step for C source
$(DBG_BUILD_DIR)/%.c.o: %.c Makefile
	mkdir -p $(dir $@)
	$(CC) $(DBG_CPPFLAGS) $(CFLAGS) -c $< -o $@

# Build step for C source
$(REL_BUILD_DIR)/%.c.o: %.c Makefile
	mkdir -p $(dir $@)
	$(CC) $(REL_CPPFLAGS) $(CFLAGS) -c $< -o $@

# Build step for C++ source
$(DBG_BUILD_DIR)/%.cpp.o: %.cpp Makefile
	mkdir -p $(dir $@)
	$(CXX) $(DBG_CPPFLAGS) $(CXXFLAGS) -c $< -o $@

# Build step for C++ source
$(REL_BUILD_DIR)/%.cpp.o: %.cpp Makefile
	mkdir -p $(dir $@)
	$(CXX) $(REL_CPPFLAGS) $(CXXFLAGS) -c $< -o $@

# Build step for C++ source
$(DBG_BUILD_DIR)/%.cc.o: %.cc Makefile
	mkdir -p $(dir $@)
	$(CXX) $(DBG_CPPFLAGS) $(CXXFLAGS) -c $< -o $@

# Build step for C++ source
$(REL_BUILD_DIR)/%.cc.o: %.cc Makefile
	mkdir -p $(dir $@)
	$(CXX) $(REL_CPPFLAGS) $(CXXFLAGS) -c $< -o $@

.PHONY: clean all debug_bundle release_bundle

clean:
	-rm -r $(DBG_BUILD_DIR)
	-rm -r $(REL_BUILD_DIR)

# Include the .d makefiles. The - at the front suppresses the errors of missing
# Makefiles. Initially, all the .d files will be missing, and we don't want those
# errors to show up.
-include $(DEPS)