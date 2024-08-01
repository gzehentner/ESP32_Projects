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
#include <waterlevel_defines.h>


#include <waterlevel.h>

#if MyUSE_ADC == ADS1115_ADC
  #if BOARDTYPE == ESP32
    #include <Wire.h>
    #include <Adafruit_Sensor.h>
    #include <Adafruit_ADS1X15.h>
    const int maxAdc_value = 0x7FFF;  // 15bit ADC
    const float mili_volt_per_bit = 0.125 ;  // with gain = 1
  #else
    #include <Wire.h>
    #include <ADS1X15.h>
    const int maxAdc_value = 0x7FFF;  // 15bit ADC
    const float mili_volt_per_bit = 0.125 ;  // with gain = 1
  #endif
#else
  const int maxAdc_value = 0x3FF;   // 10bit ADC
#endif

CurrentLoopSensor::CurrentLoopSensor(byte pin, uint16_t resistor, byte vref, uint16_t maxValue) :
  pin (pin),
  resistor (resistor),
  vref (vref),
  maxValue (maxValue),
  #if MyUSE_ADC == ADS1115_ADC
    minAdc (0.004 * resistor / mili_volt_per_bit * 1000),
    maxAdc (0.020 * resistor / mili_volt_per_bit * 1000)
  #else
    minAdc (210), // (0.004 * resistor * maxAdc_value / (vref / 10.0)),
    maxAdc (0.020 * resistor * maxAdc_value / (vref / 10.0))
  #endif
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
  if (maxAdc > maxAdc_value - 1) {
    Serial.println(F("[Sensor] E:resistor might be to large for your VREF"));
    err++;
  }
  if (err == 1)
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
  return GET_ANALOG;
}

/*
   do the measurement and return the result
 */
int CurrentLoopSensor::getValue()
{
  adc = 0;
  for (byte i = 0; i < measures; i++)
  {
    adc += GET_ANALOG;
    // delay(10);
  }
  adc = adc / measures;
  //int32_t value = (adc - 186) * 500L / (931 - 186);                          // for 1023*500 we need a long
  int32_t value = (adc - minAdc) * int32_t(maxValue) / (maxAdc - minAdc);      // for 1023*500 we need a long  // -> pressure
  if (value > maxValue) value = maxValue;
  else if (value < 0) value = 0;
  return  value;
}

// get the sensor value without filtering --> filtering can be done in calling software
int CurrentLoopSensor::getValueUnfiltered()
{
  adc = 0;
  adc = GET_ANALOG;
  
  //int32_t value = (adc - 186) * 500L / (931 - 186);                          // for 1023*500 we need a long
  int32_t value = (adc - minAdc) * int32_t(maxValue) / (maxAdc - minAdc);      // for 1023*500 we need a long  // -> pressure
  value += valOffset;
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
    adc += GET_ANALOG;
    // only one measure; filtering is done on the upper side
    // delay(10);
  }
  adc = adc / measures;
  return  adc;
}