  //* *******************************************************************
//  Current2Waterlevel -- header
// 
//  Use Currentloop to measure the analog value
//  Convert to Waterlevel
//* *******************************************************************

#ifndef Current2Waterlevel_h_
#define  Current2Waterlevel_h_ 
  void Current2Waterlevel();
  void beginCurrentLoopSensor();

/*=================================*/
/* calculate PWM duty cycle for simulation of analog value 
  representing level in mm
  
  Input: level Waterlevel [in mm]
  Return: PWM duty cycle as number with 255 = 100%
  
  */
  float Waterlevel2dutyCycle (float level);
  
#endif

