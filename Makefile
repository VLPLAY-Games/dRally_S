CC := gcc
#CC := clang
MSBUILD := MSBuild.exe

DEFINES := -DDR_MULTIPLAYER -DIPXNET -DDR_LETTERBOX #-DDR_CDCHECK
INCLUDES := -I/usr/include/SDL2
FLAGS := -O3 -Werror #-m32 -march=i386
LDFLAGS := -lm -lSDL2 -lSDL2_net

include objs.mk

all: drally_linux

drally_linux: $(UNI_OBJS) $(LINUX_OBJS) $(RACE_OBJS) $(MENU_OBJS) $(SHOP_OBJS) $(UNDERGROUND_OBJS) $(BI_OBJS) $(OBJS) $(MP_CMN_OBJS) $(MP_OBJS) $(DFR_OBJS)
	$(CC) -o $@ $^ $(FLAGS) $(LDFLAGS)

%.o: %.c drally.h drally_keyboard.h Makefile
	$(CC) -g -c -o $@ $<  $(INCLUDES) $(FLAGS) $(DEFINES)

.PHONY: clean dRally.exe

dRally.exe:
	@$(MSBUILD)

clean:
	@rm -f *.o drally_linux
