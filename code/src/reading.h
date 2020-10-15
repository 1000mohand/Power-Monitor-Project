#include <Arduino.h>
#include <style.hpp>


#define LED 2
#define SENSOR 36


typedef byte Pin;
struct pin_reads{
	short read [5] {};
	unsigned accum_reads = 0;
};

inline std::map<Pin,short> pin_reading {};


constexpr inline Pin  input_pins[] = {36, 39, 34, 35, 32, 33
                                     };  // All pins
//constexpr inline Pin  input_pins[] = {36, 39, 34, 35
//                                     };
constexpr inline Pin output_pins[] = {23, 22,  1,  3, 21, 19, 18,  5,
                                      14, 16,  4,  2, 15};

void pin_config();

short get_device_voltage(Pin pin);

short get_device_voltage(class Device dev);