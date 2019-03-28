#include "newTransaction.h"

#include "main_ui.h"
#include "ux.h"
#include "sky.h"

#include "string.h"

#include "os_io_seproxyhal.h"

// UI struct for screen with custom screen (e.g. You got new TX)
const bagl_element_t bagl_custom_text[] = {
    UI_BACKGROUND(),
    UI_TEXT(0x83, 15, 12, 100, global.transactionContext.custom_text_line_1),
    UI_TEXT(0x83, 15, 27, 100, global.transactionContext.custom_text_line_2)
};

// Handler for buttons pressed action
unsigned int bagl_custom_text_button(unsigned int button_mask, unsigned int button_mask_counter) {
	switch (button_mask) {
        case BUTTON_EVT_RELEASED | BUTTON_LEFT | BUTTON_RIGHT:
            //screen_printf("button trigger at custom text screen \n");
            UX_MENU_DISPLAY(0, menu_main, NULL);

            // TODO: go to the screen with input address
            // probably need to "reimplement" code from sign transaction function    
            break;
        }

	return 0;
}

void go_to_custom_text_screen(unsigned char *first_line, unsigned int first_size, unsigned char *second_line, unsigned int second_size){
    if(first_line)
        os_memmove(global.transactionContext.custom_text_line_1, first_line, MIN(first_size, SCREEN_MAX_CHARS));

    if(second_line)
        os_memmove(global.transactionContext.custom_text_line_2, second_line, MIN(second_size, SCREEN_MAX_CHARS));

    UX_DISPLAY(bagl_custom_text, NULL);   
}
