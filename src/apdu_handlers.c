#include "apdu_handlers.h"

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
        default:
            return NULL;
    }
}

void handleGetPublicKey(uint8_t p1, uint8_t p2, uint8_t *dataBuffer, uint16_t dataLength, volatile unsigned int *flags,
                        volatile unsigned int *tx) {
    cx_ecfp_public_key_t public_key;

    unsigned int bip44_path[BIP44_PATH_LEN];
    uint32_t i;
    for (i = 0; i < BIP44_PATH_LEN; i++) {
        bip44_path[i] = (dataBuffer[0] << 24) | (dataBuffer[1] << 16) | (dataBuffer[2] << 8) |
                        (dataBuffer[3]);
        dataBuffer += 4;
    }
    
    derive_keypair(bip44_path, NULL, &public_key);

    // push the public key onto the response buffer.
    os_memmove(G_io_apdu_buffer, public_key.W, 65);
    *tx = 65;

    generate_address(public_key.W, global.getPublicKeyContext.address);
    screen_printf("Address: %s\n", global.getPublicKeyContext.address);

    THROW(INS_RET_SUCCESS);
}

void handleGetVersion(uint8_t p1, uint8_t p2, uint8_t *dataBuffer, uint16_t dataLength, volatile unsigned int *flags,
                      volatile unsigned int *tx) {
    G_io_apdu_buffer[0] = APPVERSION[0] - '0';
    G_io_apdu_buffer[1] = APPVERSION[2] - '0';
    G_io_apdu_buffer[2] = APPVERSION[4] - '0';
    THROW(INS_RET_SUCCESS);
}

void handleGetSignedPublicKey(uint8_t p1, uint8_t p2, uint8_t *dataBuffer, uint16_t dataLength, volatile unsigned int *flags,
                              volatile unsigned int *tx) {
}

void handleGetAddress(uint8_t p1, uint8_t p2, uint8_t *dataBuffer, uint16_t dataLength, volatile unsigned int *flags,
                      volatile unsigned int *tx) {
}
