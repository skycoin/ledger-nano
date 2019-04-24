#include <inttypes.h>

#include "apdu_handlers.h"

static signTxnContext_t *ctx = &global.signTxnContext;
//char info_text[SCREEN_MAX_CHARS];

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
    screen_printf("\n    p1 %d      p2 %d\n", p1, p2);
    ctx->dataBuffer = dataBuffer;
    ctx->dataLength = dataLength;
    ctx->tx = tx;
    *tx = 0;


    if (!ctx->initialized && p1 == 0x0) {
        go_to_transaction_screen("You received\0", 13, "new transaction\0", 16);
    } else if (ctx->initialized && p1 == 0x1) {
        if (ctx->txn_state < TXN_START_OUT || ctx->txn_state == TXN_RET_SIGS || ctx->txn_state == TXN_COMPUTE_SIGS) {
            switch (txn_next_elem(ctx)) {
                case TXN_ERROR:
                    ctx->initialized = false;
                    io_async_exchange_error();
                    show_message("Error happened\0", 15);
                    break;
                case TXN_OUT:
                    prepare_output_approval();
                    UX_DISPLAY(transaction_screen, transaction_screen_prepro);
                    break;
                case TXN_PARTIAL:
                    io_async_exchange_ok();
                    break;
                case TXN_FINISHED:
                    ctx->initialized = false;
                    io_async_exchange_ok();
                    show_message("Success\0", 8);
                    break;
                default:
                    io_async_exchange_ok();
            }
        } else {
            UX_DISPLAY(transaction_screen, transaction_screen_prepro);
        }
    } else {
        ctx->initialized = false;
        io_async_exchange_error();
        show_message("Error happened\0", 15);
    }
    *flags |= IO_ASYNCH_REPLY;
}
