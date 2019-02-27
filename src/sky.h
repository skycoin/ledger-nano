#ifndef SKY_H
#define SKY_H

#include "os.h"
#include "cx.h"
#include "ux.h"
#include <stdbool.h>
#include "os_io_seproxyhal.h"



/** encode number in base58 format */
int encode_base_58(const unsigned char *pbegin, int len, char *result);

/** get address from public key */
static void to_address(const unsigned char *public_key_compressed, char *result);

/** displays the address in base58 */
void display_address(const unsigned char * public_key, unsigned char *dst);

#endif