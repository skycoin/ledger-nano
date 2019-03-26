#ifndef SKY_H
#define SKY_H

#include "os.h"
#include "cx.h"
#include "ux.h"
#include <stdbool.h>
#include "os_io_seproxyhal.h"

/** the length of a SHA256 hash */
#define SHA256_HASH_LEN 32
/** the length of public key */
#define PK_LEN 65
/** the length of a signature */
#define SIG_LEN 65
/** the length of a compressed public key */
#define COMPRESSED_PK_LEN 33
/** the length of a RIPMD160 hash */
#define RIPMD_HASH_LEN 20
/** checksum is first 4 bytes of sha256 */
#define CHECKSUM_LEN 4
/** length of address (RIPMD160 hash + VERSION byte + checksum) */
#define ADDRESS_LEN (RIPMD_HASH_LEN + 1 + CHECKSUM_LEN)
/** length can be between 27 and 34 bytes + \0 */
#define ADDRESS_BASE58_LEN 35
/** the current version in the address field */
#define ADDRESS_VERSION 0
/** array of base58 alphabet letters */
static const char BASE_58_ALPHABET[] = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";

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
void convert_signature_from_TLV_to_RS(const unsigned char *tlv_signature, unsigned char *dst);

/** create the signature of a hash */
void sign(cx_ecfp_private_key_t *private_key, const unsigned char *hash, unsigned char *signature);

#endif
