# ================================================================
# Five Nights at Freddy's 2 - C Port
# Cross-platform Makefile
# ================================================================
#
# Targets:
#   make                - Build SDL2 version (default)
#   make sdl2           - Build SDL2 version
#   make stub           - Build stub version (for testing compilation)
#   make clean          - Remove build artifacts
#
# Platform defines:
#   FNAF2_PLATFORM_SDL2 - SDL2 backend (PC: Linux, Windows, macOS)
#   FNAF2_PLATFORM_STUB - Stub backend (template for porting)
#
# For custom platforms, create your own platform_*.c and compile with:
#   make PLATFORM=your_platform PLATFORM_CFLAGS=... PLATFORM_LDFLAGS=...
#
# Cross-compilation example (PS1 with PSn00bSDK):
#   make CC=mipsel-none-elf-gcc \
#        PLATFORM=ps1 \
#        PLATFORM_CFLAGS="-DFNAF2_NO_FLOAT -DFNAF2_NO_STDLIB" \
#        PLATFORM_LDFLAGS="-lpsxgpu -lpsxspu -lpsxetc"
# ================================================================

# Compiler
CC      ?= gcc
AR      ?= ar

# Build directory
BUILD   = build

# Source files (platform-independent game logic)
GAME_SRCS = \
    src/animatronic.c \
    src/camera.c \
    src/music_box.c \
    src/office.c \
    src/night.c \
    src/menu.c \
    src/resource.c \
    src/game.c \
    src/main.c

# Common compiler flags
COMMON_CFLAGS = -Wall -Wextra -Wpedantic -std=c99 -Isrc

# ================================================================
# SDL2 TARGET (default)
# ================================================================

SDL2_CFLAGS  = $(COMMON_CFLAGS) -DFNAF2_PLATFORM_SDL2 $(shell pkg-config --cflags sdl2 SDL2_image SDL2_mixer SDL2_ttf 2>/dev/null)
SDL2_LDFLAGS = $(shell pkg-config --libs sdl2 SDL2_image SDL2_mixer SDL2_ttf 2>/dev/null)

# Fallback if pkg-config not available
ifeq ($(SDL2_LDFLAGS),)
    SDL2_LDFLAGS = -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_ttf
endif

SDL2_SRCS    = $(GAME_SRCS) src/platform/platform_sdl2.c
SDL2_OBJS    = $(SDL2_SRCS:src/%.c=$(BUILD)/sdl2/%.o)
SDL2_TARGET  = $(BUILD)/fnaf2

# ================================================================
# STUB TARGET (for compilation testing / porting template)
# ================================================================

STUB_CFLAGS  = $(COMMON_CFLAGS) -DFNAF2_PLATFORM_STUB
STUB_LDFLAGS =
STUB_SRCS    = $(GAME_SRCS) src/platform/platform_stub.c
STUB_OBJS    = $(STUB_SRCS:src/%.c=$(BUILD)/stub/%.o)
STUB_TARGET  = $(BUILD)/fnaf2_stub

# ================================================================
# STATIC LIBRARY (for embedding in other projects)
# ================================================================

LIB_CFLAGS  = $(COMMON_CFLAGS)
LIB_SRCS    = $(GAME_SRCS)
LIB_OBJS    = $(LIB_SRCS:src/%.c=$(BUILD)/lib/%.o)
LIB_TARGET  = $(BUILD)/libfnaf2.a

# ================================================================
# CUSTOM PLATFORM
# ================================================================

PLATFORM         ?=
PLATFORM_CFLAGS  ?=
PLATFORM_LDFLAGS ?=
PLATFORM_SRC     ?= src/platform/platform_$(PLATFORM).c

# ================================================================
# DEFAULT TARGET
# ================================================================

.PHONY: all sdl2 stub lib custom clean

all: sdl2

# ================================================================
# SDL2 BUILD
# ================================================================

sdl2: $(SDL2_TARGET)

$(SDL2_TARGET): $(SDL2_OBJS)
	@mkdir -p $(dir $@)
	$(CC) $(SDL2_OBJS) -o $@ $(SDL2_LDFLAGS)
	@echo "Built: $@"

$(BUILD)/sdl2/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(SDL2_CFLAGS) -c $< -o $@

# ================================================================
# STUB BUILD
# ================================================================

stub: $(STUB_TARGET)

$(STUB_TARGET): $(STUB_OBJS)
	@mkdir -p $(dir $@)
	$(CC) $(STUB_OBJS) -o $@ $(STUB_LDFLAGS)
	@echo "Built: $@"

$(BUILD)/stub/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(STUB_CFLAGS) -c $< -o $@

# ================================================================
# STATIC LIBRARY
# ================================================================

lib: $(LIB_TARGET)

$(LIB_TARGET): $(LIB_OBJS)
	@mkdir -p $(dir $@)
	$(AR) rcs $@ $(LIB_OBJS)
	@echo "Built: $@"

$(BUILD)/lib/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(LIB_CFLAGS) -c $< -o $@

# ================================================================
# CUSTOM PLATFORM BUILD
# ================================================================

ifneq ($(PLATFORM),)
CUSTOM_CFLAGS  = $(COMMON_CFLAGS) -DFNAF2_PLATFORM_$(shell echo $(PLATFORM) | tr a-z A-Z) $(PLATFORM_CFLAGS)
CUSTOM_SRCS    = $(GAME_SRCS) $(PLATFORM_SRC)
CUSTOM_OBJS    = $(CUSTOM_SRCS:src/%.c=$(BUILD)/$(PLATFORM)/%.o)
CUSTOM_TARGET  = $(BUILD)/fnaf2_$(PLATFORM)

custom: $(CUSTOM_TARGET)

$(CUSTOM_TARGET): $(CUSTOM_OBJS)
	@mkdir -p $(dir $@)
	$(CC) $(CUSTOM_OBJS) -o $@ $(PLATFORM_LDFLAGS)
	@echo "Built: $@"

$(BUILD)/$(PLATFORM)/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CUSTOM_CFLAGS) -c $< -o $@
endif

# ================================================================
# DEBUG BUILD
# ================================================================

debug: SDL2_CFLAGS += -g -O0 -DDEBUG
debug: sdl2

release: SDL2_CFLAGS += -O2 -DNDEBUG
release: sdl2

# ================================================================
# CLEAN
# ================================================================

clean:
	rm -rf $(BUILD)
	@echo "Cleaned build directory"
