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
    *dataLength -= buffer_size - ctx->offset;
    ctx->offset = 0;
}

void handleSignTxn(uint8_t p1, uint8_t p2, uint8_t *dataBuffer, uint16_t dataLength, volatile unsigned int *flags,
                   volatile unsigned int *tx) {
    if (!ctx->initialized) {
        cx_sha256_init(&ctx->hash);
        ctx->txn_state = TXN_START_IN;
        ctx->initialized = true;
        ctx->offset = 0;
    }
    while (dataLength || ctx->txn_state == TXN_READY || ctx->txn_state == TXN_ERROR) {
        switch (ctx->txn_state) {
            case TXN_START_IN: {
                if (ctx->offset + dataLength < 4) {
                    save_data_to_buffer(&dataBuffer, &dataLength);
                } else {
                    read_data_to_buffer(&dataBuffer, &dataLength, 4);

                    cx_hash(&ctx->hash.header, 0, ctx->buffer, 4, NULL);
                    ctx->txn.in_num = U4LE(ctx->buffer, 0);

                    ctx->txn_state = TXN_IN;
                    ctx->curr_obj = 0;
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
                    os_memmove(ctx->txn.sig_input[ctx->curr_obj].input, ctx->buffer, 32);
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
                        cx_hash(&ctx->hash.header, CX_LAST, ctx->buffer, 37, ctx->txn.inner_hash);
                        ctx->txn_state = TXN_READY;
                    } else {
                        cx_hash(&ctx->hash.header, 0, ctx->buffer, 37, NULL);
                    }
                }
                break;
            }
            case TXN_READY: {
                screen_printf("Transaction is valid\n");

                PRINTF("Inner hash %.*h\n", SHA256_HASH_LEN, ctx->txn.inner_hash);
                os_memmove(G_io_apdu_buffer, ctx->txn.inner_hash, SHA256_HASH_LEN);
                *tx += SHA256_HASH_LEN;
                screen_printf("\nNumber of inputs %u\n", ctx->txn.in_num);
                for (unsigned int i = 0; i < ctx->txn.in_num; i++) {
                    PRINTF("    Input %.*h\n", SHA256_HASH_LEN, ctx->txn.sig_input[i].input);

                    static cx_sha256_t hash;
                    unsigned char sig_hash[SHA256_HASH_LEN];
                    cx_ecfp_private_key_t private_key;

                    screen_printf("Bip44 path: ");
                    for (int i = 0; i < BIP44_PATH_LEN; i++) {
                        screen_printf("%x ", global.getPublicKeyContext.bip44_path[i]);
                    }
                    screen_printf("\n");

                    cx_sha256_init(&hash);
                    cx_hash(&hash.header, 0, ctx->txn.inner_hash, SHA256_HASH_LEN, NULL);
                    cx_hash(&hash.header, CX_LAST, ctx->txn.sig_input[i].input, SHA256_HASH_LEN, sig_hash);

                    derive_keypair(global.getPublicKeyContext.bip44_path, &private_key, NULL);

                    unsigned char signature[SIG_LEN];

                    sign(&private_key, sig_hash, ctx->txn.sig_input[i].signature);

                    PRINTF("    Signature %.*h\n\n", SIG_LEN, ctx->txn.sig_input[i].signature);
                    os_memmove(G_io_apdu_buffer + *tx, ctx->txn.sig_input[i].signature, SIG_LEN);
                    *tx += SIG_LEN;
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
                screen_printf("\n\n");

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
