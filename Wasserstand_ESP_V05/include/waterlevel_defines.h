/*

Some basic defines for the whole project

*/
// #define Xyes 1
// #define Xno  0

// is defined in platformio.ini
    // #define isLiveSystem 0

#ifndef WATERLEVEL_DEFINES_H
  #define WATERLEVEL_DEFINES_H

  #define VERSION "6.1" // the version of this sketch

  #define internADC 0
  #define ADS1115_ADC   1


  #ifdef ARDUINO_ARCH_ESP32               // if it is not a board with ESP32, then a
    #define BOARDTYPE ESP32
    // defined in platformio.ini 
    // #define MyUSE_ADC ADS1115_ADC
    //
  #else
    #define BOARDTYPE ESP8266                // Board with ESP8266 is used (internal ADC)
    // defined in platformio.ini 
    // #define MyUSE_ADC internADC
  #endif

#if MyUSE_ADC == ADS1115_ADC
  #if BOARDTYPE == ESP32
    #include <Wire.h>
    #include <Adafruit_Sensor.h>
    #include <Adafruit_ADS1X15.h>
  #else
    #include <Wire.h>
    #include <ADS1X15.h>
  #endif
#endif

  /* *******************************************************************
          the board settings / die Einstellungen der verschiedenen Boards
  ********************************************************************/
  #if isLiveSystem == 1
    #define TXT_BOARDID "164"                   // an ID for the board
    #define TXT_BOARDNAME "Wasserstand " // the name of the board
    #define CSS_MAINCOLOR "blue"                // don't get confused by the different webservers and use different colors
  #else
    #define TXT_BOARDID "Develop"           // show that something is configured as development version
    #define TXT_BOARDNAME "Wasserstand" // the name of the board
    #define CSS_MAINCOLOR "blue"                // don't get confused by the different webservers and use different colors
  #endif
  /*=================================================================*/
  /* Prepare WaterLevel Application */


  
  #if BOARDTYPE == ESP8266
    /* -- Pin-Def for ESP8266 -- */
    #define GPin_AHH   3 // weiß    // RX
    #define GPin_AH   13 // grau    // D7
    #define GPin_AL   12 // violett // D6
    // #define GPin_ALL  14 // blau   
    // #define GPout_GND 12 // schwarz
  #else
    /* -- Pin-Def for ESP32 -- */  // Board159
    #define GPin_AHH  2  // blau   rot
    #define GPin_AH  13  // grau   grün
    #define GPin_AL  12  // lila   gelb
    // #define GPin_ALL 16  // grün   orange (GPIO16 is not usable, because it is used for PSRAM)
  
  #endif

 
  // definitions for analog-digital conversion
  // #if MyUSE_ADC == ADS1115_ADC
    // #define ADC_BIT  15  // only 15 bit for single ended signals
    // #define Ain_Level 0  // input = adc0

    // #if BOARDTYPE == ESP32
    //   #define GET_ANALOG ads.readADC_Differential_0_1()
    //   #define I2C_SDA 14
    //   #define I2C_SCL 15
    // #else
    //   #define GET_ANALOG ads.readADC(Ain_Level)
    //   // #define I2C_SDA 13
    //   // #define I2C_SCL 15
    // #endif

      // definitions for analog-digital conversion
    #if MyUSE_ADC == ADS1115_ADC
      #define ADC_BIT  15  // only 15 bit for single ended signals
      #define Ain_Level 0  // input = adc0

      #if BOARDTYPE == ESP32
        #define GET_ANALOG ads.readADC_Differential_0_1()
        #define I2C_SDA 14
        #define I2C_SCL 15
      #else
        #define GET_ANALOG ads.readADC(0)
        #define I2C_SDA 13
        #define I2C_SCL 15
      #endif

  #else // use builtin ADC
    #if BOARDTYPE == ESP32
      Builtin ADC cannot be used with ESP32
    #endif

    #define GET_ANALOG analogRead(pin)
    #define ADC_BIT 10
    #define Ain_Level PIN_A0
    #define PWM_OUT 1 // PWM-Output to IO1
  #endif

  #if BOARDTYPE == ESP32
        // pin of builtin led
    const int builtin_led = 4;
  #else
    #define builtin_led 2 
    #define BLUE_LED builtin_led
  #endif

  #if isLiveSystem == 1
  #else
    // #define DEBUG_PRINT_HEAP
  
    // #define DEBUG_PRINT_RAW  // print raw voltage values without calculating current values; used for debug ADC function
    #define SIM_VALUES  // use small values for loops to get a fast simulation of firmware
    // #define SIM_FADING_LEVEL  // generate a fading value for analog input to simulate funkctions without an external electronic
  #endif

  #if BOARDTYPE == ESP32
    // length of ring buffer
    #define iRingValueMax  1000 // 50 // new value every three minutes --> 720: buffer for one complete day (but then we get heap overflow)
    #define iLongtermRingValueMax 120 // 10// 120 // 370 // one value a day, buffer for one year (now we have four values a day so we have one month)
    // maximum lines to be printed and points in graph
    const int maxLines  = 1000;
    const int maxPoints = 1000;
  #else // BOARDTYPE == ESP8266
    // length of ring buffer
    #define iRingValueMax  100 // 50 // new value every three minutes --> 720: buffer for one complete day (but then we get heap overflow)
    #define iLongtermRingValueMax 120 // 10// 120 // 370 // one value a day, buffer for one year (now we have four values a day so we have one month)
    // maximum lines to be printed and points in graph
    const int maxLines  = 100;
    const int maxPoints = 100;
  #endif

  #ifdef SIM_VALUES
    const int  filterCntMax = 1;  //  GZE: zum Test ganz ohne Filter!!       // time for myvalue: measureInterval * filterCntMax  
    // const int  filterCntMax = 100;        // time for myvalue: measureInterval * filterCntMax  
    const unsigned long  longtermInterval = 10000;  // time between two saved values in ms
  #else
    const int  filterCntMax = 10; // time for myvalue: measureInterval * filterCntMax  1200 : every three minutes
    const unsigned long  longtermInterval = 1000*60*60*6; // four times a day
  #endif

  /* -- Alarm-Level -- */
  // Pegel wurde verändert, hängt um 6cm höher
  // --> Werte korrigieren
  #define Level_AHH 185 // Oberkante Schacht = 187cm // Oberkante Bodenplatte = (OK-Schacht + 8 cm) = 1,95cm
  #define Level_AH  170  // Warnschwelle
  #define Level_AL  155
  #define Level_ALL 140 // Unterkante KG Rohr
                        // Aktueller Niedrig-Stand Nov 2023 = 99 cm

  // #define BUTTON1_PIN 19

  // value offset
  // adapt measured value to TS [mm]
  #define valOffset -15

#endif
