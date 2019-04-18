#ifndef SKYCOIN_UX_H
#define SKYCOIN_UX_H

#include "os.h"
#include "cx.h"
#include "os_io_seproxyhal.h"
#include "stdbool.h"
//#include "skycoin-api/txn.h"

#define BIP44_PATH_LEN 5 // length of BIP44 path
#define BIP44_BYTE_LENGTH (BIP44_PATH_LEN * sizeof(unsigned int)) // length of BIP44 path, in bytes

#define SCREEN_MAX_CHARS 16 // Maximum number of characters to be displayed on screen(for scrolling text only) 
#define SCROLLING_TEXT_DELAY 300 // Delay for scrolling text, before the next iteration(movement) will be done (in milliseconds) 
#define SCROLLING_TEXT_BIG_DELAY 1200 //  Big delay (e.g. for a longer stop in the beginning and in the end)

// display stepped screens
extern unsigned int ux_step;
extern unsigned int ux_step_count;
extern uint8_t ux_loop_over_curr_element; // only for Nano S

typedef struct {
    unsigned int bip44_path[BIP44_PATH_LEN];
    char address[36]; // set default value for address
    char address_copy[SCREEN_MAX_CHARS]; // copy of address to be then displayed as scrolling text (for UI needs only)
} getPublicKeyContext_t;

typedef struct {
    char custom_text_line_1[SCREEN_MAX_CHARS]; // text which will be displayed in custom_test_screen (e.g. " You received ")
    char custom_text_line_2[SCREEN_MAX_CHARS]; // text which will be displayed in custom_test_screen (e.g. "new transaction")

    unsigned int total_outputs; // total number of outputs in transaction
    unsigned int current_output; // currently processed output
    char current_output_display[SCREEN_MAX_CHARS]; // only for UI
    char info_line[SCREEN_MAX_CHARS];
    char out_address[36]; // output address(output)
    char amount[SCREEN_MAX_CHARS]; // amount of coint to send
    char out_address_or_amount[SCREEN_MAX_CHARS]; // output copy of address or amount(they will be changing each n second) to be then displayed as scrolling text (for UI needs only) 
} transactionContext_t;

//================================
// TXN
#define MAX_INPUTS 9
#define MAX_OUTPUTS 9

typedef struct {
    unsigned char address[21];
    uint64_t coin_num; // To get actual amount of coins, you should divide by 10^6
    uint64_t hour_num;
} txn_output_t;

typedef union {
    unsigned char input[36];
    unsigned char signature[65];
} sig_input_t;

typedef struct {
    uint8_t inner_hash[32];
    unsigned int in_num;
    sig_input_t sig_input[MAX_INPUTS];

    unsigned int out_num;
    txn_output_t outputs[MAX_OUTPUTS];
} txn_t;

typedef enum {
    TXN_PARTIAL = 1,
    TXN_PARTIAL_OUT,
    TXN_START_IN,
    TXN_IN,
    TXN_START_OUT,
    TXN_OUT,
    TXN_READY,
    TXN_COMPUTE_SIGS,
    TXN_RET_SIGS,
    TXN_ERROR,
    TXN_FINISHED
} txn_state_t;

//=========================

typedef struct {
    bool initialized;
    cx_sha256_t hash;
    txn_state_t txn_state;
    txn_t txn;

    unsigned char buffer[37];
    unsigned char offset;
    unsigned char curr_obj;


    uint8_t *dataBuffer;
    uint16_t dataLength;
    volatile unsigned int *flags;
    volatile unsigned int *tx;
//    bool is_approved;
} signTxnContext_t;

typedef struct {
    getPublicKeyContext_t getPublicKeyContext;
    transactionContext_t transactionContext;
    signTxnContext_t signTxnContext;
} commandContext;

extern commandContext global;
extern ux_state_t ux;


// ********************************************************************************
// Defines for easier UI structures usage
// ********************************************************************************
#define DEFAULT_FONT BAGL_FONT_OPEN_SANS_REGULAR_11px | BAGL_FONT_ALIGNMENT_CENTER // default font
#define BOLD_FONT BAGL_FONT_OPEN_SANS_EXTRABOLD_11px | BAGL_FONT_ALIGNMENT_CENTER
#define COLOR_WHITE 0xFFFFFF // White color (hex)
#define SCREEN_MAX_CHARS 16 // Maximum number of characters to be displayed on screen(for scrolling text only)

#define UI_BACKGROUND() {{BAGL_RECTANGLE,0,0,0,128,32,0,0,BAGL_FILL,0,COLOR_WHITE,0,0},NULL,0,0,0,NULL,NULL,NULL}
#define UI_ICON_LEFT(userid, glyph) {{BAGL_ICON,userid,3,12,7,7,0,0,0,COLOR_WHITE,0,0,glyph},NULL,0,0,0,NULL,NULL,NULL}
#define UI_ICON_RIGHT(userid, glyph) {{BAGL_ICON,userid,117,13,8,6,0,0,0,COLOR_WHITE,0,0,glyph},NULL,0,0,0,NULL,NULL,NULL}
#define UI_TEXT_CUSTOM_FONT(userid, x, y, w, text, font) {{BAGL_LABELINE,userid,x,y,w,12,0,0,0,COLOR_WHITE,0,font,0},(char *)text,0,0,0,NULL,NULL,NULL}
#define UI_TEXT(userid, x, y, w, text) {{BAGL_LABELINE,userid,x,y,w,12,0,0,0,COLOR_WHITE,0,DEFAULT_FONT,0},(char *)text,0,0,0,NULL,NULL,NULL}
#define UI_BOLD_TEXT(userid, x, y, w, text) {{BAGL_LABELINE,userid,x,y,w,12,0,0,0,COLOR_WHITE,0,BOLD_FONT,0},(char *)text,0,0,0,NULL,NULL,NULL}

#endif
