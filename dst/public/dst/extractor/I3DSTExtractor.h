/**
    copyright  (C) 2007
    the icecube collaboration

    @date $Date: 2007-07-30 $
    @author juancarlos@icecube.wisc.edu

    @brief I3Module that reads an I3DSTHeader and subsequent I3DST object from
    frame and does one or more fo the following:
        <ul>
        <li>writes a TDST tree to a ROOT file 
        <li>writes either a .dst or .zdst (compressed binary dst format)
        <li>partially recreates original I3Frame objects from which it extracted information.
        </ul>
*/

#include "dataclasses/physics/I3Trigger.h"
#include "dataclasses/physics/I3TriggerHierarchy.h"

#ifndef DST_I3DSTMODULE_H_INCLUDED
#define DST_I3DSTMODULE_H_INLCUDED

using namespace std;


namespace I3DSTExtractorUtils {
  /**
    * Add appropiate I3Triggers to I3TriggerHierarchy based on the dst
    * triggertag.
    *
    * @param triggers I3TriggerHierarchy to which trigger will be added
    * @param triggertag 2-byte segment which encodes the trigger information in dst
    */
  void SetTrigger(I3TriggerHierarchyPtr& triggers, uint16_t triggertag);
}

#endif
