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

void handleGetSignedPublicKey(uint8_t p1, uint8_t p2, uint8_t *dataBuffer, uint16_t dataLength, volatile unsigned int *flags,
                        volatile unsigned int *tx) {
    cx_ecfp_public_key_t public_key;
    cx_ecfp_private_key_t private_key;
    unsigned int bip44_path[BIP44_PATH_LEN];

    get_bip44_path(dataBuffer, bip44_path);
    derive_keypair(bip44_path, &private_key, &public_key);

    unsigned char public_key_compressed[COMPRESSED_PK_LEN];
    compress_public_key(public_key.W, public_key_compressed);
    
    unsigned char public_key_hash[SHA256_HASH_LEN];
    cx_sha256_t pubkey_hasher;
    cx_sha256_init(&pubkey_hasher);
    
    cx_hash(&pubkey_hasher.header, CX_LAST, public_key.W, PK_LEN, public_key_hash);
    unsigned char signature[PK_LEN];

    sign(&private_key, public_key_hash, signature);

    os_memmove(G_io_apdu_buffer, signature, PK_LEN);
    *tx += PK_LEN;
    
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
    os_memmove(G_io_apdu_buffer, public_key.W, PK_LEN);
    *tx += PK_LEN;

    THROW(INS_RET_SUCCESS);
}

void handleGetAddress(uint8_t p1, uint8_t p2, uint8_t *dataBuffer, uint16_t dataLength, volatile unsigned int *flags,
                      volatile unsigned int *tx) {
    cx_ecfp_public_key_t public_key;
    unsigned int bip44_path[BIP44_PATH_LEN];

    get_bip44_path(dataBuffer, bip44_path);
    derive_keypair(bip44_path, NULL, &public_key);

    generate_address(public_key.W, global.getPublicKeyContext.address);

    // push the address onto the response buffer.
    os_memmove(G_io_apdu_buffer, global.getPublicKeyContext.address, strlen(global.getPublicKeyContext.address));
    *tx += strlen(global.getPublicKeyContext.address);

    THROW(INS_RET_SUCCESS);
}
