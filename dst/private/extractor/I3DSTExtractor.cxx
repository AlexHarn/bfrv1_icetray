#include "dst/extractor/I3DSTExtractor.h"
#include "recclasses/I3DST.h"
#include "dataclasses/physics/I3Trigger.h"
#include "dataclasses/TriggerKey.h"
#include "dataclasses/physics/I3TriggerHierarchy.h"


using namespace std;

void I3DSTExtractorUtils::SetTrigger(I3TriggerHierarchyPtr& triggers, uint16_t triggertag) 
{
  // Create a new trigger object to record the result, and set the variables
  I3TriggerHierarchy::iterator mTriter;
  I3Trigger Trigger;
  I3Trigger inTrigger;
  I3Trigger topTrigger;
  I3Trigger twrTrigger;
  I3Trigger spaseTrigger;

  // set trigger key of top-level trigger
  Trigger.GetTriggerKey() = TriggerKey(TriggerKey::GLOBAL, TriggerKey::MERGED);
  mTriter = triggers->insert(triggers->begin(), Trigger);

  uint16_t mask;

  if ( DSTUtils::isInIce(triggertag) ) {
			mask = triggertag >> DST_IN_ICE;
			inTrigger.SetTriggerFired(true);
			if ( mask & DST_SIMPLE_MULTIPLICITY ) {
				inTrigger.GetTriggerKey() = 
					TriggerKey(TriggerKey::IN_ICE,TriggerKey::SIMPLE_MULTIPLICITY);
				triggers->append_child(mTriter, inTrigger);
			} if ( mask & DST_MIN_BIAS ) {
				inTrigger.GetTriggerKey() = 
					TriggerKey(TriggerKey::IN_ICE,TriggerKey::MIN_BIAS); 
				triggers->append_child(mTriter, inTrigger);
			} 
  } if ( DSTUtils::isIceTop(triggertag) ) {
			mask = triggertag >> DST_ICE_TOP;
			topTrigger.SetTriggerFired(true);
			if ( mask & DST_SIMPLE_MULTIPLICITY) {
				topTrigger.GetTriggerKey() = 
					TriggerKey(TriggerKey::ICE_TOP,TriggerKey::SIMPLE_MULTIPLICITY);
				triggers->append_child(mTriter, topTrigger);
			} if ( mask & DST_MIN_BIAS ) {
				topTrigger.GetTriggerKey() = 
					TriggerKey(TriggerKey::ICE_TOP,TriggerKey::MIN_BIAS); 
				triggers->append_child(mTriter, topTrigger);
			}
  } if ( DSTUtils::isTWR(triggertag) ) {
			mask = triggertag >> DST_AMANDA_TWR_DAQ;
			twrTrigger.SetTriggerFired(true);
			if ( mask & DST_FRAGMENT_MULTIPLICITY ) {
				twrTrigger.GetTriggerKey() = 
					TriggerKey(TriggerKey::AMANDA_TWR_DAQ,TriggerKey::FRAGMENT_MULTIPLICITY);
				triggers->append_child(mTriter, twrTrigger);
			} 
  } if ( DSTUtils::isSPASE(triggertag) ) {
			mask = triggertag >> DST_SPASE;
			spaseTrigger.SetTriggerFired(true);
			if ( mask & DST_SPASE_2 ) {
				spaseTrigger.GetTriggerKey() = 
					TriggerKey(TriggerKey::SPASE,TriggerKey::SPASE_2);
				triggers->append_child(mTriter, spaseTrigger);
			} 
	}
}

