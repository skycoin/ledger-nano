#ifndef SKYCOIN_NEW_TRANSACTION_H
#define SKYCOIN_NEW_TRANSACTION_H

#include "main_ui.h"
#include "../ux.h"
#include "../skycoin-api/txn.h"
#include "../skycoin-api/skycoin_crypto.h"
#include "../apdu_handlers.h"

#include "os_io_seproxyhal.h"

// UI struct for screen with custom screen (e.g. You got new TX)
extern const bagl_element_t transaction_screen[6];

void prepare_output_approval();

// Handler for buttons pressed action
unsigned int transaction_screen_button(unsigned long button_mask, unsigned long button_mask_counter);

unsigned int transaction_screen_prepro(const bagl_element_t *element);

void go_to_transaction_screen(unsigned char *first_line, unsigned int first_size, unsigned char *second_line,
                              unsigned int second_size);

#endif
