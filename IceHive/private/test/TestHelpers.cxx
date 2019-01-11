/**
 * $Id: TestHelpers.cxx 124115 2014-10-03 16:42:30Z mzoll $
 * $Author: mzoll $
 * $Date: 2014-10-03 18:42:30 +0200 (fre, 03 okt 2014) $
 * $Revision: 124115 $
 *
 * A Unit test which generates some artificial test cases and let the Cleaning gnaw on them;
 */

#include "TestHelpers.h"

I3RecoPulse MakeRecoPulse (const double t, const double c, const double w, const uint8_t flags) {
  I3RecoPulse p;
  p.SetTime(t);
  p.SetCharge(c);
  p.SetWidth(w);
  p.SetFlags(flags);
  return p;
}