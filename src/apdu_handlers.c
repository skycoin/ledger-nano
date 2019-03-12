#include <inttypes.h>
#include "apdu_handlers.h"

#include "sky.h"
#include "ux.h"
#include "cx.h"
#include "util_funcs.h"

#define U8LE(buf, off) (((uint64_t)(U4LE(buf, off + 4)) << 32) | ((uint64_t)(U4LE(buf, off))     & 0xFFFFFFFF))

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
    unsigned int bip44_path[BIP44_PATH_LEN];

    get_bip44_path(dataBuffer, bip44_path);
    derive_keypair(bip44_path, &private_key, &public_key);

    unsigned char result[32];
    cx_sha256_t pubKeyHash;
    cx_sha256_init(&pubKeyHash);

    cx_hash(&pubKeyHash.header, CX_LAST, public_key.W, 65, result);
    *tx += cx_ecdsa_sign((void *) &private_key, CX_RND_RFC6979 | CX_LAST, CX_SHA256, result, sizeof(result),
                         G_io_apdu_buffer + *tx, NULL);

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
    unsigned int bip44_path[BIP44_PATH_LEN];

    get_bip44_path(dataBuffer, bip44_path);
    derive_keypair(bip44_path, NULL, &public_key);

    // push the public key onto the response buffer.
    os_memmove(G_io_apdu_buffer, public_key.W, 65);
    *tx += 65;

    THROW(INS_RET_SUCCESS);
}

void handleGetAddress(uint8_t p1, uint8_t p2, uint8_t *dataBuffer, uint16_t dataLength, volatile unsigned int *flags,
                      volatile unsigned int *tx) {
    cx_ecfp_public_key_t public_key;
    unsigned int bip44_path[BIP44_PATH_LEN];

    get_bip44_path(dataBuffer, bip44_path);
    derive_keypair(bip44_path, NULL, &public_key);

    generate_address(public_key.W, global.getPublicKeyContext.address);
    screen_printf("Public key: %s\n", public_key.W);
    screen_printf("Address: %s\n", global.getPublicKeyContext.address);

    // push the address onto the response buffer.
    os_memmove(G_io_apdu_buffer, global.getPublicKeyContext.address, 35);
    *tx += 35;

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
    *dataLength -= buffer_size - ctx->offset;
    ctx->offset = 0;
}

void handleSignTxn(uint8_t p1, uint8_t p2, uint8_t *dataBuffer, uint16_t dataLength, volatile unsigned int *flags,
                   volatile unsigned int *tx) {
    if (!ctx->initialized) {
        cx_sha256_init(&ctx->hash);
        ctx->txn_state = TXN_STARTED;
        ctx->initialized = true;
        ctx->offset = 0;
    }
    while (dataLength || ctx->txn_state == TXN_READY || ctx->txn_state == TXN_ERROR) {
        switch (ctx->txn_state) {
            case TXN_STARTED: {
                read_data_to_buffer(&dataBuffer, &dataLength, 37);

                ctx->txn.len = U4LE(ctx->buffer, 0);
                ctx->txn.type = ctx->buffer[4];
                os_memmove(ctx->txn.inner_hash, ctx->buffer + 5, 32);

                ctx->txn_state = TXN_START_SIG;
                break;
            }
            case TXN_START_SIG: {
                read_data_to_buffer(&dataBuffer, &dataLength, 4);
                ctx->txn.sig_num = U4LE(ctx->buffer, 0);

                ctx->txn_state = TXN_SIG;
                ctx->curr_obj = 0;
                break;
            }
            case TXN_SIG: {
                // 65 bytes are signature
                if (ctx->offset + dataLength < 65) {
                    save_data_to_buffer(&dataBuffer, &dataLength);
                } else {
                    read_data_to_buffer(&dataBuffer, &dataLength, 65);

                    os_memmove(ctx->txn.sigs[ctx->curr_obj], ctx->buffer, 65);
                    ctx->curr_obj += 1;
                }
                if (ctx->curr_obj == ctx->txn.sig_num) {
                    ctx->txn_state = TXN_START_IN;
                }
                break;
            }
            case TXN_START_IN: {
                if (ctx->offset + dataLength < 4) {
                    save_data_to_buffer(&dataBuffer, &dataLength);
                } else {
                    read_data_to_buffer(&dataBuffer, &dataLength, 4);

                    cx_hash(&ctx->hash.header, 0, ctx->buffer, 4, NULL);
                    ctx->txn.in_num = U4LE(ctx->buffer, 0);

                    ctx->txn_state = TXN_IN;
                    ctx->curr_obj = 0;
                    if (ctx->txn.in_num != ctx->txn.sig_num) {
                        screen_printf("Number of inputs and signatures is not equal\n%u %u\n", ctx->txn.in_num,
                                      ctx->txn.sig_num);
                        ctx->txn_state = TXN_ERROR;
                    }
                }
                break;
            }
            case TXN_IN: {
                // 32 bytes are input
                if (ctx->offset + dataLength < 32) {
                    save_data_to_buffer(&dataBuffer, &dataLength);
                } else {
                    read_data_to_buffer(&dataBuffer, &dataLength, 32);

                    cx_hash(&ctx->hash.header, 0, ctx->buffer, 32, NULL);
                    os_memmove(ctx->txn.inputs[ctx->curr_obj], ctx->buffer, 32);
                    ctx->curr_obj += 1;
                }
                if (ctx->curr_obj == ctx->txn.in_num) {
                    ctx->txn_state = TXN_START_OUT;
                }
                break;
            }
            case TXN_START_OUT: {
                if (dataLength < 4) {
                    save_data_to_buffer(&dataBuffer, &dataLength);
                } else {
                    read_data_to_buffer(&dataBuffer, &dataLength, 4);

                    cx_hash(&ctx->hash.header, 0, ctx->buffer, 4, NULL);
                    ctx->txn.out_num = U4LE(ctx->buffer, 0);

                    ctx->txn_state = TXN_OUT;
                    ctx->curr_obj = 0;
                }
                break;
            }
            case TXN_OUT: {
                // 37 bytes are input
                if (ctx->offset + dataLength < 37) {
                    save_data_to_buffer(&dataBuffer, &dataLength);
                } else {
                    read_data_to_buffer(&dataBuffer, &dataLength, 37);

                    txn_output_t *cur_out = &ctx->txn.outputs[ctx->curr_obj];
                    os_memmove(cur_out->address, ctx->buffer, 21);

                    cur_out->coin_num = U8LE(ctx->buffer, 21);
                    cur_out->hour_num = U8LE(ctx->buffer, 29);

                    ctx->curr_obj += 1;

                    if (ctx->curr_obj == ctx->txn.out_num) {
                        uint8_t real_inner_hash[32];

                        cx_hash(&ctx->hash.header, CX_LAST, ctx->buffer, 37, real_inner_hash);
                        PRINTF("INNER HASH %.*h\n", 32, real_inner_hash);

                        if (mem_equal(real_inner_hash, ctx->txn.inner_hash, 32)) {
                            ctx->txn_state = TXN_READY;
                        } else {
                            ctx->txn_state = TXN_ERROR;
                        }
                    } else {
                        cx_hash(&ctx->hash.header, 0, ctx->buffer, 37, NULL);
                    }
                }
                break;
            }
            case TXN_READY: {
                screen_printf("Transaction is valid\n");

                screen_printf("Len of txn: %u\n", ctx->txn.len);
                screen_printf("Type of txn: %c\n", ctx->txn.type);
                PRINTF("Inner hash %.*h\n", 32, ctx->txn.inner_hash);
                screen_printf("\nNumber of sigs %u\n", ctx->txn.sig_num);
                for (unsigned int i = 0; i < ctx->txn.sig_num; i++) {
                    PRINTF("    Signature %.*h\n", 65, ctx->txn.sigs[i]);
                }
                screen_printf("\nNumber of inputs %u\n", ctx->txn.in_num);
                for (unsigned int i = 0; i < ctx->txn.in_num; i++) {
                    PRINTF("    Input %.*h\n", 32, ctx->txn.inputs[i]);
                }
                screen_printf("\nNumber of outputs %u\n", ctx->txn.out_num);
                for (unsigned int i = 0; i < ctx->txn.out_num; i++) {
                    char address[36];
                    txn_output_t *cur_out = &ctx->txn.outputs[i];
                    address_to_base58(cur_out->address, address);
                    screen_printf("    Output address %s\n", address);
                    screen_printf("    Number of coins %u\n", (unsigned long int) cur_out->coin_num);
                    screen_printf("    Number of hours %u\n\n", (unsigned long int) cur_out->hour_num);
                }
                screen_printf("\n\n\n");

                ctx->initialized = false;
                THROW(INS_RET_SUCCESS);
            }
            case TXN_ERROR: {
                screen_printf("Transaction is invalid\n");
                ctx->initialized = false;
                THROW(0x6B00);
            }
        }
    }
    THROW(INS_RET_SUCCESS);
}
