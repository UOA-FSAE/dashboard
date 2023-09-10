#ifndef DASHBOARD_MORSE_H
#define DASHBOARD_MORSE_H

typedef struct morse_handle {
    const char * string;
    int letter_counter;     // counts along the letters
    int morse_counter;      // counts along the dashes and dots
    int sound_counter;      // counts the dash/dot out
    int false_counter;
} morse_handle;

void init_morse_handle(morse_handle *handle,const char *string);

bool iterate_morse(morse_handle *handle);

#endif //DASHBOARD_MORSE_H
