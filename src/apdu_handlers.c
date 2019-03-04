#include "apdu_handlers.h"

#include "ux.h"

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

void handleSignTxn(uint8_t p1, uint8_t p2, uint8_t *dataBuffer, uint16_t dataLength, volatile unsigned int *flags,
                   volatile unsigned int *tx) {
    screen_printf("Init %d\n", ctx->initialized);
    if (!ctx->initialized) {
        ctx->txn_state = TXN_STARTED;
        ctx->initialized = true;
    }
    screen_printf("state %d\n", ctx->txn_state);
    while (dataLength) {
        switch (ctx->txn_state) {
            case TXN_STARTED: {
                // First 4 bytes are length of txn
                ctx->txn.len = U4LE(dataBuffer, 0);
                // Next byte is type of transaction
                ctx->txn.type = dataBuffer[4];
                // Next 32 bytes are inner hash
                os_memmove(ctx->txn.inner_hash, dataBuffer + 5, 32);
                dataBuffer += 37;
                dataLength -= 37;
                ctx->txn_state = TXN_START_SIG;
                break;
            }
            case TXN_START_SIG: {
                ctx->txn.sig_num = U4LE(dataBuffer, 0);
                screen_printf("Number of sigs %u\n", ctx->txn.sig_num);
                ctx->curr_obj = 0;
                ctx->offset = 0;
                dataBuffer += 4;
                dataLength -= 4;
                ctx->txn_state = TXN_SIG;
                break;
            }
            case TXN_SIG: {
                // 65 bytes are signature
                if (ctx->offset + dataLength < 65) {
                    os_memmove(ctx->buffer, dataBuffer, dataLength);
                    ctx->offset = dataLength;
                } else {
                    screen_printf("Offset %u\n", ctx->offset);
                    os_memmove(ctx->buffer + ctx->offset, dataBuffer, 65 - ctx->offset);
                    dataBuffer += 65 - ctx->offset;
                    dataLength -= 65 - ctx->offset;
                    ctx->offset = 0;

                    os_memmove(ctx->txn.sigs[ctx->curr_obj], ctx->buffer, 65);

                    ctx->offset = 0;
                    ctx->curr_obj += 1;
                }
                if (ctx->curr_obj == ctx->txn.sig_num) {
                    ctx->txn_state = TXN_START_IN;
                }
                break;
            }
            case TXN_IN: {
                if(ctx->offset != 0) {
                    ctx->txn_state = TXN_ERROR;
                }

                break;
            }
            case TXN_OUT: {

                break;
            }
            case TXN_READY: {

                break;
            }
            case TXN_ERROR: {
                screen_printf("Some problem in transaction happened");
                THROW(0x6666)
                break;
            }
        }
        if (ctx->txn_state == TXN_START_IN) {
            break;
        }
    }

    screen_printf("Len of txn: %u\n", ctx->txn.len);
    screen_printf("Type of txn: %c\n", ctx->txn.type);
    PRINTF("Inner hash %.*h\n", 32, ctx->txn.inner_hash);
    PRINTF("Signature %.*h\n", 65, ctx->txn.sigs[0]);

    THROW(INS_RET_SUCCESS);
}
