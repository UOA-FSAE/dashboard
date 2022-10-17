#ifndef __DISPCONTROLE_H
#define __DISPCONTROLE_H

//For drawing can data
void drawString(dataType &oldData, dataType &newData, uint16_t x, uint16_t y, TEXT_ALIGNMENT alignment, uint16_t colour, const char * format, const GFXfont *font);

//For drawing string labels
void drawString(uint16_t x, uint16_t y, TEXT_ALIGNMENT alignment, uint16_t colour, const char * format, const GFXfont *font);

[[noreturn]] void updateDisplay(void *pvParameters);

#endif