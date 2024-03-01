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
 
#include <NoiascaCurrentLoop.h>

CurrentLoopSensor::CurrentLoopSensor(byte pin, uint16_t resistor, byte vref, uint16_t maxValue) :
  pin (pin),
  resistor (resistor),
  vref (vref),
  maxValue (maxValue),
  minAdc (210), // (0.004 * resistor * 1024 / (vref / 10.0)),
  maxAdc (0.020 * resistor * 1024 / (vref / 10.0))
  {}

int CurrentLoopSensor::begin()
{
  pinMode(pin, INPUT);
  return 1;        // assume success
}

void CurrentLoopSensor::check()
{
  byte err = 0;
  if (minAdc < 0)
  {
    Serial.println(F("[Sensor] E:resistor might be to low for your VREF"));
    err++;
  }
  if (maxAdc > 1024 - 1) {
    Serial.println(F("[Sensor] E:resistor might be to large for your VREF"));
    err++;
  }
  if (err == 0)
    Serial.println(F("[Sensor] I:parameters ok, you can remove .check() from setup"));
  else
  {
    Serial.print(F("minAdc=")); Serial.println(minAdc);
    Serial.print(F("maxAdc=")); Serial.println(maxAdc);
  }
}

/*
   return the previous measured raw ADC value
 */
int CurrentLoopSensor::getAdc()
{
  // return adc;
  return 10; // GZE analogRead(pin);
}

/*
   do the measurement and return the result
 */
int CurrentLoopSensor::getValue()
{
  adc = 0;
  for (byte i = 0; i < measures; i++)
  {
// GZE    adc += analogRead(pin);
    delay(10);
  }
  adc = adc / measures;
  //int32_t value = (adc - 186) * 500L / (931 - 186);                          // for 1023*500 we need a long
  int32_t value = (adc - minAdc) * int32_t(maxValue) / (maxAdc - minAdc);      // for 1023*500 we need a long  // -> pressure
  if (value > maxValue) value = maxValue;
  else if (value < 0) value = 0;
  return  value;
}

/*
   do the measurement and return the ADC value
 */
int CurrentLoopSensor::getFilteredAdc()
{
  adc = 0;
  for (byte i = 0; i < measures; i++)
  {
    // GZE adc += analogRead(pin);
    delay(10);
  }
  adc = adc / measures;
  return  adc;
}