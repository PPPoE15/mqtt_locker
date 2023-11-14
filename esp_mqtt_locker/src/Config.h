#ifndef CONFIG_H
#define CONFIG_H

#include "WiFiDevice.h"
#include "ClassMessage.h"

#define RXD2 18 // uart1 pins for locker control
#define TXD2 19

#define statusHeader 0x80
#define   openHeader 0x8A
#define   openCode   0x11
#define statusCode   0x33

#define numLockersOnPlate 50
#define NUM_PLATES 1
#define NUM_BYTES 7

#define EMG_PIN 5
#define PROTO_STATE_ADDR 100

#endif // CONFIG_H