#include <Arduino.h>
#include <map>
#include <cmath>
#include <freertos/task.h>

#include "reading.h"


void reading (void* param_ptr){
    Loop code_block()

    for (var read : pin_reading){
        read.second = analogRead(read.first);
    }
    vTaskDelay(500/portTICK_PERIOD_MS);

    end_block()
}

void pin_config(){
    for (Pin pin : output_pins){
    	pinMode(pin, OUTPUT);
	}


    for (Pin pin :  input_pins){
    	pinMode(pin, INPUT_PULLUP);
		pin_reading[pin] = 0;
	}

    xTaskCreate(reading,"readings",1280,NULL,2,NULL);
}