#include "newTransaction.h"


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
            // UX_MENU_DISPLAY(0, menu_main, NULL);
            show_output_confirmation();

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


const bagl_element_t bagl_output_confirmation_screen[] = {
    UI_BACKGROUND(),
    UI_ICON_LEFT(0x01, BAGL_GLYPH_ICON_CROSS),
    UI_ICON_RIGHT(0x02, BAGL_GLYPH_ICON_CHECK),
    UI_TEXT(0x00, 20, 12, 88, global.transactionContext.info_line),
    UI_BOLD_TEXT(0x83, 25, 27, 88, global.transactionContext.out_address_copy),
};

unsigned int bagl_output_confirmation_screen_button(unsigned int button_mask, unsigned int button_mask_counter) {
    switch (button_mask) {
        case BUTTON_EVT_RELEASED | BUTTON_LEFT:
            screen_printf("confirmation: NO \n");
            UX_MENU_DISPLAY(0, menu_main, NULL);
    
            // TODO: cancel the whole signing process 
            break;
        case BUTTON_EVT_RELEASED | BUTTON_RIGHT:
            screen_printf("confirmation: YES \n");
            UX_MENU_DISPLAY(0, menu_main, NULL);
        
            // TODO: go to the another output or finish with that if no any more  
            break;
        }

	return 0;
}

void show_output_confirmation(){
    os_memmove(global.transactionContext.out_address, "12345678976543234567876543\0", 27);
    os_memmove(global.transactionContext.out_address_copy, "98765434562134\0", 15);
    os_memmove(global.transactionContext.amount, "SKY: 1.23\0", 10);
    os_memmove(global.transactionContext.info_line, "Address:\0", 9);

    // os_memmove(global.transactionContext.current_output, 1, 4);
    // os_memmove(global.transactionContext.total_outputs, 5, 4);

    UX_DISPLAY(bagl_output_confirmation_screen, NULL);   
}