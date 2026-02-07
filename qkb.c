#include "input-event-codes.h"
#include "keymap.h"
#include <ftdi.h>
#include <libevdev/libevdev-uinput.h>
#include <libevdev/libevdev.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#define STROBE 2
#define CLOCK 7

volatile sig_atomic_t quit = 0;

int clock_keypress(struct ftdi_context *ftdi)
{
    unsigned char buf[1];
    unsigned char k = 0;
    for (int i = 0; i < 8; i++) {
        buf[0] = (1 << CLOCK);
        ftdi_write_data(ftdi, buf, 1);
        usleep(1000);
        ftdi_read_pins(ftdi, buf);
        k = k | ((buf[0] & 1) << i);
        buf[0] = 0;
        ftdi_write_data(ftdi, buf, 1);
        usleep(1000);
    }
    printf("0x%02x %s, ", keymap[k].keycode, keymap[k].name);
    return keymap[k].keycode;
}

int open_uinput_device(struct libevdev_uinput **uidev)
{
    int err;
    struct libevdev *dev;

    // Create a new libevdev device
    // and enable it to send keystrokes
    dev = libevdev_new();
    libevdev_set_name(dev, "Qantel Keyboard");
    libevdev_enable_event_type(dev, EV_KEY);
    libevdev_enable_event_type(dev, EV_SYN);

    // Also enable all the keys in the keymap
    for (int k = 0; k < 256; k++) {
        if (keymap[k].keycode) {
            int ret = libevdev_enable_event_code(dev, EV_KEY, keymap[k].keycode, NULL);
            if (ret != 0) {
                fprintf(stderr, "Error registering event type: %s\n", keymap[k].name);
                return 2;
            }
        }
    }

    // Now we create a uinput device, based on our libevdev device. We don't need to open
    // /dev/uinput because we let libevdev manage that for us.
    err = libevdev_uinput_create_from_device(dev, LIBEVDEV_UINPUT_OPEN_MANAGED, uidev);
    return err;
}

void handle_signal(int signum) {
    if (signum == SIGINT) {
        quit = 1;
    }
}

int main(int argc, char **argv)
{
    struct ftdi_context *ftdi;
    int f;
    unsigned char buf[1];
    int retval = 0;
    struct libevdev_uinput *uidev;
    struct sigaction sa;
    sa.sa_handler = handle_signal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;


    if ((ftdi = ftdi_new()) == 0) {
        fprintf(stderr, "ftdi_new failed\n");
        return EXIT_FAILURE;
    }

    f = ftdi_usb_open(ftdi, 0x0403, 0x6014);

    if (f < 0 && f != -5) {
        fprintf(stderr, "unable to open ftdi device: %d (%s)\n", f, ftdi_get_error_string(ftdi));
        retval = 1;
        goto done;
    }

    int ret = open_uinput_device(&uidev);
    if (ret != 0) { goto done; }

    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("Error trapping SIGINT");
        goto cleanup;
    }

    printf("FTDI open succeeded: %d\n", f);
    printf("Enabling bitbang mode\n");
    // 0xFF all output
    // 0x00 all input
    ftdi_set_bitmode(ftdi, 0x80, BITMODE_BITBANG);

    bool strobe = false;

    while (!quit) {
        f = ftdi_read_pins(ftdi, buf);
        if (buf[0] & (1 << STROBE)) {
            if (strobe) { strobe = false; }
        } else {
            if (!strobe) {
                int key = clock_keypress(ftdi);
                libevdev_uinput_write_event(uidev, EV_KEY, key, 1);
                libevdev_uinput_write_event(uidev, EV_SYN, SYN_REPORT, 0);
                usleep(1000);
                libevdev_uinput_write_event(uidev, EV_KEY, key, 0);
                libevdev_uinput_write_event(uidev, EV_SYN, SYN_REPORT, 0);
                strobe = true;
            }
        }
        fflush(stdout);
    }

    printf("Exiting \n");

    ftdi_disable_bitbang(ftdi);
    ftdi_usb_close(ftdi);

cleanup:
    libevdev_uinput_destroy(uidev);
done:
    ftdi_free(ftdi);

    return retval;
}
