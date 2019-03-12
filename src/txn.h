#ifndef LEDGER_NANO_TXN_H
#define LEDGER_NANO_TXN_H

#define MAX_INPUTS 10
#define MAX_OUTPUTS 8

typedef struct {
    unsigned char address[21];
    uint64_t coin_num; // To get actual amount of coins, you should divide by 10^6
    uint64_t hour_num;
} txn_output_t;

typedef struct {
    unsigned int len;
    unsigned char type;
    uint8_t inner_hash[32];

    unsigned int sig_num;
    unsigned char sigs[MAX_INPUTS][65];

    unsigned int in_num;
    unsigned char inputs[MAX_INPUTS][32];

    unsigned int out_num;
    txn_output_t outputs[MAX_OUTPUTS];
} txn_t;

typedef enum {
    TXN_STARTED,
    TXN_START_SIG,
    TXN_SIG,
    TXN_START_IN,
    TXN_IN,
    TXN_START_OUT,
    TXN_OUT,
    TXN_READY,
    TXN_ERROR
} txn_state_t;

#endif //LEDGER_NANO_TXN_H
