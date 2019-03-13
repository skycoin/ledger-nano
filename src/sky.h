#ifndef SKY_H
#define SKY_H

#include "os.h"
#include "cx.h"
#include "ux.h"
#include <stdbool.h>
#include "os_io_seproxyhal.h"

/** get bip44 address from data buffer */
void get_bip44_path(uint8_t *dataBuffer, unsigned int bip44_path[]);

/** encode number in base58 format */
int encode_base_58(const unsigned char *pbegin, int len, char *result);

/** get address from public key */
static void to_address(const unsigned char *public_key_compressed, char *result);

/** derives public and private keys from the given BIP44 path */
void derive_keypair(unsigned int bip44_path[], cx_ecfp_private_key_t *private_key, cx_ecfp_public_key_t *public_key);

/** convert public key from uncompressed to compressed (dst size -> 33 bytes) */
void compress_public_key(const unsigned char *public_key, unsigned char *dst);

/** generates the address in base58 */
void generate_address(const unsigned char *public_key, unsigned char *dst);

/** convert TLV representation of a signature to R|S format(applicable for skycoin) */
void convert_signature_from_TLV_to_RS(const unsigned char * tlv_signature, unsigned char *dst);

/** create the signature of a hash */
void sign(cx_ecfp_private_key_t *private_key, const unsigned char *hash, unsigned char *signature);

#endif