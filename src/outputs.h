#ifndef TimeAlarms_H
#define  TimeAlarms_H

#include <TimeAlarms.h>
class outputs {
public:
  static void enableChannelA();
  static void disableChannelA();
  static void enableChannelB();
  static void disableChannelB();
  static void enableAuxPump();
  static void disableAuxPump();
  static void setAuxPumpDelay(int);
  static String getOutputs();
};
#endif
