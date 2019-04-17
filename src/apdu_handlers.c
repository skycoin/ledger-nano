#include <inttypes.h>

#include "apdu_handlers.h"


//#define U8LE(buf, off) (((uint64_t)(U4LE(buf, off + 4)) << 32) | ((uint64_t)(U4LE(buf, off))     & 0xFFFFFFFF))

static signTxnContext_t *ctx = &global.signTxnContext;

handler_fn_t *lookupHandler(uint8_t ins) {
    switch (ins) {
        case INS_GET_VERSION:
            return handleGetVersion;
        case INS_GET_PUBLIC_KEY:
            return handleGetPublicKey;
        case INS_GET_SIGNED_PUBLIC_KEY:
            return handleGetSignedPublicKey;
        case INS_GET_ADDRESS:
            return handleGetAddress;
        case INS_SIGN_TRANSACTION:
            return handleSignTxn;
        default:
            return NULL;
    }
}

void
handleGetSignedPublicKey(uint8_t p1, uint8_t p2, uint8_t *dataBuffer, uint16_t dataLength, volatile unsigned int *flags,
                         volatile unsigned int *tx) {
    cx_ecfp_public_key_t public_key;
    cx_ecfp_private_key_t private_key;

    get_bip44_path(dataBuffer);
    derive_keypair(global.getPublicKeyContext.bip44_path, &private_key, &public_key);

    unsigned char public_key_compressed[COMPRESSED_PK_LEN];
    compress_public_key(public_key.W, public_key_compressed);

    unsigned char public_key_hash[SHA256_HASH_LEN];
    cx_sha256_t pubkey_hasher;
    cx_sha256_init(&pubkey_hasher);

    cx_hash(&pubkey_hasher.header, CX_LAST, public_key.W, PK_LEN, public_key_hash);
    unsigned char signature[SIG_LEN];

    sign(&private_key, public_key_hash, signature);

    os_memmove(G_io_apdu_buffer, signature, SIG_LEN);
    *tx += SIG_LEN;

    THROW(INS_RET_SUCCESS);
}

void handleGetVersion(uint8_t p1, uint8_t p2, uint8_t *dataBuffer, uint16_t dataLength, volatile unsigned int *flags,
                      volatile unsigned int *tx) {
    G_io_apdu_buffer[0] = APPVERSION[0] - '0';
    G_io_apdu_buffer[1] = APPVERSION[2] - '0';
    G_io_apdu_buffer[2] = APPVERSION[4] - '0';
    *tx += 3;
    THROW(INS_RET_SUCCESS);
}

void handleGetPublicKey(uint8_t p1, uint8_t p2, uint8_t *dataBuffer, uint16_t dataLength, volatile unsigned int *flags,
                        volatile unsigned int *tx) {
    cx_ecfp_public_key_t public_key;
//    unsigned int bip44_path[BIP44_PATH_LEN];

    get_bip44_path(dataBuffer);
    derive_keypair(global.getPublicKeyContext.bip44_path, NULL, &public_key);

    // push the public key onto the response buffer.
    os_memmove(G_io_apdu_buffer, public_key.W, PK_LEN);
    *tx += PK_LEN;

    THROW(INS_RET_SUCCESS);
}

void handleGetAddress(uint8_t p1, uint8_t p2, uint8_t *dataBuffer, uint16_t dataLength, volatile unsigned int *flags,
                      volatile unsigned int *tx) {
    cx_ecfp_public_key_t public_key;

    get_bip44_path(dataBuffer);
    derive_keypair(global.getPublicKeyContext.bip44_path, NULL, &public_key);

    generate_address(public_key.W, global.getPublicKeyContext.address);

    // push the address onto the response buffer.
    os_memmove(G_io_apdu_buffer, global.getPublicKeyContext.address, strlen(global.getPublicKeyContext.address));
    *tx += strlen(global.getPublicKeyContext.address);

    THROW(INS_RET_SUCCESS);
}

void save_data_to_buffer(uint8_t **dataBuffer, uint16_t *dataLength) {
    os_memmove(ctx->buffer, *dataBuffer, *dataLength);
    ctx->offset = *dataLength;
    *dataBuffer += *dataLength;
    *dataLength = 0;
}

void read_data_to_buffer(uint8_t **dataBuffer, uint16_t *dataLength, unsigned char buffer_size) {
    os_memmove(ctx->buffer + ctx->offset, *dataBuffer, buffer_size - ctx->offset);
    *dataBuffer += buffer_size - ctx->offset;
    ctx->dataBuffer += buffer_size - ctx->offset;
    *dataLength -= buffer_size - ctx->offset;
    ctx->offset = 0;
}

int parseTxn(uint8_t *dataBuffer, uint16_t *dataLength, volatile unsigned int *tx, volatile unsigned int *flags) {
    // screen_printf("Start dataLength: %d\n", *dataLength);
    int fl = 1;
    int cur_out_num = 0;

    while (*dataLength || ctx->txn_state == TXN_READY || ctx->txn_state == TXN_ERROR ||
           ctx->txn_state == TXN_RET_SIGS) {
        switch (ctx->txn_state) {
            case TXN_START_IN: {
//                screen_printf("START_IN\n");
                //    screen_printf("dataLength: %d\n", *dataLength);
                if (ctx->offset + *dataLength < 4) {
                    save_data_to_buffer(&dataBuffer, dataLength);
                } else {
                    read_data_to_buffer(&dataBuffer, dataLength, 4);

                    cx_hash(&ctx->hash.header, 0, ctx->buffer, 4, NULL);
                    ctx->txn.in_num = U4LE(ctx->buffer, 0);

                    ctx->txn_state = TXN_IN;
                    ctx->curr_obj = 0;
                }
                break;
            }
            case TXN_IN: {
                // 32 bytes are input + 4 bytes address_index of BIP44 address
//                screen_printf("TXN_IN dataLength: %d\n", *dataLength);

                if (ctx->offset + *dataLength < 36) {
                    save_data_to_buffer(&dataBuffer, dataLength);
                    // screen_printf("data has been saved to the buffer, dataLength: %d\n", *dataLength);

                } else {
//                    screen_printf("IN\n");
                    // screen_printf("dataLength: %d\n", *dataLength);
                    read_data_to_buffer(&dataBuffer, dataLength, 36);

                    cx_hash(&ctx->hash.header, 0, ctx->buffer, 32, NULL);
                    os_memmove(ctx->txn.sig_input[ctx->curr_obj].input, ctx->buffer, 36);
                    ctx->curr_obj += 1;
                }
                if (ctx->curr_obj == ctx->txn.in_num) {
                    ctx->txn_state = TXN_START_OUT;
                }
                break;
            }
            case TXN_START_OUT: {
//                screen_printf("TXN_START_OUT dataLength: %d\n", *dataLength);

                if (*dataLength < 4) {
                    save_data_to_buffer(&dataBuffer, dataLength);
                } else {
                    // screen_printf("START_OUT\n");
                    read_data_to_buffer(&dataBuffer, dataLength, 4);

                    cx_hash(&ctx->hash.header, 0, ctx->buffer, 4, NULL);
                    ctx->txn.out_num = U4LE(ctx->buffer, 0);

                    ctx->txn_state = TXN_OUT;
                    ctx->curr_obj = 0;
                }

                // screen_printf("END_OUT\n");

                break;
            }
            case TXN_OUT: {
//                screen_printf("TXT_OUT dataLength: %d\n", *dataLength);

                // 37 bytes are input
                if (ctx->offset + *dataLength < 37) {
                    save_data_to_buffer(&dataBuffer, dataLength);
                } else {
                    //    screen_printf("OUT\n");
                    read_data_to_buffer(&dataBuffer, dataLength, 37);

                    txn_output_t *cur_out = &ctx->txn.outputs[ctx->curr_obj];
                    os_memmove(cur_out->address, ctx->buffer, 21);

                    cur_out->coin_num = U8LE(ctx->buffer, 21);
                    cur_out->hour_num = U8LE(ctx->buffer, 29);

                    ctx->curr_obj += 1;

//                    screen_printf("\nNumber of outputs %u\n", ctx->txn.out_num);
//                    screen_printf("Cur object %d %d\n", ctx->curr_obj, *dataLength);
                    if (ctx->curr_obj == ctx->txn.out_num) {
                        cx_hash(&ctx->hash.header, CX_LAST, ctx->buffer, 37, ctx->txn.inner_hash);
                        ctx->txn_state = TXN_READY;
//                        cur_out_num = 0;
//                        io_async_exchange_ok();
//                        show_output_confirmation();
                    } else {
                        cx_hash(&ctx->hash.header, 0, ctx->buffer, 37, NULL);
                    }
                    fl = 0;
                    cur_out_num += 1;
                }
                break;
            }
            case TXN_READY: {
//                screen_printf("Transaction is valid\n");

//                PRINTF("Inner hash %.*h\n", SHA256_HASH_LEN, ctx->txn.inner_hash);
                os_memmove(G_io_apdu_buffer, ctx->txn.inner_hash, SHA256_HASH_LEN);
                *tx += SHA256_HASH_LEN;
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

//                    PRINTF("    Signature %.*h\n\n", SIG_LEN, ctx->txn.sig_input[i].signature);
                }
                global.getPublicKeyContext.bip44_path[4] = old_bip44;
//                screen_printf("\nNumber of outputs %u\n", ctx->txn.out_num);
//                for (unsigned int i = 0; i < ctx->txn.out_num; i++) {
//                    char address[36];
//                    txn_output_t *cur_out = &ctx->txn.outputs[i];
//                    address_to_base58(cur_out->address, address);
//                    screen_printf("    Output address %s\n", address);
//                    screen_printf("    Number of coins %u\n", (unsigned long int) cur_out->coin_num);
//                    screen_printf("    Number of hours %u\n\n", (unsigned long int) cur_out->hour_num);
//                }
//                screen_printf("\n\n");
                ctx->curr_obj = 0;
                ctx->txn_state = TXN_RET_SIGS;
                break;
            }
            case TXN_RET_SIGS: {
//                screen_printf("\n Transaction is valid\n");
                int offset = ctx->curr_obj == 0 ? SHA256_HASH_LEN : 0;
                while (ctx->curr_obj != ctx->txn.in_num) {
                    if (offset + SIG_LEN > 255) {
                        THROW(0x6199);
                    }
                    os_memmove(G_io_apdu_buffer + offset,
                               ctx->txn.sig_input[ctx->curr_obj++].signature,
                               SIG_LEN);
                    *tx += SIG_LEN;
                    offset += SIG_LEN;
                }
                ctx->initialized = false;
                ctx->tx = tx;
                show_output_confirmation();
                *flags |= IO_ASYNCH_REPLY;
//                THROW(INS_RET_SUCCESS);
                break;
            }
            case TXN_ERROR: {
//                screen_printf("Transaction is invalid\n");
                ctx->initialized = false;
                THROW(0x6B00);
                break;
            }
        }
    }

    if (cur_out_num) {
//        io_async_exchange_ok();
//        screen_printf("need to confirm\n");
    }

//    screen_printf("After state machine, dataLength: %d\n", *dataLength);

    return fl;
}

void io_exchange_with_code(uint16_t code, uint16_t tx) {
    G_io_apdu_buffer[tx++] = code >> 8;
    G_io_apdu_buffer[tx++] = code & 0xFF;
    io_exchange(CHANNEL_APDU | IO_RETURN_AFTER_TX, tx);
}

void io_async_exchange_ok() {
    io_exchange_with_code(INS_RET_SUCCESS, *ctx->tx);
}

void io_async_exchange_error() {
    io_exchange_with_code(0x6B0, 0);
}


void handleSignTxn(uint8_t p1, uint8_t p2, uint8_t *dataBuffer, uint16_t dataLength, volatile unsigned int *flags,
                   volatile unsigned int *tx) {

    screen_printf("\n    Start handler %d\n", ctx->initialized);
    ctx->dataBuffer = dataBuffer;
    ctx->dataLength = dataLength;
    ctx->flags = flags;
    ctx->tx = tx;
    *tx = 0;
    screen_printf("    Start tx %d\n\n", *ctx->tx);

    if (!ctx->initialized) {
        go_to_custom_text_screen("You received\0", 13, "new transaction\0", 16);
    } else {
        if (ctx->txn_state < TXN_START_OUT || ctx->txn_state == TXN_RET_SIGS) {
            switch (txn_next_elem(ctx)) {
                case TXN_ERROR:
                    ctx->initialized = false;
                    io_async_exchange_error();
                    break;
                case TXN_OUT:
                    screen_printf("need to approve\n");
                    os_memmove(global.transactionContext.custom_text_line_1, "Some skycoin\0", 13);
                    os_memmove(global.transactionContext.custom_text_line_2, "coins\0", 6);
                    UX_DISPLAY(bagl_custom_text, NULL);
                    break;
                case TXN_PARTIAL:
                    io_async_exchange_ok();
                    break;
                case TXN_FINISHED:
                    io_async_exchange_ok();
                    ui_idle();
                    ctx->initialized = false;
                    break;
                default:
                    io_async_exchange_ok();
            }
        } else {
            UX_DISPLAY(bagl_custom_text, NULL);
        }
    }
    *ctx->flags |= IO_ASYNCH_REPLY;
}
