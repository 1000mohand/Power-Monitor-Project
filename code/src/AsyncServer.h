#include <Arduino.h>
#include "freertos/semphr.h"

inline volatile xSemaphoreHandle LED_mux         = xSemaphoreCreateMutex();
inline volatile bool             manually_changed  {false};

void serverInit();