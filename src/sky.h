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

/** get address in base58 format */
void address_to_base58(const unsigned char *address, char *result);

/** derives public and private keys from the given BIP44 path */
void derive_keypair(unsigned int bip44_path[], cx_ecfp_private_key_t *private_key, cx_ecfp_public_key_t *public_key);

/** generates the address in base58 */
void generate_address(const unsigned char * public_key, unsigned char *dst);

#endif