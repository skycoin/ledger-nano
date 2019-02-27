#include "ui.h"
#include "ux.h"
#include "sky.h"

#include "string.h"

#include "os_io_seproxyhal.h"

// /** notification to restart the hash */
// unsigned char hashTainted;

static const bagl_element_t *io_seproxyhal_touch_exit(const bagl_element_t *e);

// /** all text descriptions. */
// char tx_desc[MAX_TX_TEXT_SCREENS][MAX_TX_TEXT_LINES][MAX_TX_TEXT_WIDTH];

// /** currently displayed text description. */
// char curr_tx_desc[MAX_TX_TEXT_LINES][MAX_TX_TEXT_WIDTH];

// /** currently displayed public key */
// char current_public_key[MAX_TX_TEXT_LINES][MAX_TX_TEXT_WIDTH];

// /** currently displayed public address */
// char address[36];

/** copy of address(only for UI displaying the scrolling text) */
char address_copy[40];


// ********************************************************************************
// Defines for easier UI structures usage
// ********************************************************************************

/** default font */
#define DEFAULT_FONT BAGL_FONT_OPEN_SANS_REGULAR_11px | BAGL_FONT_ALIGNMENT_CENTER

/** text description font. */
#define TX_DESC_FONT BAGL_FONT_OPEN_SANS_REGULAR_11px | BAGL_FONT_ALIGNMENT_CENTER

/** White color (hex) */
#define COLOR_WHITE 0xFFFFFF

/** Maximum number of characters to be displayed on screen(for scrolling text only) */
#define SCREEN_MAX_CHARS 16

// ********************************************************************************
//
//                   Structures with predescribed UI elements
//
// ********************************************************************************


// ********************************************************************************
// UI struct for screen with address(derived from pk)
// ********************************************************************************

static const bagl_element_t bagl_ui_address[] = {
    {	{	BAGL_RECTANGLE, 0x00, 0, 0, 128, 32, 0, 0, BAGL_FILL, 0x000000, 0xFFFFFF, 0, 0 }, NULL, 0, 0, 0, NULL, NULL, NULL, },
    /* center text */
    {	{	BAGL_LABELINE, 0x83, 6, 20, 128, 12, 0x80|10, 0, 0, 0xFFFFFF, 0x000000, DEFAULT_FONT, 0 }, address_copy, 0, 0, 0, NULL, NULL, NULL, },
    /* left icon is a X */
    {	{	BAGL_ICON, 0x00, 3, 12, 7, 7, 0, 0, 0, 0xFFFFFF, 0x000000, 0, BAGL_GLYPH_ICON_CROSS }, NULL, 0, 0, 0, NULL, NULL, NULL, },
};

static unsigned int bagl_ui_address_button(unsigned int button_mask, unsigned int button_mask_counter) {
	switch (button_mask) {
        case BUTTON_EVT_RELEASED | BUTTON_LEFT:
            UX_MENU_DISPLAY(0, menu_main, NULL);
            break;
        }

	return 0;
}


void swap_str_elems(void* str, int from, int to){
    char tmp = ((char *) str)[from];
    ((char *) str)[to] = ((char *) str)[from];
    ((char *) str)[from] = tmp;
}

int current_offset, direction;
unsigned int simple_prepro(const bagl_element_t *element) {
    if(element->component.userid == 0x83) {
        strcpy(element->text, global.getPublicKeyContext.address + current_offset);
        ((char *) element->text) [SCREEN_MAX_CHARS] = '\0';        

        current_offset += direction;

        if(current_offset == 0 || (current_offset + SCREEN_MAX_CHARS == 40))
            direction *= -1;

        UX_CALLBACK_SET_INTERVAL(300);
    }

    return 1;
}

void go_to_address(unsigned int userid){
    // append and prepend a white space to the address
    ux_step = 0;
    ux_step_count = 4;

    current_offset = 0;
    direction = 1;
    char tmp_blank[] = "                                       \0";
    os_memmove(address_copy, tmp_blank, 40);
    
    UX_DISPLAY(bagl_ui_address, simple_prepro);   
}