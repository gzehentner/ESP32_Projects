  //* *******************************************************************
//  Current2Waterlevel -- header
// 
//  Use Currentloop to measure the analog value
//  Convert to Waterlevel
//* *******************************************************************

#ifndef EvaluateSensor_h_
#define EvaluateSensor_h_ 

  extern String subject;
  extern String htmlMsg;
  
  void Current2Waterlevel();
  void beginCurrentLoopSensor();
  void ReadRelaisSetErrorLevel();
  void SetAlarmState_from_relais();

/*=================================*/
/* calculate PWM duty cycle for simulation of analog value 
  representing level in mm
  
  Input: level Waterlevel [in mm]
  Return: PWM duty cycle as number with 255 = 100%
  
  */
  float Waterlevel2dutyCycle (float level);
  
#endif

