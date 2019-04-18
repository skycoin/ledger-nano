#include "txn.h"

static signTxnContext_t *ctx = &global.signTxnContext;

#define U8LE(buf, off) (((uint64_t)(U4LE(buf, off + 4)) << 32) | ((uint64_t)(U4LE(buf, off))     & 0xFFFFFFFF))


void save_data_to_buffer() {
    os_memmove(ctx->buffer, ctx->dataBuffer, ctx->dataLength);
    ctx->offset = ctx->dataLength;
    ctx->dataBuffer += ctx->dataLength;
    ctx->dataLength = 0;
}

void read_data_to_buffer(unsigned char buffer_size) {
    os_memmove(ctx->buffer + ctx->offset, ctx->dataBuffer, buffer_size - ctx->offset);
    ctx->dataBuffer += buffer_size - ctx->offset;
    ctx->dataLength -= buffer_size - ctx->offset;
    ctx->offset = 0;
}

static void __txn_next_elem(signTxnContext_t *ctx) {
    switch (ctx->txn_state) {
        case TXN_START_IN: {
            screen_printf("START_IN\n");
            if (ctx->offset + ctx->dataLength < 4) {
                save_data_to_buffer();
                THROW(TXN_PARTIAL);
            } else {
                read_data_to_buffer(4);

                cx_hash(&ctx->hash.header, 0, ctx->buffer, 4, NULL);
                ctx->txn.in_num = U4LE(ctx->buffer, 0);

                ctx->txn_state = TXN_IN;
                ctx->curr_obj = 0;
            }
            break;
        }
        case TXN_IN: {
            // 32 bytes are input + 4 bytes address_index of BIP44 address
            screen_printf("TXN_IN dataLength: %d\n", ctx->dataLength);
            screen_printf("TXN_IN cur_ob: %d\n\n", ctx->curr_obj);

            if (ctx->offset + ctx->dataLength < 36) {
                save_data_to_buffer();
                THROW(TXN_PARTIAL);
            } else {
                read_data_to_buffer(36);

                cx_hash(&ctx->hash.header, 0, ctx->buffer, 32, NULL);
                os_memmove(ctx->txn.sig_input[ctx->curr_obj].input, ctx->buffer, 36);

                PRINTF("input %.*H\n", 36, ctx->txn.sig_input[ctx->curr_obj].input);
                ctx->curr_obj += 1;
            }
            if (ctx->curr_obj == ctx->txn.in_num) {
                ctx->txn_state = TXN_START_OUT;
            }
            break;
        }
        case TXN_START_OUT: {
            screen_printf("TXN_START_OUT dataLength: %d\n", ctx->dataLength);

            if (ctx->dataLength < 4) {
                save_data_to_buffer();
                THROW(TXN_PARTIAL);
            } else {
                read_data_to_buffer(4);

                cx_hash(&ctx->hash.header, 0, ctx->buffer, 4, NULL);
                ctx->txn.out_num = U4LE(ctx->buffer, 0);

                ctx->txn_state = TXN_OUT;
                ctx->curr_obj = 0;
            }
            break;
        }
        case TXN_OUT: {
            screen_printf("TXT_OUT dataLength: %d\n", ctx->dataLength);

            if (ctx->curr_obj == ctx->txn.out_num) {
                ctx->txn_state = TXN_READY;
                ctx->curr_obj = 0;
                THROW(TXN_READY);
            }
            // 37 bytes are input
            if (ctx->offset + ctx->dataLength < 37) {
                save_data_to_buffer();
                THROW(TXN_PARTIAL);
            } else {
                read_data_to_buffer(37);

                txn_output_t *cur_out = &ctx->txn.outputs[ctx->curr_obj];
                os_memmove(cur_out->address, ctx->buffer, 21);

                cur_out->coin_num = U8LE(ctx->buffer, 21);
                cur_out->hour_num = U8LE(ctx->buffer, 29);

                ctx->curr_obj += 1;

                screen_printf("Cur object %d %d\n", ctx->curr_obj, ctx->dataLength);
                if (ctx->curr_obj == ctx->txn.out_num) {
                    cx_hash(&ctx->hash.header, CX_LAST, ctx->buffer, 37, ctx->txn.inner_hash);
                } else {
                    cx_hash(&ctx->hash.header, 0, ctx->buffer, 37, NULL);
                }

                if (ctx->offset + ctx->dataLength < 37 && ctx->txn_state != TXN_READY) {
                    save_data_to_buffer();
                    THROW(TXN_PARTIAL_OUT);
                } else {
                    THROW(TXN_OUT);
                }
            }
            break;
        }
        case TXN_COMPUTE_SIGS: {
//                screen_printf("\nNumber of inputs %u\n", ctx->txn.in_num);
            unsigned int old_bip44 = global.getPublicKeyContext.bip44_path[4];
            for (unsigned int i = 0; i < ctx->txn.in_num; i++) {
//                    PRINTF("    Input %.*h\n", SHA256_HASH_LEN, ctx->txn.sig_input[i].input);

                static cx_sha256_t hash;
                unsigned char sig_hash[SHA256_HASH_LEN];
                cx_ecfp_private_key_t private_key;

                cx_sha256_init(&hash);
                cx_hash(&hash.header, 0, ctx->txn.inner_hash, SHA256_HASH_LEN, NULL);
                cx_hash(&hash.header, CX_LAST, ctx->txn.sig_input[i].input, SHA256_HASH_LEN, sig_hash);

                unsigned int new_bip44 = U4LE(ctx->txn.sig_input[i].input, 32);
                global.getPublicKeyContext.bip44_path[4] = new_bip44;
                derive_keypair(global.getPublicKeyContext.bip44_path, &private_key, NULL);

                sign(&private_key, sig_hash, ctx->txn.sig_input[i].signature);
                ctx->curr_obj += 1;
            }
            global.getPublicKeyContext.bip44_path[4] = old_bip44;
//            screen_printf("\nNumber of outputs %u\n", ctx->txn.out_num);
//            for (unsigned int i = 0; i < ctx->txn.out_num; i++) {
//                char address[36];
//                txn_output_t *cur_out = &ctx->txn.outputs[i];
//                address_to_base58(cur_out->address, address);
//                screen_printf("    Output address %s\n", address);
//                screen_printf("    Number of coins %u\n", (unsigned long int) cur_out->coin_num);
//                screen_printf("    Number of hours %u\n\n", (unsigned long int) cur_out->hour_num);
//            }
//            screen_printf("\n\n");
            ctx->curr_obj = 0;
            ctx->txn_state = TXN_RET_SIGS;
        }
        case TXN_RET_SIGS: {
            while (ctx->curr_obj != ctx->txn.in_num) {
                if (*ctx->tx + SIG_LEN > 255) {
                    THROW(TXN_PARTIAL);
                } else {
                    os_memmove(G_io_apdu_buffer + *ctx->tx,
                               ctx->txn.sig_input[ctx->curr_obj++].signature,
                               SIG_LEN);
                    *ctx->tx += SIG_LEN;
                }
            }
            ctx->initialized = false;
            THROW(TXN_FINISHED);
            break;
        }
    }
}

txn_state_t txn_next_elem(signTxnContext_t *ctx) {
    // Like many transaction decoders, we use exceptions to jump out of deep
    // call stacks when we encounter an error. There are two important rules
    // for Ledger exceptions: declare modified variables as volatile, and do
    // not THROW(0). Presumably, 0 is the sentinel value for "no exception
    // thrown." So be very careful when throwing enums, since enums start at 0
    // by default.
    volatile txn_state_t result;
    BEGIN_TRY{
            TRY{
                    // read until we reach a displayable element or the end of the buffer
                    for (;;) {
                        __txn_next_elem(ctx);
                    }
            }
            CATCH_OTHER(e) {
                result = e;
            }
            FINALLY {
            }
    }
    END_TRY;
    return result;
}
