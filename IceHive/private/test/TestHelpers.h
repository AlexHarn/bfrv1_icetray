/**
 * $Id: TestHelpers.cxx 124115 2014-10-03 16:42:30Z mzoll $
 * $Author: mzoll $
 * $Date: 2014-10-03 18:42:30 +0200 (fre, 03 okt 2014) $
 * $Revision: 124115 $
 *
 * A Unit test which generates some artificial test cases and let the Cleaning gnaw on them;
 */

#ifndef TESTHELPERS_H
#define TESTHELPERS_H

#define ICINTERSTRINGSPACING 125.
#define DCINTERSTRINGSPACING 72.17

#include "dataclasses/I3Constants.h"
#include "icetray/I3Units.h"

#include "dataclasses/physics/I3RecoPulse.h"

I3RecoPulse MakeRecoPulse (const double t, const double c, const double w=1., const uint8_t flags=0);

#endif // TESTHELPERS_H
