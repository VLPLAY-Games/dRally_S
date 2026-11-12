/* Just enough of libnx's switch.h (the software keyboard) to build switch_text.c on a PC. */
#ifndef STUB_SWITCH_H
#define STUB_SWITCH_H

typedef unsigned int Result;
#define R_FAILED(r) ((r) != 0)

typedef struct { int max; } SwkbdConfig;

extern const char * stub_keyboard_text;     /* what the "keyboard" returns, NULL for cancel */
extern int stub_keyboard_shown;

Result swkbdCreate(SwkbdConfig * c, int n);
void swkbdConfigMakePresetDefault(SwkbdConfig * c);
void swkbdConfigSetHeaderText(SwkbdConfig * c, const char * s);
void swkbdConfigSetGuideText(SwkbdConfig * c, const char * s);
void swkbdConfigSetStringLenMax(SwkbdConfig * c, int n);
void swkbdConfigSetStringLenMin(SwkbdConfig * c, int n);
Result swkbdShow(SwkbdConfig * c, char * out, unsigned long size);
void swkbdClose(SwkbdConfig * c);

#endif
