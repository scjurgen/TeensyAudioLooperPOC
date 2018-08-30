
#include "Arduino.h"

extern void getData(uint8_t channel, uint8_t* target, uint32_t size);
extern void playNew(uint8_t channel, const char* filename, uint32_t delayStart);
extern void update();
