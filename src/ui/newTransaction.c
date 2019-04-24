#include "newTransaction.h"

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
//    screen_printf("Preparing data for output approval screen\n");
    signTxnContext_t *ctx = &global.signTxnContext;

    // current output (e.g. "1/6")
    os_memset(global.transactionContext.current_output_display, 0, SCREEN_MAX_CHARS);
    char tmp_cur_out[SCREEN_MAX_CHARS];
    SPRINTF(tmp_cur_out, "%d/%d", ctx->curr_obj, ctx->txn.out_num);
    os_memmove(global.transactionContext.current_output_display, tmp_cur_out, strlen(tmp_cur_out) + 1);
    screen_printf("\ncurrent_output_display: %s\n", global.transactionContext.current_output_display);

    // address
    char address[36];
    txn_output_t *cur_out = &ctx->txn.cur_output;
    address_to_base58(cur_out->address, address);
    os_memmove(global.transactionContext.out_address, address, strlen(address) + 1);
    screen_printf("address: %s\n", global.transactionContext.out_address);

    // Amount of SKY to send
    char tmp_amount[SCREEN_MAX_CHARS];

    // we are getting the amount of SKY multiplied by 10^6 but have to show it as it was initially(with floating-point)
    // Ledger does not support floating point, so we need to get decimal and float part by using integer operations (/ and %)
    int decimal_amount = (int) (cur_out->coin_num / 1000000);
    int amount_mantis = (int) (cur_out->coin_num % 1000000);

    // the problem is when we got e.g. 3030000 (3.03 SKY),
    // then decimal part -> 3, mantis -> 30000
    SPRINTF(tmp_amount, "%d.%06d", decimal_amount, amount_mantis);
    // Delete zeros at the end
    while (tmp_amount[strlen(tmp_amount) - 1] == '0')
        tmp_amount[strlen(tmp_amount) - 1] = '\0';

    os_memmove(global.transactionContext.amount, tmp_amount, strlen(tmp_amount) + 1);
    screen_printf("amount: %s\n", global.transactionContext.amount);
}

// Handler for buttons pressed action
unsigned int bagl_custom_text_button(unsigned long button_mask, unsigned long button_mask_counter) {
    switch (button_mask) {
        case BUTTON_EVT_RELEASED | BUTTON_LEFT:
            screen_printf("Cancel TX signing process\n");

            io_async_exchange_error();
            ui_idle();
            break;

        case BUTTON_EVT_RELEASED | BUTTON_RIGHT:
        case BUTTON_EVT_RELEASED | BUTTON_LEFT | BUTTON_RIGHT: {
            signTxnContext_t *ctx = &global.signTxnContext;

            os_memmove(global.transactionContext.custom_text_line_1, "Transaction\0", 12);
            os_memmove(global.transactionContext.custom_text_line_2, "processing...\0", 14);

            if (!ctx->initialized) {
                cx_sha256_init(&ctx->hash);
                ctx->txn_state = TXN_START_IN;
                ctx->initialized = true;
                ctx->offset = 0;
            }
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

// Preprocessor for output confirmation screen
// we have 2 lines: info(Address or SKY) and description(actual/specific value)
// if currently we are displaying address, then we wait until it goes forward and backward
// and then switch to the screen with SKY, where we just display amount of SKY(statically)
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
        if (element->component.userid == 0x01 || element->component.userid == 0x02 ||
            element->component.userid == 0x11) {
            return NULL;
        } else {
            UX_CALLBACK_SET_INTERVAL(SCROLLING_TEXT_DELAY);
        }
    }

    return 1;
}

void go_to_custom_text_screen(unsigned char *first_line, unsigned int first_size, unsigned char *second_line,
                              unsigned int second_size) {
    os_memmove(global.transactionContext.custom_text_line_1, first_line, MIN(first_size, SCREEN_MAX_CHARS));
    os_memmove(global.transactionContext.custom_text_line_2, second_line, MIN(second_size, SCREEN_MAX_CHARS));

    ux_step = 0;
    ux_step_count = 4;

    // Initialize variables for working with scrolling text
    current_offset = 0;
    direction = 1;

    UX_DISPLAY(bagl_custom_text, custom_screen_prepro);
}
