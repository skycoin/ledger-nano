#include "ui.h"

#include "os_io_seproxyhal.h"

/** default font */
#define DEFAULT_FONT BAGL_FONT_OPEN_SANS_REGULAR_11px | BAGL_FONT_ALIGNMENT_CENTER

/** text description font. */
#define TX_DESC_FONT BAGL_FONT_OPEN_SANS_REGULAR_11px | BAGL_FONT_ALIGNMENT_CENTER

/** UI state flag */
ux_state_t ux;

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

/** UI state enum */
enum UI_STATE uiState;

/** notification to refresh the view, if we are displaying the public key */
unsigned char publicKeyNeedsRefresh;

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

const ux_menu_entry_t menu_settings[] = {
    {menu_main, NULL, 0, NULL, "No settings yet", NULL, 0, 0},
    {menu_main, NULL, 1, &C_nanos_icon_back, "Back", NULL, 61, 40},
    UX_MENU_END};

const ux_menu_entry_t menu_about[] = {
    {NULL, NULL, 0, NULL, "Version", APPVERSION, 0, 0},
    {menu_main, NULL, 1, &C_nanos_icon_back, "Back", NULL, 61, 40},
    UX_MENU_END};

const ux_menu_entry_t menu_main[] = {
    //{NULL, NULL, 0, &NAME3(C_nanos_badge_, COINID, ), "Use wallet to", "view
    // accounts", 33, 12},
    {NULL, NULL, 0, NULL, "Use wallet to", "view accounts", 0, 0},
    {menu_settings, NULL, 0, NULL, "Settings", NULL, 0, 0},
    {menu_about, NULL, 0, NULL, "About", NULL, 0, 0},
    {NULL, os_sched_exit, 0, &C_nanos_icon_dashboard, "Quit app", NULL, 50, 29},
    UX_MENU_END};


/**
 * buttons for the idle screen
 *
 * exit on Left button, or on Both buttons. Do nothing on Right button only.
 */
static unsigned int bagl_ui_idle_nanos_button(unsigned int button_mask, unsigned int button_mask_counter) {
	switch (button_mask) {
	case BUTTON_EVT_RELEASED | BUTTON_RIGHT:
		// ui_public_key_1();
		break;
	case BUTTON_EVT_RELEASED | BUTTON_LEFT:
		io_seproxyhal_touch_exit(NULL);
		break;
	}

	return 0;
}

static unsigned int
bagl_ui_sample_nanos_button(unsigned int button_mask,
                            unsigned int button_mask_counter) {
    switch (button_mask) {
    case BUTTON_EVT_RELEASED | BUTTON_LEFT | BUTTON_RIGHT: // EXIT
        io_seproxyhal_touch_exit(NULL);
        break;
    }
    return 0;
}

static const bagl_element_t *io_seproxyhal_touch_exit(const bagl_element_t *e) {
    // Go back to the dashboard
    os_sched_exit(0);
    return NULL;
}

void ui_idle(void) {
    UX_MENU_DISPLAY(0, menu_main, NULL);
}
