#include "ui.h"
#include "sky.h"

#include "string.h"

#include "os_io_seproxyhal.h"

/** UI state flag */
ux_state_t ux;

// display stepped screens
unsigned int ux_step;
unsigned int ux_step_count;
uint8_t ux_loop_over_curr_element; // only for Nano S

/** notification to restart the hash */
unsigned char hashTainted;

unsigned char G_io_seproxyhal_spi_buffer[IO_SEPROXYHAL_BUFFER_SIZE_B];

static const bagl_element_t *io_seproxyhal_touch_exit(const bagl_element_t *e);

/** all text descriptions. */
char tx_desc[MAX_TX_TEXT_SCREENS][MAX_TX_TEXT_LINES][MAX_TX_TEXT_WIDTH];

/** currently displayed text description. */
char curr_tx_desc[MAX_TX_TEXT_LINES][MAX_TX_TEXT_WIDTH];

/** currently displayed public key */
char current_public_key[MAX_TX_TEXT_LINES][MAX_TX_TEXT_WIDTH];

/** currently displayed public address */
char address[36];

/** copy of address(only for UI displaying the scrolling text) */
char address_copy[40];

/** UI state enum */
enum UI_STATE uiState;

/** notification to refresh the view, if we are displaying the public key */
unsigned char publicKeyNeedsRefresh;


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
// Main menu with settings, about, version app and ability to quit the app
// ********************************************************************************
const ux_menu_entry_t menu_main[];
const ux_menu_entry_t menu_settings[];

void go_to_address(unsigned int userid);
const ux_menu_entry_t menu_settings[] = {
    {NULL, go_to_address, 0, NULL, "Address", NULL, 0, 0},
    {menu_main, NULL, 1, &C_nanos_icon_back, "Back", NULL, 61, 40},
    UX_MENU_END};

const ux_menu_entry_t menu_about[] = {
    {NULL, NULL, 0, NULL, "Version", APPVERSION, 0, 0},
    {menu_main, NULL, 1, &C_nanos_icon_back, "Back", NULL, 61, 40},
    UX_MENU_END};

const ux_menu_entry_t menu_main[] = {
    {NULL, NULL, 0, NULL, "Use wallet to", "view accounts", 0, 0},
    {menu_settings, NULL, 0, NULL, "Settings", NULL, 0, 0},
    {menu_about, NULL, 0, NULL, "About", NULL, 0, 0},
    {NULL, os_sched_exit, 0, &C_nanos_icon_dashboard, "Quit app", NULL, 50, 29},
    UX_MENU_END};


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

int current_offset;
int direction;
unsigned int simple_prepro(const bagl_element_t *element) {
    if(element->component.userid == 0x83) {
        // screen_printf("current_offset: %d\n", current_offset);
        // screen_printf("text: %s\n", element->text);

        if(current_offset < 5) {
            strcpy(element->text+(5-current_offset), address + current_offset);
        } else {
            strcpy(element->text, address + current_offset);
        }
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
    char tmp_blank[] = "                                       ";
    os_memmove(address_copy, tmp_blank, 40);
    
    UX_DISPLAY(bagl_ui_address, simple_prepro);
    
}

// override point, but nothing more to do
void io_seproxyhal_display(const bagl_element_t *element) {
    if ((element->component.type & (~BAGL_TYPE_FLAGS_MASK)) != BAGL_NONE) {
        io_seproxyhal_display_default((bagl_element_t *)element);
    }
}

static const bagl_element_t *io_seproxyhal_touch_exit(const bagl_element_t *e) {
    // Go back to the dashboard
    os_sched_exit(0);
    return NULL;
}

unsigned char io_event(unsigned char channel) {
    // nothing done with the event, throw an error on the transport layer if
    // needed

    // can't have more than one tag in the reply, not supported yet.
    switch (G_io_seproxyhal_spi_buffer[0]) {
    case SEPROXYHAL_TAG_FINGER_EVENT:
        UX_FINGER_EVENT(G_io_seproxyhal_spi_buffer);
        break;

    case SEPROXYHAL_TAG_BUTTON_PUSH_EVENT: // for Nano S
        UX_BUTTON_PUSH_EVENT(G_io_seproxyhal_spi_buffer);
        break;

    case SEPROXYHAL_TAG_DISPLAY_PROCESSED_EVENT:
        if (UX_DISPLAYED()) {
            // TODO perform actions after all screen elements have been
            // displayed
        } else {
            UX_DISPLAYED_EVENT();
        }
        break;

    case SEPROXYHAL_TAG_TICKER_EVENT:
        UX_TICKER_EVENT(G_io_seproxyhal_spi_buffer, {
            // don't redisplay if UX not allowed (pin locked in the common bolos
            // ux ?)
            if (ux_step_count && UX_ALLOWED) {
                // prepare next screen
                if(!ux_loop_over_curr_element) {
                    ux_step = (ux_step + 1) % ux_step_count;
                }
                // redisplay screen
                UX_REDISPLAY();
            }
        });
        break;

    // unknown events are acknowledged
    default:
        UX_DEFAULT_EVENT();
        break;
    }

    // close the event if not done previously (by a display or whatever)
    if (!io_seproxyhal_spi_is_status_sent()) {
        io_seproxyhal_general_status();
    }

    // command has been processed, DO NOT reset the current APDU transport
    return 1;
}

void ui_idle(void) {
    ux_step_count = 0;
    ux_loop_over_curr_element = 0;

    // set default value for address
    os_memmove(address, "No address generated yet           \0", 36);


    UX_MENU_DISPLAY(0, menu_main, NULL);
}
