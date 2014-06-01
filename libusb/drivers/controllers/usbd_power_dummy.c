#include <usbd_power.h>

/** USB device without power management **/

int PWRconfigure(unsigned prio, unsigned subprio, int clk) {
  return 0;
}

void PWRmanagementEnable() {
}

void PWRreduce() {
}

void PWRresume() {
}

remote_wakeup_t PWRgetRemoteWakeUp() {
  return RW_DISABLED;
}

void PWRsetRemoteWakeUp(remote_wakeup_t rw) {
}

void PWRremoteWakeUp() {
}
