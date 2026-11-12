CC := gcc
#CC := clang
MSBUILD := MSBuild.exe

DEFINES := -DDR_MULTIPLAYER -DIPXNET -DDR_LETTERBOX #-DDR_CDCHECK
INCLUDES := -I/usr/include/SDL2 -Iinclude
FLAGS := -O3 -Werror #-m32 -march=i386
LDFLAGS := -lm -lSDL2 -lSDL2_net

include objs.mk

# Sources live in src/ (see the layout in README.md); objects go to build/linux.
BUILD := build/linux
vpath %.c $(shell find src -type d -not -path 'src/platform/switch*')


all: drally_linux

LINUX_ALL_OBJS := $(addprefix $(BUILD)/,$(UNI_OBJS) $(LINUX_OBJS) $(RACE_OBJS) $(MENU_OBJS) $(SHOP_OBJS) $(UNDERGROUND_OBJS) $(BI_OBJS) $(OBJS) $(MP_CMN_OBJS) $(MP_OBJS) $(DFR_OBJS))

drally_linux: $(LINUX_ALL_OBJS)
	$(CC) -o $@ $^ $(FLAGS) $(LDFLAGS)

$(BUILD)/%.o: %.c include/drally.h include/drally_keyboard.h Makefile
	@mkdir -p $(BUILD)
	$(CC) -g -c -o $@ $<  $(INCLUDES) $(FLAGS) $(DEFINES)

.PHONY: clean dRally.exe

dRally.exe:
	@$(MSBUILD)

clean:
	@rm -rf $(BUILD) drally_linux
