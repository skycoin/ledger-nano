#include "main_ui.h"
#include "ux.h"
#include "sky.h"

#include "string.h"

#include "os_io_seproxyhal.h"


// UI struct for screen with address(derived from pk)
static const bagl_element_t bagl_ui_address[] = {
    UI_BACKGROUND(),
    UI_TEXT(0x83, 16, 20, 100, global.getPublicKeyContext.address_copy),
    UI_ICON_LEFT(0x00, BAGL_GLYPH_ICON_CROSS)
};

// Handler for buttons pressed action
static unsigned int bagl_ui_address_button(unsigned int button_mask, unsigned int button_mask_counter) {
	switch (button_mask) {
        case BUTTON_EVT_RELEASED | BUTTON_LEFT:
            UX_MENU_DISPLAY(0, menu_main, NULL);
            break;
        }

	return 0;
}

// Helper function to be used later for scrolling text 
void swap_str_elems(void* str, int from, int to){
    char tmp = ((char *) str)[from];
    ((char *) str)[to] = ((char *) str)[from];
    ((char *) str)[from] = tmp;
}

// Preprocessor for address screen
// Each iteration it "moves" the text of `address` so it looks like scrolling-text
int current_offset, direction;
unsigned int ui_address_scrolling_text_prepro(const bagl_element_t *element) {
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

void go_to_address_screen(unsigned int userid){
    // initialize variables for updating the UI each interval 
    ux_step = 0; ux_step_count = 4;
    // Initialize variables for working with scrolling text
    current_offset = 0; direction = 1;
        
    UX_DISPLAY(bagl_ui_address, ui_address_scrolling_text_prepro);   
}