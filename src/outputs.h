#ifndef TimeAlarms_H
#define  TimeAlarms_H

#include <TimeAlarms.h>
class outputs {
public:
  static void enableChannelA();
  static void disableChannelA();
  static void enableChannelB();
  static void disableChannelB();
  static void enableChannelC();
  static void disableChannelC();
  static void enableChannelD();
  static void disableChannelD();
  static void enableChannelE();
  static void disableChannelE();
  
  static void enableAuxExit();
  static void disableAuxExit();
  static void setAuxPumpDelay(int);
  static String getOutputs();
  static bool outputsChanged;
};
#endif
