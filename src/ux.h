#ifndef SKYCOIN_UX_H
#define SKYCOIN_UX_H

#include "cx.h"
#include "os_io_seproxyhal.h"

#define BIP44_PATH_LEN 5 // length of BIP44 path
#define BIP44_BYTE_LENGTH (BIP44_PATH_LEN * sizeof(unsigned int)) // length of BIP44 path, in bytes

// display stepped screens
extern unsigned int ux_step;
extern unsigned int ux_step_count;
extern uint8_t ux_loop_over_curr_element; // only for Nano S

typedef struct {
    char address[36]; // set default value for address
	char address_copy[40]; // copy of address to be then displayed as scrolling text (for UI needs only)
} getPublicKeyContext_t;


typedef struct {
	getPublicKeyContext_t getPublicKeyContext;
} commandContext;

extern commandContext global;
extern ux_state_t ux;


// ********************************************************************************
// Defines for easier UI structures usage
// ********************************************************************************
#define DEFAULT_FONT BAGL_FONT_OPEN_SANS_REGULAR_11px | BAGL_FONT_ALIGNMENT_CENTER // default font
#define COLOR_WHITE 0xFFFFFF // White color (hex)
#define SCREEN_MAX_CHARS 16 // Maximum number of characters to be displayed on screen(for scrolling text only)

#define UI_BACKGROUND() {{BAGL_RECTANGLE,0,0,0,128,32,0,0,BAGL_FILL,0,COLOR_WHITE,0,0},NULL,0,0,0,NULL,NULL,NULL}
#define UI_ICON_LEFT(userid, glyph) {{BAGL_ICON,userid,3,12,7,7,0,0,0,COLOR_WHITE,0,0,glyph},NULL,0,0,0,NULL,NULL,NULL}
#define UI_ICON_RIGHT(userid, glyph) {{BAGL_ICON,userid,117,13,8,6,0,0,0,COLOR_WHITE,0,0,glyph},NULL,0,0,0,NULL,NULL,NULL}
#define UI_TEXT(userid, x, y, w, text) {{BAGL_LABELINE,userid,x,y,w,12,0,0,0,COLOR_WHITE,0,DEFAULT_FONT,0},(char *)text,0,0,0,NULL,NULL,NULL}

#endif
