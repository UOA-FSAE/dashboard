#ifndef __DISPCONTROLE_H
#define __DISPCONTROLE_H

typedef enum {
	blank,
	race,
	driver,
	temperatures,
	electrical,
	competition
} page;

[[noreturn]] void updateDisplay(void *pvParameters);

#endif