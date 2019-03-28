#ifndef LEDGER_NANO_TXN_H
#define LEDGER_NANO_TXN_H

#define MAX_INPUTS 9
#define MAX_OUTPUTS 9

typedef struct {
    unsigned char address[21];
    uint64_t coin_num; // To get actual amount of coins, you should divide by 10^6
    uint64_t hour_num;
} txn_output_t;

typedef union {
    unsigned char input[36];
    unsigned char signature[65];
} sig_input_t;

typedef struct {
    uint8_t inner_hash[32];
    unsigned int in_num;
    sig_input_t sig_input[MAX_INPUTS];

    unsigned int out_num;
    txn_output_t outputs[MAX_OUTPUTS];
} txn_t;

typedef enum {
    TXN_START_IN,
    TXN_IN,
    TXN_START_OUT,
    TXN_OUT,
    TXN_READY,
    TXN_RET_SIGS,
    TXN_ERROR
} txn_state_t;

#endif //LEDGER_NANO_TXN_H
