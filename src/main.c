#include "os.h"
#include "cx.h"

#include "os_io_seproxyhal.h"

#include "ui.h"

/** start of the buffer, reject any transmission that doesn't start with this, as it's invalid. */
#define CLA 0x80

/** #### instructions start #### **/

/** instruction to reset . */
#define INS_RESET 0x00

/** instruction to sign transaction and send back the signature. */
#define INS_SIGN 0x02

/** instruction to send back the public key. */
#define INS_GET_PUBLIC_KEY 0x04

/** instruction to send back the public key, and a signature of the private key signing the public key. */
#define INS_GET_SIGNED_PUBLIC_KEY 0x08

/** #### instructions end #### */


unsigned short io_exchange_al(unsigned char channel, unsigned short tx_len) {
    switch (channel & ~(IO_FLAGS)) {
    case CHANNEL_KEYBOARD:
        break;

    // multiplexed io exchange over a SPI channel and TLV encapsulated protocol
    case CHANNEL_SPI:
        if (tx_len) {
            io_seproxyhal_spi_send(G_io_apdu_buffer, tx_len);

            if (channel & IO_RESET_AFTER_REPLIED) {
                reset();
            }
            return 0; // nothing received from the master so far (it's a tx
                      // transaction)
        } else {
            return io_seproxyhal_spi_recv(G_io_apdu_buffer,
                                          sizeof(G_io_apdu_buffer), 0);
        }

    default:
        THROW(INVALID_PARAMETER);
    }
    return 0;
}


static void sky_main(void) {
    volatile unsigned int rx = 0;
    volatile unsigned int tx = 0;
    volatile unsigned int flags = 0;

    // DESIGN NOTE: the bootloader ignores the way APDU are fetched. The only
    // goal is to retrieve APDU.
    // When APDU are to be fetched from multiple IOs, like NFC+USB+BLE, make
    // sure the io_event is called with a
    // switch event, before the apdu is replied to the bootloader. This avoid
    // APDU injection faults.
    for (;;) {
        volatile unsigned short sw = 0;

        BEGIN_TRY {
            TRY {
                rx = tx;
                tx = 0; 
                // ensure no race in catch_other if io_exchange throws an error
                rx = io_exchange(CHANNEL_APDU | flags, rx);
                flags = 0;

                // no apdu received, well, reset the session, and reset the
                // bootloader configuration
                if (rx == 0) {
                    THROW(0x6982);
                }

                if (G_io_apdu_buffer[0] != CLA) {
                    THROW(0x6E00);
                }


                // check the second byte (0x01) for the instruction.
                switch (G_io_apdu_buffer[1]) {
                case INS_RESET: // reset
                    flags |= IO_RESET_AFTER_REPLIED;
                    THROW(0x9000);
                    break;

                // getting the public key
                case INS_GET_PUBLIC_KEY: {
                    cx_ecfp_public_key_t publicKey;
                    cx_ecfp_private_key_t privateKey;

                    if (rx < APDU_HEADER_LENGTH + BIP44_BYTE_LENGTH) {
                        hashTainted = 1;
                        THROW(0x6D09);
                    }

                    /** BIP44 path, used to derive the private key from the mnemonic by calling os_perso_derive_node_bip32. */
                    unsigned char * bip44_in = G_io_apdu_buffer + APDU_HEADER_LENGTH;

                    unsigned int bip44_path[BIP44_PATH_LEN];
                    uint32_t i;
                    for (i = 0; i < BIP44_PATH_LEN; i++) {
                        bip44_path[i] = (bip44_in[0] << 24) | (bip44_in[1] << 16) | (bip44_in[2] << 8) | (bip44_in[3]);
                        bip44_in += 4;
                    }
                    unsigned char privateKeyData[32];
                    os_perso_derive_node_bip32(CX_CURVE_256R1, bip44_path, BIP44_PATH_LEN, privateKeyData, NULL);
                    cx_ecdsa_init_private_key(CX_CURVE_256R1, privateKeyData, 32, &privateKey);

                    // generate the public key.
                    cx_ecdsa_init_public_key(CX_CURVE_256R1, NULL, 0, &publicKey);
                    cx_ecfp_generate_pair(CX_CURVE_256R1, &publicKey, &privateKey, 1);

                    // push the public key onto the response buffer.
                    os_memmove(G_io_apdu_buffer, publicKey.W, 65);
                    tx = 65;

                    // display_public_key(publicKey.W);
                    // refresh_public_key_display();

                    // return 0x9000 OK.
                    THROW(0x9000);
                }
                    break;
                
                case 0x01: // case 1
                    THROW(0x9000);
                    break;

                case 0x02: // echo
                    tx = rx;
                    THROW(0x9000);
                    break;

                case 0xFF: // return to dashboard
                    goto return_to_dashboard;

                default:
                    THROW(0x6D00);
                    break;
                }
            }
            CATCH_OTHER(e) {
                switch (e & 0xF000) {
                case 0x6000:
                case 0x9000:
                    sw = e;
                    break;
                default:
                    sw = 0x6800 | (e & 0x7FF);
                    break;
                }
                // Unexpected exception => report
                G_io_apdu_buffer[tx] = sw >> 8;
                G_io_apdu_buffer[tx + 1] = sw;
                tx += 2;
            }
            FINALLY {
            }
        }
        END_TRY;
    }

return_to_dashboard:
    return;
}

void io_seproxyhal_display(const bagl_element_t *element) {
    io_seproxyhal_display_default((bagl_element_t *)element);
}

unsigned char io_event(unsigned char channel) {
    // nothing done with the event, throw an error on the transport layer if
    // needed

    // can't have more than one tag in the reply, not supported yet.
    switch (G_io_seproxyhal_spi_buffer[0]) {
    case SEPROXYHAL_TAG_FINGER_EVENT:
        UX_FINGER_EVENT(G_io_seproxyhal_spi_buffer);
        break;

    case SEPROXYHAL_TAG_BUTTON_PUSH_EVENT: // for Nano S
        UX_BUTTON_PUSH_EVENT(G_io_seproxyhal_spi_buffer);
        break;

    case SEPROXYHAL_TAG_DISPLAY_PROCESSED_EVENT:
        if (UX_DISPLAYED()) {
            // TODO perform actions after all screen elements have been
            // displayed
        } else {
            UX_DISPLAYED_EVENT();
        }
        break;

    // unknown events are acknowledged
    default:
        break;
    }

    // close the event if not done previously (by a display or whatever)
    if (!io_seproxyhal_spi_is_status_sent()) {
        io_seproxyhal_general_status();
    }

    // command has been processed, DO NOT reset the current APDU transport
    return 1;
}

__attribute__((section(".boot"))) int main(void) {
    // exit critical section
    __asm volatile("cpsie i");

    hashTainted = 1;

    UX_INIT();

    // ensure exception will work as planned
    os_boot();

    BEGIN_TRY {
        TRY {
            io_seproxyhal_init();

// #ifdef LISTEN_BLE
//             if (os_seph_features() &
//                 SEPROXYHAL_TAG_SESSION_START_EVENT_FEATURE_BLE) {
//                 BLE_power(0, NULL);
//                 // restart IOs
//                 BLE_power(1, NULL);
//             }
// #endif

            USB_power(0);
            USB_power(1);

            ui_idle();

            sky_main();
        }
        CATCH_OTHER(e) {
        }
        FINALLY {
        }
    }
    END_TRY;
}
