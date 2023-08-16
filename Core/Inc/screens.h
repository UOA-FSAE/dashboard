
#ifndef INC_SCREENS_H_
#define INC_SCREENS_H_

enum SCREENS {
	DRIVER_SCREEN,
	DEBUG_SCREEN
};

// CALL THIS FIRST
void init_displays();

// CALL THIS SECOND
void init_screens();

void update_screen();

void change_screens(enum SCREENS screen);

void cycle_screens();

#endif /* INC_SCREENS_H_ */
