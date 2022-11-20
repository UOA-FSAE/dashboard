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
// We are working in 3 different colour spaces,
// Each LED takes a value for brightness, and a set of RGB values.
// The display uses RGB565 natively.
// Neither of these is particularly nice to work with directly, so we include 888 HSV and 888 RGB with functions to convert between colour spaces.

colour565 textColour = 0xFFFFFF;
// Inline construction of 565 from 888
colour565 RED565 = 0xFF0000;
colour565 BLUE565 = 0x0000FF;
colour565 GREEN565 = 0x00FF00;
colour565 ORANGE565 = 0xF68F21;

//colourMap 
struct colourMap{
	float low; 		// Minimum threshold
	float high; 	// Maximum threshold
	uint32_t colour;// Colour between these thresholds
};

//LUT's for mapping data values to colours
const std::vector<colourMap> minCellVoltage_LUT {
	{ 0.0,	3.1,	0xFF0000 },	//TOO LOW
	{ 3.1,	3.5,	0xF68F21 },	//KINDA LOW
	{ 3.5,	4.2,	0x00FF00 },	//IDEAL
	{ 4.2,	5.0,	0xFF0000 }	//TOO HIGH
};

const std::vector<colourMap> GLVVoltage_LUT {
	{ 0.0,	23,		0xFF0000 },	//TOO LOW
	{ 23,	25,		0xFFFF00 },	//KINDA LOW
	{ 25,	29,		0X00FF00 },	//IDEAL
	{ 29,	100,	0xFF0000 }	//TOO HIGH
};

const std::vector<colourMap> motorTemp_LUT {
	{ 0,	15,		0xFFFFFF },	//White
	{ 15,	60,		0x00FF00 },	//Green
	{ 60,	70,		0xFFFF00 },	//Yellow
	{ 70,	999,	0xFF0000 }	//Red?
};

const std::vector<colourMap> inverterTemp_LUT {
	{ 0,	15,		0xFFFFFF },	//White
	{ 15,	45,		0x00FF00 },	//Green
	{ 45,	50,		0xFFFF00 },	//Yellow
	{ 50,	999,	0xFF0000 }	//Red?
};

const std::vector<colourMap> accumulatorTemp_LUT {
	{ 0,	15,		0xFFFFFF },	//White
	{ 15,	45,		0x00FF00 },	//Green
	{ 45,	55,		0xFFFF00 },	//Yellow
	{ 55,	999,	0xFF0000 }	//Red?
};

const std::vector<colourMap> SOC_LUT {
	{ 0,	10,		0xFF0000 },	//THIS IS NOT FINE
	{ 10,	25,		0xFFFF00 },	//This is fine...
	{ 25,	100,	0x00FF00 }	//This is fine
};

const std::vector<colourMap> lapDelta_LUT {
	{ 0,		10,		0x00FF00 },
	{ 10,		10000,	0xFFFF00 },
	{ 10000,	99999,	0xFF0000 }
};

//This function updates and draws data to the screen if it's new/updated
template<class dataType>
void drawString(disp *d,dataType &oldData, dataType &newData, uint16_t x, uint16_t y, TEXT_ALIGNMENT alignment, uint16_t colour, const char * format, const GFXfont *font) {

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
void drawString(disp *d,uint16_t x, uint16_t y, TEXT_ALIGNMENT alignment, uint16_t colour, const char * format, const GFXfont *font) {

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

//This function returns the colour from a LUT
template<class dataType>
uint32_t getColour(const std::vector<colourMap> &colourLUT, const dataType data) {

	//Iterate through the LUT to find the right colourMap to use
	for(const auto& colourMap : colourLUT){
		if(data >= colourMap.low && data < colourMap.high) {
			return colourMap.colour;
		}
	}

	//Default Text Colour
	return 0xFFFFFF;
}

void msToTimeString(char buffer[50], uint32_t ms) {

	uint8_t	milliseconds = ms % 1000;
	uint8_t	seconds = (ms / 1000) % 60;  
	uint8_t	minutes = (ms / 1000) / 60;

	sprintf(buffer, "%u:%u:%u", minutes, seconds, milliseconds);

	//If it's waaay too long then just fuckin print some dashes
	if(ms > 1000000) {
		sprintf(buffer, "--:--:---");
	}
}

[[noreturn]] void updateDisplay(void *pvParameters){
	//This is our display! CHRIS PLEASE CHECK IF I'M PASSING THESE RIGHT lol
	disp *d = new disp(480, 320);

	//This is the page handling system
	page lastPage = blank;
	page currentPage = competition;

	//Buffer for storing data and strings to write to screen
	char buffer[50];	//Longest data expected to be "00.00 %", Longest strings from error messages or times

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

    while(true){
		if (xMessageBufferReceive(
                message_buffer,
				//Message buffer writes to local copy defined above
                &data,
                sizeof(struct Vehicle_Data),
                pdMS_TO_TICKS(1000)
        )) {
			//This is imported directly from the old code, I think its fine
			if(currentPage != lastPage) {
				//Clear the screen completely if the page has changed
				d->fillScreen(0x0000);

				//Reset Data Displayed so that the new page is completely updated
				resetDataStructure(data_displayed);

				//Update the screen with the required 'static' text. (Labels which do not change)
				switch(currentPage) {

					case competition:
						//Hard Coded location for labels
						drawString(d,(DISPLAY_WIDTH/4),	(MARGIN + 100), CENTER,  (uint16_t)0xFFFF, "Rear Left", &FreeSans12pt7b);
						drawString(d,(DISPLAY_WIDTH/4),	MARGIN,  		CENTER,  (uint16_t)0xFFFF, "Front Left", &FreeSans12pt7b);
						drawString(d,(DISPLAY_WIDTH/4)*3,	MARGIN,  		CENTER,  (uint16_t)0xFFFF, "Front Right", &FreeSans12pt7b);
						drawString(d,(DISPLAY_WIDTH/4)*3,	(MARGIN + 100), CENTER,  (uint16_t)0xFFFF, "Rear Right", &FreeSans12pt7b);

						//X, Y, Alignment, Colour, Format Specifier, Font
						drawString(d,(DISPLAY_WIDTH/4)*3,	200,  RIGHT,  (uint16_t)0xFFFF, "TS SoC: ",					&FreeSans12pt7b);
						drawString(d,(DISPLAY_WIDTH/4)*3,	225,  RIGHT,  (uint16_t)0xFFFF, "TS Low Cell Voltage: ",	&FreeSans12pt7b);
						drawString(d,(DISPLAY_WIDTH/4)*3,	250,  RIGHT,  (uint16_t)0xFFFF, "TS High Cell Temp: ", 		&FreeSans12pt7b);
						drawString(d,(DISPLAY_WIDTH/4)*3,	275,  RIGHT,  (uint16_t)0xFFFF, "GLV Voltage: ",			&FreeSans12pt7b);

						break;

					case blank:
						//Do nothing
						break;

					case race:

						//X, Y, Alignment, Colour, Format Specifier, Font
						drawString(d,MARGIN, 					 50,  LEFT,  (uint16_t)0xFFFF, "Best: ", 	&FreeSans12pt7b);
						drawString(d,MARGIN, 					 100, LEFT,  (uint16_t)0xFFFF, "Delta: ", 	&FreeSansBold24pt7b);
						drawString(d,MARGIN, 					 150, LEFT,  (uint16_t)0xFFFF, "Curr: ", 	&FreeSans12pt7b);
						drawString(d,(DISPLAY_WIDTH - MARGIN), 100, RIGHT, (uint16_t)0xFFFF, "kph",		&FreeSans12pt7b);

						break;

					case driver:
						d->setTextAlignment(CENTER);

						d->drawTextFast((DISPLAY_WIDTH / 2), 50, "Steering Angle:", &FreeSans12pt7b, (uint16_t)0xFFFF, (uint16_t)0x0000);
						//TODO:  Implement this
						//drawSteeringAngle();
						d->setTextAlignment(LEFT);

						d->drawTextFast(MARGIN, 150, "Front Bias:", &FreeSans12pt7b, (uint16_t)0xFFFF, (uint16_t)0x0000);
						d->drawTextFast(MARGIN, 175, "Front Brake:", &FreeSans12pt7b, (uint16_t)0xFFFF, (uint16_t)0x0000);
						d->drawTextFast(MARGIN, 200, "Rear Brake:", &FreeSans12pt7b, (uint16_t)0xFFFF, (uint16_t)0x0000);

						d->drawTextFast((DISPLAY_WIDTH / 2), 150, "Throttle:", &FreeSans12pt7b, (uint16_t)0xFFFF, (uint16_t)0x0000);
						d->drawTextFast((DISPLAY_WIDTH / 2), 175, "Torque:", &FreeSans12pt7b, (uint16_t)0xFFFF, (uint16_t)0x0000);
						d->drawTextFast((DISPLAY_WIDTH / 2), 200, "RPM:", &FreeSans12pt7b, (uint16_t)0xFFFF, (uint16_t)0x0000);
		
						break;

					case temperatures:

						d->setTextAlignment(LEFT);

						for(uint8_t i = 0; i < 4; ++i) {

							uint16_t xPos;
							uint16_t yPos;

							//Horizontal alignment
							if(i <= 1) {
								xPos = MARGIN;
							} else {
								xPos = (DISPLAY_WIDTH / 2 + MARGIN);
							}

							//Vertical alignment
							if(i == 1 || i == 2) {
								yPos = 50;
							} else {
								yPos = 150;
							}

							d->drawTextFast(xPos, yPos, "Gearbox:", &FreeSans12pt7b, (uint16_t)0xFFFF, (uint16_t)0x0000);
							d->drawTextFast(xPos, yPos + 25, "Motor:", &FreeSans12pt7b, (uint16_t)0xFFFF, (uint16_t)0x0000);
							d->drawTextFast(xPos, yPos + 50, "Inverter:", &FreeSans12pt7b, (uint16_t)0xFFFF, (uint16_t)0x0000);
						}

						break;

					case electrical:

						d->setTextAlignment(LEFT);

						d->drawTextFast(MARGIN, 50, "SoC:", &FreeSans12pt7b, (uint16_t)0xFFFF, (uint16_t)0x0000);
						d->drawTextFast(MARGIN, 75, "Lowest Cell:", &FreeSans12pt7b, (uint16_t)0xFFFF, (uint16_t)0x0000);
						d->drawTextFast(MARGIN, 100, "Current:", &FreeSans12pt7b, (uint16_t)0xFFFF, (uint16_t)0x0000);
						d->drawTextFast(MARGIN, 125, "Hottest Cell:", &FreeSans12pt7b, (uint16_t)0xFFFF, (uint16_t)0x0000);
					
						d->drawTextFast((DISPLAY_WIDTH / 2 + MARGIN), 50, "SoC:", &FreeSans12pt7b, (uint16_t)0xFFFF, (uint16_t)0x0000);
						d->drawTextFast((DISPLAY_WIDTH / 2 + MARGIN), 75, "Voltage:", &FreeSans12pt7b, (uint16_t)0xFFFF, (uint16_t)0x0000);
						d->drawTextFast((DISPLAY_WIDTH / 2 + MARGIN), 100, "Current:", &FreeSans12pt7b, (uint16_t)0xFFFF, (uint16_t)0x0000);

						break;
				}

				lastPage = currentPage;	
			}

			//Depending on which page we want to display, display the right information :P
			switch(currentPage) {

				case competition:
					//drivetrain temps
					for(uint8_t i = 0; i < 4; ++i) {

						uint16_t xPos;
						uint16_t yPos;

						//Horizontal alignment
						if(i <= 1) {
							xPos = MARGIN;
						} else {
							xPos = (DISPLAY_WIDTH / 2 + MARGIN);
						}

						//Vertical alignment
						if(i == 1 || i == 2) {
							yPos = MARGIN + 25;
						} else {
							yPos = MARGIN + 125;
						}

						//Old, New, X, Y, Alignment, Colour, Format Specifier, Font
						textColour = getColour(motorTemp_LUT, data.drive[i].motorTemp);
						drawString(d,data_displayed.drive[i].motorTemp, data.drive[i].motorTemp, 		 xPos, yPos,	  LEFT, textColour.raw, "Motor: %u C",	  &FreeSans12pt7b);

						textColour = getColour(inverterTemp_LUT, data.drive[i].inverterTemp);
						drawString(d,data_displayed.drive[i].inverterTemp, data.drive[i].inverterTemp, xPos, yPos + 25, LEFT, textColour.raw, "Inverter: %u C", &FreeSans12pt7b);
					}

					//TS and GLV 
					//Old, New, X, Y, Alignment, Colour, Format Specifier, Font
					textColour = getColour(SOC_LUT, data.ts.soc);
					drawString(d,data_displayed.ts.soc, 			data.ts.soc, 	 	(DISPLAY_WIDTH - MARGIN), 	200, 	RIGHT, textColour.raw, "%u%%", 		&FreeSans12pt7b);

					textColour = getColour(minCellVoltage_LUT, data.ts.minVoltage);
					drawString(d,data_displayed.ts.minVoltage, 	data.ts.minVoltage, (DISPLAY_WIDTH - MARGIN),	225,	RIGHT, textColour.raw, "%#2.2f V%", &FreeSans12pt7b);

					textColour = getColour(accumulatorTemp_LUT, data.ts.maxTemp);
					drawString(d,data_displayed.ts.maxTemp,  		data.ts.maxTemp,  	(DISPLAY_WIDTH - MARGIN),	250,	RIGHT, textColour.raw, "%u C", 		&FreeSans12pt7b);
					
					//GLV
					textColour = getColour(GLVVoltage_LUT, data.glv.voltage);
					drawString(d,data_displayed.glv.voltage, 		data.glv.voltage, 	(DISPLAY_WIDTH - MARGIN), 	275,	RIGHT, textColour.raw, 	"%#2.2f V", &FreeSans12pt7b);

					break;

				case blank:

					//Clear the screen completely
					d->fillScreen(0x0000);

					break;
			
				case race:

					d->setTextAlignment(LEFT);
				
					msToTimeString(buffer,data.race.bestLapTime);
					d->drawTextFast(MARGIN + d->getStringLength(1, "Best: ", &FreeSans12pt7b), 50, buffer, &FreeSans12pt7b, (uint16_t)GREEN565.raw, (uint16_t)0x0000);
					
					msToTimeString(buffer,data.race.deltaLapTime);

					textColour = getColour(lapDelta_LUT, data.race.deltaLapTime);
					d->drawTextFast(MARGIN + d->getStringLength(1, "Delta: ", &FreeSansBold24pt7b), 100, buffer, &FreeSansBold24pt7b, (uint16_t)textColour.raw, (uint16_t)0x0000);

					msToTimeString(buffer,data.race.currentLapTime);
					d->drawTextFast(MARGIN + d->getStringLength(1, "Curr: ", &FreeSans12pt7b), 150, buffer, &FreeSans12pt7b, (uint16_t)0xFFFF, (uint16_t)0x0000);
					
					//Old, New, X, Y, Alignment, Colour, Format Specifier, Font
					drawString(d,data_displayed.race.lapNumber, data.race.lapNumber, (DISPLAY_WIDTH - MARGIN), 50, RIGHT, (uint16_t)0xFFFF, "Lap# %u", &FreeSans12pt7b);
					drawString(d,data_displayed.race.currentSpeed, data.race.currentSpeed, (DISPLAY_WIDTH - MARGIN) - d->getStringLength(1, "kph", &FreeSans12pt7b), 100, RIGHT, (uint16_t)0xFFFF, "%u", &FreeSansBold24pt7b);
					
					textColour = getColour(SOC_LUT, data.ts.soc);
					drawString(d,data_displayed.ts.soc, data.ts.soc, (DISPLAY_WIDTH - MARGIN), 150, RIGHT, textColour.raw, "%u%%", &FreeSans12pt7b);

					break;
			
				case driver:

					//Old, New, X, Y, Alignment, Colour, Format Specifier, Font
					drawString(d,data_displayed.driver.steeringAngle, 		data.driver.steeringAngle, 		(DISPLAY_WIDTH / 2), 			75, CENTER, (uint16_t)0xFFFF, "%i deg", &FreeSans12pt7b);

					//Old, New, X, Y, Alignment, Colour, Format Specifier, Font
					drawString(d,data_displayed.driver.brakeBias, 			data.driver.brakeBias, 			(DISPLAY_WIDTH / 2 - MARGIN), 	150, RIGHT, (uint16_t)0xFFFF, "%u%%", &FreeSans12pt7b);
					drawString(d,data_displayed.driver.frontBrakePressure, 	data.driver.frontBrakePressure, (DISPLAY_WIDTH / 2 - MARGIN), 	175, RIGHT, (uint16_t)0xFFFF, "%u kPa", &FreeSans12pt7b);
					drawString(d,data_displayed.driver.rearBrakePressure, 	data.driver.rearBrakePressure, 	(DISPLAY_WIDTH / 2 - MARGIN), 	200, RIGHT, (uint16_t)0xFFFF, "%u kPa", &FreeSans12pt7b);

					//Old, New, X, Y, Alignment, Colour, Format Specifier, Font
					drawString(d,data_displayed.driver.throttle, 				data.driver.throttle,			(DISPLAY_WIDTH - MARGIN), 		150, RIGHT, (uint16_t)0xFFFF, "%u%%", &FreeSans12pt7b);
					drawString(d,data_displayed.driver.torque, 				data.driver.torque, 			(DISPLAY_WIDTH - MARGIN), 		170, RIGHT, (uint16_t)0xFFFF, "%u%%", &FreeSans12pt7b);
					drawString(d,data_displayed.driver.rpm, 					data.driver.rpm, 				(DISPLAY_WIDTH - MARGIN), 		200, RIGHT, (uint16_t)0xFFFF, "%u", &FreeSans12pt7b);
		
					break;
			
				case temperatures:

					for(uint8_t i = 0; i < 4; ++i) {

						uint16_t xPos;
						uint16_t yPos;

						//Horizontal alignment
						if(i <= 1) {
							xPos = (DISPLAY_WIDTH / 2 - MARGIN);
						} else {
							xPos = (DISPLAY_WIDTH - MARGIN);
						}

						//Vertical alignment
						if(i == 1 || i == 2) {
							yPos = 50;
						} else {
							yPos = 150;
						}

						//Old, New, X, Y, Alignment, Colour, Format Specifier, Font
						drawString(d,data_displayed.drive[i].gearboxTemp, data.drive[i].gearboxTemp, xPos, yPos, RIGHT, textColour.raw, "%u C", &FreeSans12pt7b);

						textColour = getColour(motorTemp_LUT, data.drive[i].motorTemp);
						drawString(d,data_displayed.drive[i].motorTemp, data.drive[i].motorTemp, xPos, yPos + 25, RIGHT, textColour.raw, "%u C", &FreeSans12pt7b);

						textColour = getColour(inverterTemp_LUT, data.drive[i].inverterTemp);
						drawString(d,data_displayed.drive[i].inverterTemp, data.drive[i].inverterTemp, xPos, yPos + 50, RIGHT, textColour.raw, "%u C", &FreeSans12pt7b);
					}
					break;
			
				case electrical:

					//TS
					textColour = getColour(SOC_LUT, data.ts.soc);
					//Old, New, X, Y, Alignment, Colour, Format Specifier, Font
					drawString(d,data_displayed.ts.soc, 			data.ts.soc, 	 	DISPLAY_WIDTH / 2, 	   		50, 	RIGHT, textColour.raw, "%u%%", 		&FreeSans12pt7b);

					textColour = getColour(minCellVoltage_LUT, data.ts.minVoltage);
					drawString(d,data_displayed.ts.minVoltage, 	data.ts.minVoltage, DISPLAY_WIDTH / 2,			75,		RIGHT, textColour.raw, "%#2.2f V%", &FreeSans12pt7b);
					drawString(d,data_displayed.ts.current,  		data.ts.current,  	DISPLAY_WIDTH / 2, 	   		100,	RIGHT, 0xFFFF, 		   "%#2.2f A", 	&FreeSans12pt7b);

					textColour = getColour(accumulatorTemp_LUT, data.ts.maxTemp);
					drawString(d,data_displayed.ts.maxTemp,  		data.ts.maxTemp,  	(DISPLAY_WIDTH / 2), 	   	125,	RIGHT, textColour.raw, "%u C", 		&FreeSans12pt7b);
					
					//GLV
					textColour = getColour(SOC_LUT, data.glv.soc);
					drawString(d,data_displayed.glv.soc, 	   		data.glv.soc, 	 	(DISPLAY_WIDTH - MARGIN), 	50,		RIGHT, textColour.raw,	"%u%%", 	&FreeSans12pt7b);

					textColour = getColour(GLVVoltage_LUT, data.glv.voltage);
					drawString(d,data_displayed.glv.voltage, 		data.glv.voltage, 	(DISPLAY_WIDTH - MARGIN), 	75,		RIGHT, textColour.raw, 	"%#2.2f V", &FreeSans12pt7b);
					drawString(d,data_displayed.glv.current, 		data.glv.current, 	(DISPLAY_WIDTH - MARGIN), 	100,	RIGHT, 0xFFFF,			"%#2.2f A", &FreeSans12pt7b);

					break;
			}
		}
    }
}