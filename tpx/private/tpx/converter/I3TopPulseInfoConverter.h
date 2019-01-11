/*
 * copyright  (C) 2010
 * The Icecube Collaboration
 *
 * $Id$
 *
 * @version $Revision$
 * @date $LastChangedDate$
 * @author Fabian Kislat <fabian.kislat@desy.de> Last changed by: $LastChangedBy$
 */

#ifndef TPX_I3TOPPULSEINFOCONVERTER_H_INCLUDED
#define TPX_I3TOPPULSEINFOCONVERTER_H_INCLUDED

#include <tpx/I3TopPulseInfo.h>

I3_FORWARD_DECLARATION(I3TableRowDescription);
I3_FORWARD_DECLARATION(I3TableRow);

namespace convert {

  struct I3TopPulseInfo {
    typedef ::I3TopPulseInfo booked_type;

    void AddFields(I3TableRowDescriptionPtr desc, const booked_type& = booked_type());
    void FillSingleRow(const booked_type& pulseInfo, I3TableRowPtr row);
  };

}

#endif // TPX_I3TOPPULSEINFOCONVERTER_H_INCLUDED
