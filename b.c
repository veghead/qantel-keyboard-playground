
#include "input-event-codes.h"
#include <ftdi.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "keymap.h"

#define STROBE 2
#define CLOCK 7

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
    printf("0x%02x %s, ", keymap[k].keycode, keymap[k].name);
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
