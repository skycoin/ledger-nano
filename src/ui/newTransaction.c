#include "newTransaction.h"

#include "string.h"

// UI struct for screen with custom screen (e.g. You got new TX)
const bagl_element_t bagl_custom_text[] = {
        UI_BACKGROUND(),

        UI_ICON_LEFT(0x01, BAGL_GLYPH_ICON_CROSS),
        UI_ICON_RIGHT(0x02, BAGL_GLYPH_ICON_CHECK),

        UI_TEXT_CUSTOM_FONT(0x11, 4, 10, 20, global.transactionContext.current_output_display,
                            BAGL_FONT_OPEN_SANS_REGULAR_11px | BAGL_FONT_ALIGNMENT_LEFT),

        UI_TEXT(0x83, 23, 27, 85, global.transactionContext.custom_text_line_2),
        UI_TEXT(0x80, 20, 12, 88, global.transactionContext.custom_text_line_1)
};

void prepare_output_approval() {
    signTxnContext_t *ctx = &global.signTxnContext;

    screen_printf("need to approve\n");

    global.transactionContext.total_outputs = ctx->txn.out_num;
    global.transactionContext.current_output = ctx->curr_obj;
    prepare_current_output_for_display();

    screen_printf("current_output_display: %s\n", global.transactionContext.current_output_display);

    char address[36];
    txn_output_t *cur_out = &ctx->txn.outputs[ctx->curr_obj - 1];
    address_to_base58(cur_out->address, address);
    os_memmove(global.transactionContext.out_address, address, strlen(address) + 1);

    screen_printf("address: %s\n", global.transactionContext.out_address);

    char tmp_amount[SCREEN_MAX_CHARS];
    SPRINTF(tmp_amount, "%d.%d", ctx->txn.outputs[ctx->curr_obj - 1].coin_num / 1000,
            ctx->txn.outputs[ctx->curr_obj - 1].coin_num % 1000);
    os_memmove(global.transactionContext.amount, tmp_amount, strlen(tmp_amount) + 1);

    screen_printf("amount: %s\n", global.transactionContext.amount);


//    if (ctx->curr_obj == ctx->txn.out_num) {
//        ctx->txn_state = TXN_READY;
//        ctx->curr_obj = 0;
//    }
    // os_memmove(global.transactionContext.info_line, "Address\0", 8);
    // os_memmove(global.transactionContext.out_address_or_amount, global.transactionContext.out_address, 27);

    screen_printf("5\n");
}

// Handler for buttons pressed action
unsigned int bagl_custom_text_button(unsigned long button_mask, unsigned long button_mask_counter) {
    switch (button_mask) {
        case BUTTON_EVT_RELEASED | BUTTON_LEFT:
            screen_printf("Cancel TX signing process\n");

            ui_idle();
            break;

        case BUTTON_EVT_RELEASED | BUTTON_RIGHT:
        case BUTTON_EVT_RELEASED | BUTTON_LEFT | BUTTON_RIGHT: {
            screen_printf("Right button was pressed\n");

            signTxnContext_t *ctx = &global.signTxnContext;

            os_memmove(global.transactionContext.custom_text_line_1, "Transaction\0", 12);
            os_memmove(global.transactionContext.custom_text_line_2, "processing...\0", 14);

            screen_printf("\ninitialized %d\n\n", ctx->initialized);
            // go_to_custom_text_screen("You received\0", 13, "new transaction\0", 16);

            if (!ctx->initialized) {
                screen_printf("!ctx->initialized \n");

                cx_sha256_init(&ctx->hash);

                screen_printf("state: %d\n", global.signTxnContext.txn_state);
                ctx->txn_state = TXN_START_IN;
                screen_printf("state: %d\n", global.signTxnContext.txn_state);

                ctx->initialized = true;
                ctx->offset = 0;
            }

            screen_printf("state: %d\n", global.signTxnContext.txn_state);

            switch (txn_next_elem(ctx)) {
                case TXN_PARTIAL:
                    screen_printf("need new data\n");
                    io_async_exchange_ok();
                    break;
                case TXN_OUT:
                    prepare_output_approval();
                    break;
                case TXN_PARTIAL_OUT:
                    prepare_output_approval();
                    io_async_exchange_ok();
                    break;
                case TXN_READY:
                    screen_printf("is ready\n");
//                screen_printf("Transaction is valid\n");
//                PRINTF("Inner hash %.*h\n", SHA256_HASH_LEN, ctx->txn.inner_hash);
                    os_memmove(G_io_apdu_buffer, ctx->txn.inner_hash, SHA256_HASH_LEN);
                    *ctx->tx += SHA256_HASH_LEN;

                    ctx->txn_state = TXN_COMPUTE_SIGS;
                    io_async_exchange_ok();
                    break;
                case TXN_ERROR:
                    screen_printf("Transaction is invalid\n");
                    ctx->initialized = false;
                    io_async_exchange_error();
                    break;
            }

            UX_REDISPLAY()
            break;
        }
    }

    return 0;
}

unsigned int custom_screen_prepro(const bagl_element_t *element) {
    // 83 -> out_address_or_amount line1
    // 80 -> info_line line2
    if (global.signTxnContext.txn_state == TXN_OUT) {
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
            os_memcpy(element->text, global.transactionContext.out_address + current_offset, SCREEN_MAX_CHARS - 1);
            ((char *) element->text)[SCREEN_MAX_CHARS] = '\0';

//            screen_printf("element->text: %s\n", element->text);

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

        // prepare_current_output_for_display();

    } else { // it should be just a plain text screen, so hide icons
        if (global.signTxnContext.txn_state == TXN_READY) {
            if (global.signTxnContext.curr_obj == 0) {
                os_memmove(global.transactionContext.custom_text_line_1, "Approve\0", 8);
                os_memmove(global.transactionContext.custom_text_line_2, "transaction\0", 12);

                if (element->component.userid == 0x11) {
                    return NULL;
                } else {
                    UX_CALLBACK_SET_INTERVAL(SCROLLING_TEXT_DELAY);
                }
            } else {

                os_memmove(global.transactionContext.custom_text_line_1, "Transaction\0", 12);
                os_memmove(global.transactionContext.custom_text_line_2, "is signing...\0", 14);

                if (element->component.userid == 0x01 || element->component.userid == 0x02 ||
                    element->component.userid == 0x11) {
                    return NULL;
                } else {
                    UX_CALLBACK_SET_INTERVAL(SCROLLING_TEXT_DELAY);
                }

            }
        } else {
            if (element->component.userid == 0x01 || element->component.userid == 0x02 ||
                element->component.userid == 0x11) {
                return NULL;
            } else {
                UX_CALLBACK_SET_INTERVAL(SCROLLING_TEXT_DELAY);
            }
        }
    }

    return 1;
}

void go_to_custom_text_screen(unsigned char *first_line, unsigned int first_size, unsigned char *second_line,
                              unsigned int second_size) {
    if (first_line)
        os_memmove(global.transactionContext.custom_text_line_1, first_line, MIN(first_size, SCREEN_MAX_CHARS));

    if (second_line)
        os_memmove(global.transactionContext.custom_text_line_2, second_line, MIN(second_size, SCREEN_MAX_CHARS));

    // initialize variables for updating the UI each interval
    ux_step = 0;
    ux_step_count = 4;

    // Initialize variables for working with scrolling text
    current_offset = 0;
    direction = 1;

    // os_memmove(global.transactionContext.out_address, "12345678976543234567876543\0", 27);
    // os_memmove(global.transactionContext.amount, "123.45\0", 7);

    // os_memmove(global.transactionContext.info_line, "Address\0", 8);
    // os_memmove(global.transactionContext.out_address_or_amount, &global.transactionContext.out_address,
    //            SCREEN_MAX_CHARS);

    // global.transactionContext.current_output = 1;
    // global.transactionContext.total_outputs = 5;

    if (global.signTxnContext.txn_state == TXN_OUT) {
        prepare_current_output_for_display();
    } else {
        os_memmove(global.transactionContext.current_output_display, "\0", 1);
    }

    UX_DISPLAY(bagl_custom_text, custom_screen_prepro);
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
//            volatile unsigned int *tx = global.signTxnContext.tx;
//            G_io_apdu_buffer[(*tx)++] = 0x90;
//            G_io_apdu_buffer[(*tx)++] = 0x00;
//            io_exchange(CHANNEL_APDU | IO_RETURN_AFTER_TX, *tx);
            io_async_exchange_ok(*global.signTxnContext.tx);
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

    os_memmove(global.transactionContext.current_output_display, tmp, strlen(tmp) + 1);
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
