#include "info_message.h"

static char info_text[SCREEN_MAX_CHARS];

// UI struct for screen with custom screen (e.g. You got new TX)
const bagl_element_t bagl_info_text[] = {
        UI_BACKGROUND(),
        UI_BOLD_TEXT(0x80, 20, 17, 88, info_text)
};

// Handler for buttons pressed action
unsigned int bagl_info_text_button(unsigned long button_mask, unsigned long button_mask_counter) {
    switch (button_mask) {
        case BUTTON_EVT_RELEASED | BUTTON_LEFT:
        case BUTTON_EVT_RELEASED | BUTTON_RIGHT:
        case BUTTON_EVT_RELEASED | BUTTON_LEFT | BUTTON_RIGHT:
            ui_idle();
            break;
    }
    return 0;
}

void show_message(unsigned char *info_line_text, unsigned int text_size) {
    os_memmove(info_text, info_line_text, text_size);
    UX_DISPLAY(bagl_info_text, NULL);
}
