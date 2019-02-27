#ifndef SKYCOIN_APDU_HANDLERS_H
#define SKYCOIN_APDU_HANDLERS_H

#include "os.h"
#include "sky.h"
#include "ux.h"

/** Instructions start **/
#define INS_RESET 0x00 // instruction to reset
#define INS_GET_VERSION 0x01 // instruction to send back version on program
#define INS_GET_ADDRESS 0x02 // instruction to send back the address
#define INS_GET_PUBLIC_KEY 0x04 // instruction to send back the public key
#define INS_GET_SIGNED_PUBLIC_KEY 0x08 // instruction to send back the public key, and a signature of the private key signing the public key
#define INS_RET_SUCCESS 0x9000 // instruction to send back the public key, and a signature of the private key signing the public key
/** Instructions end */

/** Offsets of different pars of ADPU. */
/** start of the buffer, reject any transmission that doesn't start with this, as it's invalid. */
#define CLA          0x80
#define OFFSET_CLA   0x00
#define OFFSET_INS   0x01
#define OFFSET_P1    0x02
#define OFFSET_P2    0x03
#define OFFSET_LC    0x04
#define OFFSET_CDATA 0x05

/** General defines for APDU and BIP44 parts */
#define APDU_HEADER_LENGTH 5 // length of the APDU (application protocol data unit) header
#define APDU_BODY_LENGTH_OFFSET 4 // offset in the APDU header which says the length of the body


typedef void handler_fn_t(
        uint8_t p1,
        uint8_t p2,
        uint8_t *dataBuffer,
        uint16_t dataLength,
        volatile unsigned int *flags,
        volatile unsigned int *tx
);

handler_fn_t handleGetVersion;
handler_fn_t handleGetPublicKey;
handler_fn_t handleGetSignedPublicKey;
handler_fn_t handleGetAddress;

handler_fn_t *lookupHandler(uint8_t ins);

void handleGetPublicKey(uint8_t p1, uint8_t p2, uint8_t *dataBuffer, uint16_t dataLength, volatile unsigned int *flags,
                        volatile unsigned int *tx);

void handleGetVersion(uint8_t p1, uint8_t p2, uint8_t *dataBuffer, uint16_t dataLength, volatile unsigned int *flags,
                      volatile unsigned int *tx);

void handleGetSignedPublicKey(uint8_t p1, uint8_t p2, uint8_t *dataBuffer, uint16_t dataLength, volatile unsigned int *flags,
                         volatile unsigned int *tx);

void handleGetAddress(uint8_t p1, uint8_t p2, uint8_t *dataBuffer, uint16_t dataLength, volatile unsigned int *flags,
                      volatile unsigned int *tx);

#endif //SKYCOIN_APDU_HANDLERS_H

