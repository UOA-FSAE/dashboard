#include <cstdlib>
#include <cstdint>
#include <cmath>
#include <stdio.h>
#include <cstring>
#include <string>
#include <vector>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/portmacro.h"
#include "freertos/message_buffer.h"

//our stuff
#include "dispController.h"
#include "disp.h"
#include "vehicle/vehicleData.h"
#include "can/canFrames.h"

//Fonts and bitmaps for screen
#include "../inc/fonts/gfxfont.h"
#include "inc/fonts/FreeMonoOblique12pt7b.h"
#include "inc/fonts/FreeSerif9pt7b.h"

#include "inc/fonts/FreeSans12pt7b.h"
#include "inc/fonts/FreeSansBold24pt7b.h"

//Values
#define DISPLAY_HEIGHT	320
#define DISPLAY_WIDTH	480
#define MARGIN 		    20

//Colors
colour565 textColour = 0xFFFFFF;

//TODO:  Move this inside the main loop and pass to other functions
//I think this is still fine though because the only thing writing to this will
//be on core 1

//This is our display!
disp *d = new disp(480, 320);

//This function updates and draws data to the screen if it's new/updated
template<class dataType>

//TODO:  Perhaps move this into disp.cpp, not sure why it was initially in main
void drawString(dataType &oldData, dataType &newData, uint16_t x, uint16_t y, TEXT_ALIGNMENT alignment, uint16_t colour, const char * format, const GFXfont *font) {

	//ONLY bother to do something if the data actually changes!
	if (newData != oldData) {

		char oldBuff[50];
		char newBuff[50];

		sprintf(oldBuff, format, oldData);
		sprintf(newBuff, format, newData);

		//ONLY bother to change the screen if the actual representation of the data has changed!
		if(strcmp(newBuff, oldBuff) != 0) {

			d->setTextAlignment(alignment);

			//Double Write to help ensure old text is cleared gud
			d->drawTextFast(x, y, oldBuff, font, (uint16_t)0x0000, 	(uint16_t)0x0000);
    		vTaskDelay(10/portTICK_RATE_MS);
			d->drawTextFast(x, y, oldBuff, font, (uint16_t)0x0000, 	(uint16_t)0x0000);

			d->drawTextFast(x, y, newBuff, font, colour, 			(uint16_t)0x0000);
	
		}

		//Update old data so that the display doesn't try update again
		oldData = newData;
	}
}

//Overloaded drawString function for non-data text drawing
void drawString(uint16_t x, uint16_t y, TEXT_ALIGNMENT alignment, uint16_t colour, const char * format, const GFXfont *font) {

	//Buffer for storing char array
	char buf[50];

	d->setTextAlignment(alignment);
	
	//Print the old data back but black to erase the existing data big brain
	sprintf(buf, format);

	//Double Write to help ensure old text is cleared gud
	d->drawTextFast(x, y, buf, font, (uint16_t)0x0000, (uint16_t)0x0000);
   	vTaskDelay(10/portTICK_RATE_MS);
	d->drawTextFast(x, y, buf, font, (uint16_t)0x0000, (uint16_t)0x0000);
	
	//Print the new data now
	sprintf(buf, format);
	d->drawTextFast(x, y, buf, font, colour, (uint16_t)0x0000);
}

[[noreturn]] void updateDisplay(void *pvParameters){
	// Message handle and data
    auto message_buffer = (MessageBufferHandle_t) pvParameters;
    struct Vehicle_Data data{};
    resetDataStructure(data);

	//A local version of data_displayed to check if anything has changed
	//Updated by the drawString function
	struct Vehicle_Data data_displayed{};
	resetDataStructure(data_displayed);

    //initialize display
    d->initDisplay();
    vTaskDelay(500/portTICK_RATE_MS);

	//TODO:  Implement pages as in original
	drawString((DISPLAY_WIDTH/4),	(MARGIN + 100), CENTER,  (uint16_t)0xFFFF, "Hello", &FreeSans12pt7b);

    while(true){
		if (xMessageBufferReceive(
                message_buffer,
				//Message buffer writes to local copy defined above
                &data,
                sizeof(struct Vehicle_Data),
                pdMS_TO_TICKS(1000)
        )) {
			//If recieved a message
			//This is a test to draw the string, we passing a pointer to the data, should currently write 0
			drawString(data_displayed.ts.soc,data.ts.soc,(DISPLAY_WIDTH - MARGIN),200,RIGHT,textColour.raw, "%u%%", &FreeSans12pt7b);
		}
    }
}