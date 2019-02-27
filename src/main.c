#include "os.h"
#include "cx.h"

#include "os_io_seproxyhal.h"

#include "sky.h"
#include "ui.h"
#include "ux.h"

#include "apdu_handlers.h"

commandContext global;
// ux_state_t ux;

static void sky_main(void) {
    volatile unsigned int rx = 0;
    volatile unsigned int tx = 0;
    volatile unsigned int flags = 0;

    os_memmove(global.getPublicKeyContext.address, "  No address generated yet         \0", 36); // set default value for address 

    // DESIGN NOTE: the bootloader ignores the way APDU are fetched. The only
    // goal is to retrieve APDU.
    // When APDU are to be fetched from multiple IOs, like NFC+USB+BLE, make
    // sure the io_event is called with a
    // switch event, before the apdu is replied to the bootloader. This avoid
    // APDU injection faults.
    for (;;) {
        volatile unsigned short sw = 0;

        BEGIN_TRY{
                TRY{
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

                        // Find function that will handle specific request
                        handler_fn_t *handlerFn = lookupHandler(G_io_apdu_buffer[OFFSET_INS]);
                        if (!handlerFn) {
                            THROW(0x6D00);
                        }
                        handlerFn(G_io_apdu_buffer[OFFSET_P1], G_io_apdu_buffer[OFFSET_P2],
                        G_io_apdu_buffer + OFFSET_CDATA, G_io_apdu_buffer[OFFSET_LC], &flags, &tx);
                }
                CATCH_OTHER(e) {
                    switch (e & 0xF000) {
                        case 0x6000:
                        case INS_RET_SUCCESS:
                            sw = e;
                            break;
                        default:
                            sw = 0x6800 | (e & 0x7FF);
                            break;
                    }

                    // Unexpected exception => report
                    G_io_apdu_buffer[tx++] = sw >> 8;
                    G_io_apdu_buffer[tx++] = sw & 0xFF;
                }
                FINALLY {
                }
        }
        END_TRY;
    }
}

__attribute__((section(".boot"))) int main(void) {
    // exit critical section
    __asm volatile("cpsie i");

    // hashTainted = 1;
    // uiState = UI_IDLE;

    UX_INIT();

    // ensure exception will work as planned
    os_boot();

    BEGIN_TRY{
            TRY{
                    io_seproxyhal_init();

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
