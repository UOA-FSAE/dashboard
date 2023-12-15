
#ifndef INC_SCREENS_H_
#define INC_SCREENS_H_

#include <stdint.h>

enum SCREENS {
	DRIVER_SCREEN,
	DEBUG_SCREEN,
    ENERGY_SCREEN,
    LAP_SCREEN,     // <- Insert after here (error should be last)
    TEMPERATURE_SCREEN,
    ERROR_SCREEN
};

// CALL THIS FIRST
void init_displays(void);

// CALL THIS SECOND
void init_screens(void);

void try_update_screen(void);

void update_screen(void);

void try_cycle_screens(void);

void cycle_screens(void);

void queue_change_screens(enum SCREENS screen);

void try_change_screens(void);

void change_screens(enum SCREENS screen);

void ms_to_minutes_seconds(uint32_t ms, uint32_t * minutes, uint32_t * seconds, uint32_t * milliseconds);

void enable_popups(void);

void disable_popups(void);

void try_enable_popups(void);

void try_disable_popups(void);


#endif /* INC_SCREENS_H_ */
