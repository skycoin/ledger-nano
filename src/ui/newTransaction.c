#include "newTransaction.h"

#include "string.h"

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
            parseTxn(global.signTxnContext.dataBuffer, &global.signTxnContext.dataLength, global.signTxnContext.tx,
                     global.signTxnContext.flags);

            if (!global.signTxnContext.dataLength) {
                screen_printf("custom text screen : response OK\n");
                io_async_exchange_ok();
            }

            ui_idle(); // TODO: show loading screen here

            break;
    }

    return 0;
}

void go_to_custom_text_screen(unsigned char *first_line, unsigned int first_size, unsigned char *second_line,
                              unsigned int second_size) {
    if (first_line)
        os_memmove(global.transactionContext.custom_text_line_1, first_line, MIN(first_size, SCREEN_MAX_CHARS));

    if (second_line)
        os_memmove(global.transactionContext.custom_text_line_2, second_line, MIN(second_size, SCREEN_MAX_CHARS));

    UX_DISPLAY(bagl_custom_text, NULL);
}


const bagl_element_t bagl_output_confirmation_screen[] = {
        UI_BACKGROUND(),
        UI_ICON_LEFT(0x01, BAGL_GLYPH_ICON_CROSS),
        UI_ICON_RIGHT(0x02, BAGL_GLYPH_ICON_CHECK),

        UI_TEXT_CUSTOM_FONT(0x11, 4, 10, 20, global.transactionContext.current_output_display,
                            BAGL_FONT_OPEN_SANS_REGULAR_11px | BAGL_FONT_ALIGNMENT_LEFT),

        UI_TEXT(0x83, 23, 30, 85, global.transactionContext.out_address_or_amount),
        UI_BOLD_TEXT(0x80, 20, 12, 88, global.transactionContext.info_line)
};


unsigned int bagl_output_confirmation_screen_button(unsigned int button_mask, unsigned int button_mask_counter) {
    switch (button_mask) {
        case BUTTON_EVT_RELEASED | BUTTON_LEFT:
            screen_printf("confirmation: NO \n");

            UX_MENU_DISPLAY(0, menu_main, NULL);

            // TODO: Return Error, that txn failed
            // TODO: cancel the whole signing process 
            break;

        case BUTTON_EVT_RELEASED | BUTTON_RIGHT:
            screen_printf("confirmation: YES \n");
//            global.signTxnContext.is_approved = true;
//            UX_MENU_DISPLAY(0, menu_main, NULL);

//            parseTxn(global.signTxnContext.dataBuffer, &global.signTxnContext.dataLength, global.signTxnContext.tx,
//                     global.signTxnContext.flags);
//            if (!global.signTxnContext.dataLength) {
            screen_printf("output : response OK\n");
//                io_async_exchange_ok();
            volatile unsigned int *tx = global.signTxnContext.tx;
            G_io_apdu_buffer[(*tx)++] = 0x90;
            G_io_apdu_buffer[(*tx)++] = 0x00;
            io_exchange(CHANNEL_APDU | IO_RETURN_AFTER_TX, *tx);
//            UX_MENU_DISPLAY(0, menu_main, NULL);
//            ui_idle(); // TODO: show loading screen here
//            }


            // TODO: go to the another output or finish with that if no any more
            break;
    }

    return 0;
}

// Preprocessor for output confirmation screen
// we have 2 lines: info(Address or SKY) and description(actual/specific value)
// if currently we are displaying address, then we wait until it goes forward and backward 
// and then switch to the screen with SKY, where we just display amount of SKY(statically) 
unsigned int output_confirmation_screen_prepro(const bagl_element_t *element) {
    if (element->component.userid == 0x80) { // info line
        strcpy(element->text, "Address:\0");

        if (current_offset == -1) {
            strcpy(element->text, "SKY:\0");
            UX_CALLBACK_SET_INTERVAL(SCROLLING_TEXT_BIG_DELAY * 3);
        } else {
            UX_CALLBACK_SET_INTERVAL(SCROLLING_TEXT_DELAY);
        }
    }

    if (element->component.userid == 0x83) { // description line (out_address_or_amount)
        strcpy(element->text, global.transactionContext.out_address + current_offset);
        ((char *) element->text)[SCREEN_MAX_CHARS] = '\0';

        current_offset += direction;

        // firstly we update current_offset, then wait and after that copy and change the string 
        // so check equality with -1, but not with 0
        // the same with (current_offset + SCREEN_MAX_CHARS) and strlen
        if (current_offset == -1 ||
            (current_offset + SCREEN_MAX_CHARS == (strlen(global.transactionContext.out_address) + 1))) {
            direction *= -1;

            if (direction == 1) { // change text to the amount of skycoins and wait much longer
                strcpy(element->text, global.transactionContext.amount);
                UX_CALLBACK_SET_INTERVAL(SCROLLING_TEXT_BIG_DELAY * 3);
            } else {
                UX_CALLBACK_SET_INTERVAL(SCROLLING_TEXT_BIG_DELAY); // wait more if we change direction
            }
        } else {
            UX_CALLBACK_SET_INTERVAL(SCROLLING_TEXT_DELAY);
        }
    }

    return 1;
}

void prepare_current_output_for_display() {
    os_memset(global.transactionContext.current_output_display, 0, SCREEN_MAX_CHARS);
    char tmp[SCREEN_MAX_CHARS];
    SPRINTF(tmp, "%d/%d", global.transactionContext.current_output, global.transactionContext.total_outputs);

    os_memmove(global.transactionContext.current_output_display, tmp, strlen(tmp));
}

void show_output_confirmation() {
    // initialize variables for updating the UI each interval 
    ux_step = 0;
    ux_step_count = 4;

    // Initialize variables for working with scrolling text
    current_offset = 0;
    direction = 1;

    os_memmove(global.transactionContext.out_address, "12345678976543234567876543\0", 27);
    os_memmove(global.transactionContext.amount, "123.45\0", 7);

    os_memmove(global.transactionContext.info_line, "Address\0", 8);
    os_memmove(global.transactionContext.out_address_or_amount, global.transactionContext.out_address, 27);

    global.transactionContext.current_output = 1;
    global.transactionContext.total_outputs = 5;
    prepare_current_output_for_display();

    screen_printf("before display confirmation screen\n");
    screen_printf("current_output_display: %s\n", global.transactionContext.current_output_display);
    screen_printf("out_address_or_amount: %s\n", global.transactionContext.out_address_or_amount);
    screen_printf("info_line: %s\n", global.transactionContext.info_line);

    UX_DISPLAY(bagl_output_confirmation_screen, output_confirmation_screen_prepro);
}
