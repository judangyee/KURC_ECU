#include "fuel_calcs.h"
#include "maths.h"
#include "unit_testing.h"
#include "globals.h"
#include "decoders.h"
#include "units.h"
#include "scheduler_fuel_controller.h"

TESTABLE_INLINE_STATIC uint16_t calculateRequiredFuel(const config2 &page2, const statuses &current) {
  uint16_t reqFuel = page2.reqFuel * 100U; //Convert to uS and an int. This is the only variable to be used in calculations
  if ((page2.strokes == FOUR_STROKE) && (current.injLayout != INJ_SEQUENTIAL))
  {
    //Default is 1 squirt per revolution, so we halve the given req-fuel figure (Which would be over 2 revolutions)
    //The req_fuel calculation above gives the total required fuel (At VE 100%) in the full cycle.
    //If we're doing more than 1 squirt per cycle then we need to split the amount accordingly.
    //(Note that in a non-sequential 4-stroke setup you cannot have less than 2 squirts as you cannot determine the
    //stroke to make the single squirt on)
    reqFuel = reqFuel / 2U; 
  }

  return reqFuel;
}

TESTABLE_INLINE_STATIC uint16_t calculatePWLimit(const config2 &page2, const statuses &current)
{
  uint32_t tempLimit = percentageApprox(page2.dutyLim, current.revolutionTime); //The pulsewidth limit is determined to be the duty cycle limit (Eg 85%) by the total time it takes to perform 1 revolution
  //Handle multiple squirts per rev
  if (page2.strokes == FOUR_STROKE) { tempLimit = tempLimit * 2U; }
  //Optimise for power of two divisions where possible
  switch(current.nSquirts)  {
    case 1:
      //No action needed
      break;
    case 2:
      tempLimit = tempLimit >> 1U;
      break;
    case 4:
      tempLimit = tempLimit >> 2U;
      break;
    case 8:
      tempLimit = tempLimit >> 3U;
      break;
    default:
      //Non-PoT squirts value. Perform (slow) uint32_t division
      tempLimit = fast_div (tempLimit, current.nSquirts);
      break;
  }
  return (uint16_t)min(tempLimit, (uint32_t)UINT16_MAX);
}


static inline uint32_t applyMapMode(uint32_t intermediate, const config2 &page2, const statuses &current) {
  if ( page2.multiplyMAP == MULTIPLY_MAP_MODE_100) { 
    uint16_t multiplier = div100(lshift<7U>((uint32_t)current.MAP));
    return rshift<7U>(intermediate * (uint32_t)multiplier); 
  }
  if( page2.multiplyMAP == MULTIPLY_MAP_MODE_BARO) { 
     uint16_t multiplier = fast_div32_16(lshift<7U>((uint32_t)current.MAP), current.baro); 
    return rshift<7U>(intermediate * (uint32_t)multiplier); 
  }
  return intermediate;
}

static inline uint32_t applyAFRMultiplier(uint32_t intermediate, const config2 &page2, const config6 &page6, const statuses &current) {
  if (page2.includeAFR == true) {
    if ((page6.egoType == EGO_TYPE_WIDE) && (current.runSecs > page6.ego_sdelay) ) {
      uint16_t multiplier = fast_div(lshift<7U>((uint16_t)current.O2), current.afrTarget);  //Include AFR (vs target) if enabled
      return rshift<7U>(intermediate * (uint32_t)multiplier); 
    }
  } else {
    if ( page2.incorporateAFR ) {
      uint16_t multiplier = fast_div(lshift<7U>((uint16_t)page2.stoich), current.afrTarget);  //Incorporate stoich vs target AFR, if enabled.
      return rshift<7U>(intermediate * (uint32_t)multiplier); 
    }
  }
  return intermediate;
}

static inline uint32_t applyCorrections(uint32_t intermediate, uint16_t corrections) {
  return percentageApprox(corrections, intermediate); 
}

static inline uint16_t computeInitialPw(uint16_t REQ_FUEL, uint8_t VE) {
  // REQ_FUEL max is 255*100 = 25500
  // VE max is 255
  // Therefore max result = 65025. So just fits in a uint16_t.
  return percentageApprox(VE, REQ_FUEL); 
}

static inline uint32_t includeOpenTime(uint32_t intermediate, uint16_t injOpen) {
    return intermediate + injOpen; //Add the injector opening time
}

static inline uint32_t includeAe(uint32_t intermediate, uint16_t REQ_FUEL, const config2 &page2, const statuses &current) {
  // We need to add Acceleration Enrichment pct increase if the engine is accelerating
  if ((current.isAcceleratingTPS) && (page2.aeApplyMode == AE_MODE_ADDER) && (current.AEamount>100U)) {
    return intermediate + percentageApprox((uint16_t)(current.AEamount - 100U), REQ_FUEL);
  }
  return intermediate;
}

TESTABLE_INLINE_STATIC uint16_t calcPrimaryPulseWidth(uint16_t injOpenTime, const config2 &page2, const config6 &page6, const config10 &page10, const statuses &current) {
  uint16_t REQ_FUEL = calculateRequiredFuel(page2, current);

  //Standard float version of the calculation
  //return (REQ_FUEL * (float)(VE/100.0) * (float)(MAP/100.0) * (float)(TPS/100.0) * (float)(corrections/100.0) + injOpenTime);
  //Note: The MAP and TPS portions are currently disabled, we use VE and corrections only
  uint32_t intermediate = 
    includeAe(
      includeOpenTime(
        applyCorrections(
          applyAFRMultiplier(
            applyMapMode(
              computeInitialPw(REQ_FUEL, current.VE), 
              page2, current),
            page2, page6, current), 
        current.corrections), 
      injOpenTime), 
      REQ_FUEL, page2, current);

  // Make sure this won't overflow when we convert to uInt. This means the maximum pulsewidth possible is 65.535mS
  return (uint16_t)min(intermediate, (uint32_t)UINT16_MAX);
}

// Apply the pwLimit when the engine is not cranking
TESTABLE_INLINE_STATIC uint16_t applyPwLimits(uint16_t pw, uint16_t pwLimit, const config10 &page10, const statuses &current) {
  (void)page10; //Staged injection was removed from this fork; kept as a parameter for call-site/test compatibility
  if(current.rotationStatus!=EngineRotationStatus::Cranking) {
    return min(pw, pwLimit);
  }
  return pw;
}

// Staged injection was removed from this fork (single-cylinder build with a single injector).
// The secondary injector channel is never used, so its pulsewidth is always 0.
TESTABLE_INLINE_STATIC pulseWidths calculateSecondaryPw(uint16_t primaryPw, uint16_t pwLimit, uint16_t injOpenTime, const config2 &page2, const config10 &page10, const statuses &current) {
  (void)pwLimit;
  (void)injOpenTime;
  (void)page2;
  (void)page10;
  (void)current;
  return { primaryPw, 0U };
}

TESTABLE_INLINE_STATIC uint16_t calculateOpenTime(const config2 &page2, const statuses &current) {
  // Convert injector open time from tune to microseconds & apply voltage correction if required
  return page2.injOpen * current.batCorrection; 
}

pulseWidths computePulseWidths(const config2 &page2, const config6 &page6, const config10 &page10, const statuses &current) {
  if (current.corrections!=0U) {
    uint16_t pwLimit = calculatePWLimit(page2, current);
    uint16_t injOpenTime = calculateOpenTime(page2, current);
    uint16_t primaryPw = applyPwLimits(calcPrimaryPulseWidth( injOpenTime, 
                                                              page2,
                                                              page6,
                                                              page10, 
                                                              current),
                                        pwLimit,
                                        page10,
                                        current);
    return calculateSecondaryPw(primaryPw, pwLimit, injOpenTime, page2, page10, current);  
  }
  return { 0U, 0U };
}