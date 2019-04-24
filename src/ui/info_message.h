#ifndef SKYCOIN_CUSTOM_TEXT_H
#define SKYCOIN_CUSTOM_TEXT_H

#include "main_ui.h"
#include "../ux.h"

#include "os_io_seproxyhal.h"

extern const bagl_element_t bagl_info_text[2];

unsigned int bagl_info_text_button(unsigned long button_mask, unsigned long button_mask_counter);

void show_message(unsigned char *info_line_text, unsigned int text_size);

#endif //SKYCOIN_CUSTOM_TEXT_H
