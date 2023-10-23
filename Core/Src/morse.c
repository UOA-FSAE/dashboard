#include <stdint.h>
#include <stdbool.h>
#include "morse.h"

// 1 is dot, 2 is space, 0 is end of letter
// if negative add 17+26
int8_t morse_lookup[37][6] = {
        {1, 2, 0, 0, 0, 0}, //A
        {2, 1, 1, 1, 0, 0}, //B
        {2, 1, 2, 1, 0, 0}, //C
        {2, 1, 1, 0, 0, 0},      //D
        {1, 0, 0, 0, 0, 0},      //E
        {1, 1, 2, 1, 0, 0},      //F
        {2, 2, 1, 0, 0, 0},      //G
        {1, 1, 1, 1, 0, 0},      //H
        {1, 1, 0, 0, 0, 0},      //I
        {1, 2, 2, 2, 0, 0},      //J
        {2, 1, 2, 0, 0, 0},     //K
        {1, 2, 1, 1, 0, 0},     //L
        {2, 2, 0, 0, 0, 0},     //M
        {2, 1, 0, 0, 0, 0},     //N
        {2, 2, 2, 0, 0, 0},     //O
        {1, 2, 2, 1, 0, 0},     //P
        {1, 2, 2, 1, 0, 0},     //Q
        {2, 2, 1, 2, 0, 0},     //R
        {1, 2, 1, 0, 0, 0},     //S
        {1, 1, 1, 0, 0, 0},     //T
        {2, 0, 0, 0, 0, 0},     //U
        {1, 1, 1, 2, 0, 0},     //V
        {1, 2, 2, 0, 0, 0},     //W
        {2, 1, 1, 2, 0, 0},     //X
        {2, 1, 2, 2, 0, 0},     //Y
        {2, 2, 1, 1, 0, 0},     //Z
        {2, 2, 2, 2, 2, 0},     //0
        {1, 2, 2, 2, 2, 0},     //1
        {1, 1, 2, 2, 2, 0},     //2
        {1, 1, 1, 2, 2, 0},     //3
        {1, 1, 1, 1, 2, 0},     //4
        {1, 1, 1, 1, 1, 0},     //5
        {2, 1, 1, 1, 1, 0},     //6
        {2, 2, 1, 1, 1, 0},     //7
        {2, 2, 2, 1, 1, 0},     //8
        {2, 2, 2, 2, 1, 0},     //9
        {-1, 0, 0, 0, 0, 0},     // space
};

int8_t get_letter_id(char letter) {
    int8_t letter_id = letter - 'A';
    if (letter_id == -33) return 36;
    if (letter_id < 0) letter_id += (17+26);
    return letter_id;
}

void init_morse_handle(morse_handle *handle,const char *string) {
    handle->string = string;
    handle->false_counter = 0;
    handle->letter_counter = 0;
    handle->morse_counter = 0;
    handle->sound_counter = morse_lookup[get_letter_id(handle->string[0])][0];
}

bool iterate_morse(morse_handle *handle) {
	if (handle->sound_counter < 0) {
		handle->sound_counter++;
		return false;
	}
    if (handle->sound_counter > 0) {
        handle->sound_counter--;
        return true;
    } else {
        if (!morse_lookup[get_letter_id(handle->string[handle->letter_counter])][handle->morse_counter+1]) {   // If end of letter
            if (!handle->string[handle->letter_counter+1]) {  // If end of string restart letter counter
                handle->letter_counter = 0;
                handle->morse_counter = 0;
                handle->sound_counter = morse_lookup[get_letter_id(handle->string[0])][0];
                handle->false_counter++;
                return false;
            } else {
                handle->letter_counter++;
                handle->morse_counter = 0;
            }
        } else {        // If still inside letter
            handle->morse_counter++;
        }
        // set sound counter here
        handle->sound_counter = morse_lookup[get_letter_id(handle->string[handle->letter_counter])][handle->morse_counter];
        return false;
    }
}
