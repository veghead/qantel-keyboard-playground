
#include "input-event-codes.h"
#include <ftdi.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define STROBE 2
#define CLOCK 7

int keymap[] = {0,
KEY_NUMERIC_7, KEY_NUMERIC_4, KEY_INSERT, 0, KEY_CLEAR, 0, /*KEY_TRANSMIT*/ 0, KEY_APOSTROPHE, KEY_SPACE, KEY_LEFTBRACE, KEY_ENTER, KEY_BACKSLASH, KEY_GRAVE, KEY_BACKSPACE, KEY_NUMERIC_1, KEY_A, KEY_C, KEY_S, KEY_E, KEY_W, KEY_3, KEY_4, KEY_X, KEY_L, 0, KEY_SEMICOLON, KEY_LEFTBRACE, KEY_P, KEY_MINUS, KEY_EQUAL, KEY_SLASH, KEY_D, KEY_B, KEY_F, KEY_T, KEY_R, KEY_5, KEY_6, KEY_V, KEY_J, KEY_DOT, KEY_K, KEY_O, KEY_I, KEY_9, KEY_0, KEY_COMMA, KEY_G, KEY_M, KEY_H, KEY_U, KEY_Y, KEY_7, KEY_8, KEY_N, KEY_F2, KEY_Z, /*KEY_CTRL*/ 0, KEY_Q, KEY_TAB, KEY_1, KEY_2, KEY_F3, KEY_DELETE, KEY_NUMERIC_0, KEY_NUMERIC_6, KEY_NUMERIC_5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, KEY_NUMERIC_9, KEY_NUMERIC_8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, KEY_TAB, KEY_MINUS, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, KEY_NUMERIC_3, KEY_DOT, KEY_NUMERIC_2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, KEY_NUMERIC_7, KEY_NUMERIC_4, 0, 0, KEY_CLEAR, 0, 0, 0, KEY_SPACE, 0, 0, KEY_BACKSLASH, KEY_GRAVE, KEY_BACKSPACE, KEY_NUMERIC_1, KEY_A, KEY_C, KEY_S, KEY_E, KEY_W, KEY_3, KEY_4, KEY_X, KEY_L, 0, KEY_SEMICOLON, 0, KEY_P, KEY_MINUS, 0, KEY_SLASH, KEY_D, KEY_B, KEY_F, KEY_T, KEY_R, KEY_5, KEY_6, KEY_V, KEY_J, KEY_DOT, KEY_K, KEY_O, KEY_I, KEY_9, KEY_0, KEY_COMMA, KEY_G, KEY_M, KEY_H, KEY_U, KEY_Y, KEY_7, KEY_8, KEY_N, KEY_F2, KEY_Z, 0, KEY_Q, KEY_TAB, KEY_1, KEY_2, KEY_F3, KEY_DELETE, KEY_NUMERIC_0, KEY_NUMERIC_6, KEY_NUMERIC_5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, KEY_NUMERIC_9, KEY_NUMERIC_8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, KEY_TAB, KEY_MINUS, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, KEY_NUMERIC_3, KEY_DOT, KEY_NUMERIC_2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

int clock_keypress(struct ftdi_context *ftdi)
{
    unsigned char buf[1];
    unsigned char k = 0;
    for (int i = 0; i < 8; i++)
    {
        buf[0] = (1 << CLOCK);
        ftdi_write_data(ftdi, buf, 1);
        usleep(5000);
        ftdi_read_pins(ftdi, buf);
        k = k | ((buf[0] & 1) << i);
        buf[0] = 0;
        ftdi_write_data(ftdi, buf, 1);
        usleep(5000);
    }
    printf("0x%02x, ", k);
}

int main(int argc, char **argv)
{
    struct ftdi_context *ftdi;
    int f, i;
    unsigned char buf[1];
    int retval = 0;

    if ((ftdi = ftdi_new()) == 0)
    {
        fprintf(stderr, "ftdi_new failed\n");
        return EXIT_FAILURE;
    }

    f = ftdi_usb_open(ftdi, 0x0403, 0x6014);

    if (f < 0 && f != -5)
    {
        fprintf(stderr, "unable to open ftdi device: %d (%s)\n", f, ftdi_get_error_string(ftdi));
        retval = 1;
        goto done;
    }

    printf("ftdi open succeeded: %d\n", f);

    printf("enabling bitbang mode\n");
    // 0xFF all output
    // 0x00 all input
    ftdi_set_bitmode(ftdi, 0x80, BITMODE_BITBANG);

    bool strobe = false;
    while (true)
    {
        f = ftdi_read_pins(ftdi, buf);
        if (buf[0] & (1 << STROBE))
        {
            if (strobe)
            {
                strobe = false;
            }
        }
        else
        {
            if (!strobe)
            {
                clock_keypress(ftdi);
                strobe = true;
            }
        }
        fflush(stdout);
    }

    printf("\n");

    printf("disabling bitbang mode\n");
    sleep(5);
    ftdi_disable_bitbang(ftdi);

    ftdi_usb_close(ftdi);
done:
    ftdi_free(ftdi);

    return retval;
}
