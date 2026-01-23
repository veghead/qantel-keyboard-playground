
#ifndef KEYCODE_H
#define KEYCODE_H

typedef struct mapping {
    int keycode;
    char *name;
} mapping_t;

extern mapping_t keymap[];

#endif // KEYCODE_H
