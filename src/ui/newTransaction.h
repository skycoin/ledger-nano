#ifndef SKYCOIN_NEW_TRANSACTION_H
#define SKYCOIN_NEW_TRANSACTION_H

#include "main_ui.h"
#include "../ux.h"
#include "../skycoin-api/txn.h"
#include "../skycoin-api/skycoin_crypto.h"
#include "../apdu_handlers.h"

#include "os_io_seproxyhal.h"

// UI struct for screen with custom screen (e.g. You got new TX)
extern const bagl_element_t bagl_custom_text[6];

void prepare_output_approval();

// Handler for buttons pressed action
unsigned int bagl_custom_text_button(unsigned long button_mask, unsigned long button_mask_counter);

unsigned int custom_screen_prepro(const bagl_element_t *element);

void go_to_custom_text_screen(unsigned char *first_line, unsigned int first_size, unsigned char *second_line,
                              unsigned int second_size);

#endif
