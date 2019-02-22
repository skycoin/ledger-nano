#ifndef SKY_H
#define SKY_H

#include "os.h"
#include "cx.h"
#include "ui.h"
#include <stdbool.h>
#include "os_io_seproxyhal.h"


/** displays the address in base58 */
void display_public_key(const unsigned char * public_key);

#endif