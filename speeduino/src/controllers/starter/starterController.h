#pragma once

#include <stdint.h>

/** @brief Set up the starter button input and starter relay output pins. */
void initialiseStarterControl(uint8_t buttonPin, uint8_t outputPin);

/** @brief Push-to-start state machine. Call periodically (10Hz is sufficient).
 *
 * A rising edge on the starter button latches the starter relay output on.
 * The latch clears automatically once the engine catches (rotationStatus
 * becomes Running), if the configured max crank time is exceeded (safety
 * cutoff), or if the button is pressed again while cranking (manual cancel).
 */
void starterControl(void);
