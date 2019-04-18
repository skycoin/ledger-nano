#ifndef SKYCOIN_NEW_TRANSACTION_H
#define SKYCOIN_NEW_TRANSACTION_H

#include "main_ui.h"
#include "../ux.h"
#include "../skycoin-api/txn.h"
#include "../skycoin-api/skycoin_crypto.h"
#include "../apdu_handlers.h"

#include "string.h"

#include "os_io_seproxyhal.h"

// UI struct for screen with custom screen (e.g. You got new TX)
extern const bagl_element_t bagl_custom_text[6];
// ...
extern const bagl_element_t bagl_output_confirmation_screen[6];

void prepare_output_approval();

// Handler for buttons pressed action
unsigned int bagl_custom_text_button(unsigned int button_mask, unsigned int button_mask_counter);
unsigned int custom_screen_prepro(const bagl_element_t *element);
void go_to_custom_text_screen(unsigned char *first_line, unsigned int first_size, unsigned char *second_line,
                              unsigned int second_size);

// ...
unsigned int bagl_output_confirmation_screen_button(unsigned int button_mask, unsigned int button_mask_counter);

void prepare_current_output_for_display();
void show_output_confirmation();


#endif
