#ifndef SKYCOIN_UX_H
#define SKYCOIN_UX_H

#include "cx.h"
#include "os_io_seproxyhal.h"


// display stepped screens
extern unsigned int ux_step;
extern unsigned int ux_step_count;
extern uint8_t ux_loop_over_curr_element; // only for Nano S

typedef struct {
	uint32_t keyIndex;
	uint8_t displayIndex;

    char address[36]; // set default value for address
	char address_copy[40]; // copy of address to be then displayed as scrolling text (for UI needs only)

	// // NUL-terminated strings for display
	// uint8_t typeStr[40]; // variable-length
	// uint8_t keyStr[40]; // variable-length
	// uint8_t fullStr[77]; // variable length
	
    // // partialStr contains 12 characters of a longer string. This allows text
	// // to be scrolled.
	// uint8_t partialStr[13];
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
#define TX_DESC_FONT BAGL_FONT_OPEN_SANS_REGULAR_11px | BAGL_FONT_ALIGNMENT_CENTER // text description font
#define COLOR_WHITE 0xFFFFFF // White color (hex)
#define SCREEN_MAX_CHARS 16 // Maximum number of characters to be displayed on screen(for scrolling text only)

#define UI_BACKGROUND() {{BAGL_RECTANGLE,0,0,0,128,32,0,0,BAGL_FILL,0,0xFFFFFF,0,0},NULL,0,0,0,NULL,NULL,NULL}
#define UI_ICON_LEFT(userid, glyph) {{BAGL_ICON,userid,3,12,7,7,0,0,0,0xFFFFFF,0,0,glyph},NULL,0,0,0,NULL,NULL,NULL}
#define UI_ICON_RIGHT(userid, glyph) {{BAGL_ICON,userid,117,13,8,6,0,0,0,0xFFFFFF,0,0,glyph},NULL,0,0,0,NULL,NULL,NULL}
#define UI_TEXT(userid, x, y, w, text) {{BAGL_LABELINE,userid,x,y,w,12,0,0,0,0xFFFFFF,0,BAGL_FONT_OPEN_SANS_REGULAR_11px|BAGL_FONT_ALIGNMENT_CENTER,0},(char *)text,0,0,0,NULL,NULL,NULL}


#endif

// #ifndef UI_H
// #define UI_H

// #include "os.h"
// #include "cx.h"

// #include<stdbool.h>

// #include "os_io_seproxyhal.h"
// #include "bagl.h"
// #include "glyphs.h"
// #include "ux.h"

// /** the timer */
// extern int exit_timer;

// /** max with of timer display */
// #define MAX_TIMER_TEXT_WIDTH 4

// /** display for the timer */
// extern char timer_desc[MAX_TIMER_TEXT_WIDTH];

// /** length of the APDU (application protocol data unit) header. */
// #define APDU_HEADER_LENGTH 5

// /** offset in the APDU header which says the length of the body. */
// #define APDU_BODY_LENGTH_OFFSET 4

// /** for signing, indicates this is the last part of the transaction. */
// #define P1_LAST 0x80

// /** for signing, indicates this is not the last part of the transaction, there are more parts coming. */
// #define P1_MORE 0x00

// /** length of BIP44 path */
// #define BIP44_PATH_LEN 5

// /** length of BIP44 path, in bytes */
// #define  BIP44_BYTE_LENGTH (BIP44_PATH_LEN * sizeof(unsigned int))

// /** length of BIP32 path */
// #define BIP32_PATH_LEN 4

// /** length of BIP32 path, in bytes */
// #define  BIP32_BYTE_LENGTH (BIP32_PATH_LEN * sizeof(unsigned int))

// /**
//  * Nano S has 320 KB flash, 10 KB RAM, uses a ST31H320 chip.
//  * This effectively limits the max size
//  * So we can only display 9 screens of data, and can only sign transactions up to 1kb in size.
//  * max size of a transaction, binary will not compile if we try to allow transactions over 1kb.
//  */
// #define MAX_TX_RAW_LENGTH 1024

// /** max width of a single line of text. */
// #define MAX_TX_TEXT_WIDTH 18

// /** max lines of text to display. */
// #define MAX_TX_TEXT_LINES 3

// /** max number of screens to display. */
// #define MAX_TX_TEXT_SCREENS 9

// /** max number of hex bytes that can be displayed (2 hex characters for 1 byte of data) */
// #define MAX_HEX_BUFFER_LEN (MAX_TX_TEXT_WIDTH / 2)

// /** max number of bytes for one line of text. */
// #define CURR_TX_DESC_LEN (MAX_TX_TEXT_LINES * MAX_TX_TEXT_WIDTH)

// /** max number of bytes for all text screens. */
// #define MAX_TX_DESC_LEN (MAX_TX_TEXT_SCREENS * CURR_TX_DESC_LEN)

// /** UI currently displayed */
// enum UI_STATE {
// 	UI_INIT, UI_IDLE, UI_TOP_SIGN, UI_TX_DESC_1,UI_TX_DESC_2, UI_SIGN, UI_DENY, UI_PUBLIC_KEY_1, UI_PUBLIC_KEY_2
// };

// /** UI state enum */
// extern enum UI_STATE uiState;

// /** UI state flag */
// extern ux_state_t ux;

// /** notification to restart the hash */
// extern unsigned char hashTainted;

// /** the hash. */
// extern cx_sha256_t hash;

// /** index of the current screen. */
// extern unsigned int curr_scr_ix;

// /** max index for all screens. */
// extern unsigned int max_scr_ix;

// /** raw transaction data. */
// extern unsigned char raw_tx[MAX_TX_RAW_LENGTH];

// /** current index into raw transaction. */
// extern unsigned int raw_tx_ix;

// /** current length of raw transaction. */
// extern unsigned int raw_tx_len;

// /** all text descriptions. */
// extern char tx_desc[MAX_TX_TEXT_SCREENS][MAX_TX_TEXT_LINES][MAX_TX_TEXT_WIDTH];

// /** currently displayed text description. */
// extern char curr_tx_desc[MAX_TX_TEXT_LINES][MAX_TX_TEXT_WIDTH];

// /** currently displayed public key */
// extern char current_public_key[MAX_TX_TEXT_LINES][MAX_TX_TEXT_WIDTH];

// // concatenate the ADDRESS_VERSION and the address.
// extern char address[36];

// /** process a partial transaction */
// const bagl_element_t * io_seproxyhal_touch_approve(const bagl_element_t *e);

// /** show the idle UI */
// void ui_idle(void);

// /** show the "Sign TX" ui, starting at the top of the Tx display */
// void ui_top_sign(void);

// /** return the length of the communication buffer */
// unsigned int get_apdu_buffer_length();

// void io_seproxyhal_display(const bagl_element_t *element);

// unsigned char io_event(unsigned char channel);

// #endif
