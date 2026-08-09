/*
Speeduino - Simple engine management for the Arduino Mega 2560 platform
Copyright (C) Josh Stewart
A full copy of the license may be found in the projects root directory
*/

/* Secondary serial / external CAN relay support was removed from this fork
 * (single-cylinder build, no dash/CAN relay hardware attached).
 *
 * pSecondarySerial is kept defined because board_avr2560.cpp (and the other
 * board_*.cpp files) unconditionally point it at a hardware serial port
 * during board init. secondserial_Command()/sendCancommand() are kept as
 * no-ops for the same reason - board_teensy41.cpp references secondarySerial
 * (the macro for *pSecondarySerial) directly, and init.cpp no longer calls
 * secondarySerial.begin(), so the port is never actually activated.
 */
#include "globals.h"
#include "comms_secondary.h"
#include "board_definition.h"

SECONDARY_SERIAL_T* pSecondarySerial;

void secondserial_Command(void) { }
void sendCancommand(uint8_t cmdtype, uint16_t canaddress, uint8_t candata1, uint8_t candata2, uint16_t sourcecanAddress)
{
  (void)cmdtype;
  (void)canaddress;
  (void)candata1;
  (void)candata2;
  (void)sourcecanAddress;
}
