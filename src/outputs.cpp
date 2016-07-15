#include "outputs.h"

int m_aux_pump_delay = 0;

void outputs::enableChannelA() {
  Alarm.delay(m_aux_pump_delay);
}
void outputs::disableChannelA() {

}
void outputs::enableChannelB() {

}
void outputs::disableChannelB() {

}
void outputs::enableAuxPump() {

}
void outputs::disableAuxPump() {

}
void outputs::setAuxPumpDelay(int delay) {
  m_aux_pump_delay = delay;
}
