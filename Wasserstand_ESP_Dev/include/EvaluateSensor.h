  //* *******************************************************************
//  Current2Waterlevel -- header
// 
//  Use Currentloop to measure the analog value
//  Convert to Waterlevel
//* *******************************************************************

#ifndef EvaluateSensor_h_
#define EvaluateSensor_h_ 

  extern int printOnChangeActive;
  extern int valueStable;

  extern String subject;
  extern String htmlMsg;
  
  void Current2Waterlevel();
  void beginCurrentLoopSensor();
  void ReadRelaisSetErrorLevel();
  void SetAlarmState_from_relais();
  void SetAlarmState_from_level();
  void calculateWorstCaseAlarmState();
  void prepareSendMail ();
  void setPegelforSimulation();


  
#endif

