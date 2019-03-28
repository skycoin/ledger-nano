#ifndef SKYCOIN_NEW_TRANSACTION_H
#define SKYCOIN_NEW_TRANSACTION_H

#include "main_ui.h"
#include "ux.h"
#include "sky.h"

#include "string.h"

#include "os_io_seproxyhal.h"

// UI struct for screen with custom screen (e.g. You got new TX)
extern const bagl_element_t bagl_custom_text[3];
// ...
extern const bagl_element_t bagl_output_confirmation_screen[5];


// Handler for buttons pressed action
unsigned int bagl_custom_text_button(unsigned int button_mask, unsigned int button_mask_counter);
void go_to_custom_text_screen(unsigned char *first_line, unsigned int first_size, unsigned char *second_line, unsigned int second_size);

// ...
unsigned int bagl_output_confirmation_screen_button(unsigned int button_mask, unsigned int button_mask_counter);
void show_output_confirmation();

#endif
