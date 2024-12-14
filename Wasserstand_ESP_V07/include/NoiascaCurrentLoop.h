/* Noiasca Current Loop Library, 
 * for measuring current loop sensors 4mA - 20mA
 * https://werner.rothschopf.net
 * Copyright (c) Werner Rothschopf
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * 2020-05-08 Version 1.0.0 - initial release
 */

#ifndef NoiascaCurrentLoop_h_
#define NoiascaCurrentLoop_h_

#define NOIASCA_CURRENT_LOOP_VERSION "NoiascaCurrentLoop 1.0.0"  // this library

#include <Arduino.h>

class CurrentLoopSensor {
  protected:
    const byte pin;                    // the pin
    const uint16_t resistor;           // Ohm of pulldown resistor
    const uint16_t vref;               // Reference Voltage * 10
    const byte measures = 1;           // keep in mind 1023 * measures has to fit in an int (time to new value: 10ms * 10)
    // const byte measures = 10;          // keep in mind 1023 * measures has to fit in an int (time to new value: 10ms * 10)
    const int maxValue;                // the maximum value the sensor can measure
    const int minAdc;                  // precalculation of minimum value of ADC
    const int maxAdc;                  // precalculation of maximum value of ADC
    int adc = 0;                       // previous measured raw ADC value

  public:
    CurrentLoopSensor(byte pin, uint16_t resistor, byte vref, uint16_t maxValue);
    int begin();                       // begin method - call in setup()
    void check();                      // checks if the resistor value fit to the other parameters
    int getAdc();                      // return the previous measured raw ADC value
    int getValue();                    // do the measurement and return the result
    int getValueUnfiltered();          // do the measurement witout filtering and return the result
    int getFilteredAdc();              // do the measurement and return the filtered ADC
};
#endif