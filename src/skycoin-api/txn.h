#ifndef LEDGER_NANO_TXN_H
#define LEDGER_NANO_TXN_H

#include "../ux.h"
#include "../skycoin-api/skycoin_crypto.h"
#include "../apdu_handlers.h"

txn_state_t txn_next_elem(signTxnContext_t *ctx);

void read_data_to_buffer(unsigned char buffer_size);

void save_data_to_buffer();

#endif //LEDGER_NANO_TXN_H
