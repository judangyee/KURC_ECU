#include "starterController.h"
#include "../../pins/boardOutputPin.h"
#include "../../../unit_testing.h"
#include "../../../globals.h"

TESTABLE_STATIC boardOutputPin_t starter_pin;
TESTABLE_STATIC uint8_t starterButtonPin = NOT_A_PIN;
TESTABLE_STATIC bool starterButtonPrevious;
TESTABLE_STATIC bool starterLatched;
TESTABLE_STATIC uint8_t starterCrankTicks; //Counts starterControl() calls while cranking. Called at 10Hz, so 1 tick == 0.1s, matching configPage15.starterMaxCrankTime's units

static inline bool __attribute__((optimize("Os"))) starterButtonPressed(void)
{
  bool rawState = digitalRead(starterButtonPin);
  return (configPage15.starterButtonPolarity != 0U) ? rawState : !rawState; //0 = active low, 1 = active high
}

static inline void starterRelayOn(void)
{
  ((configPage15.starterOutputInverted != 0U) ? starter_pin.setPinLow() : starter_pin.setPinHigh());
}
static inline void starterRelayOff(void)
{
  ((configPage15.starterOutputInverted != 0U) ? starter_pin.setPinHigh() : starter_pin.setPinLow());
}

void __attribute__((optimize("Os"))) initialiseStarterControl(uint8_t buttonPin, uint8_t outputPin)
{
  starterButtonPin = buttonPin;
  starter_pin.setPin(outputPin, OUTPUT);
  starterLatched = false;
  starterCrankTicks = 0;
  starterButtonPrevious = false;
  starterRelayOff(); //Initialise with the starter relay off
}

void starterControl(void)
{
  if ( (configPage15.starterEnabled == 0U) || (starterButtonPin == NOT_A_PIN) )
  {
    if (starterLatched) { starterLatched = false; starterRelayOff(); } //Feature was disabled mid-crank; make safe
    return;
  }

  bool buttonPressed = starterButtonPressed();
  bool buttonRisingEdge = buttonPressed && !starterButtonPrevious;
  starterButtonPrevious = buttonPressed;

  if (!starterLatched)
  {
    //Only start a new crank attempt if the engine isn't already running
    if (buttonRisingEdge && (currentStatus.rotationStatus != EngineRotationStatus::Running))
    {
      starterLatched = true;
      starterCrankTicks = 0;
      starterRelayOn();
    }
  }
  else
  {
    bool engineCaught = (currentStatus.rotationStatus == EngineRotationStatus::Running);
    bool timedOut = (configPage15.starterMaxCrankTime != 0U) && (starterCrankTicks >= configPage15.starterMaxCrankTime);
    bool cancelled = buttonRisingEdge; //Pressing the button again mid-crank cancels it

    if (engineCaught || timedOut || cancelled)
    {
      starterLatched = false;
      starterRelayOff();
    }
    else if (starterCrankTicks < UINT8_MAX)
    {
      ++starterCrankTicks;
    }
  }
}
