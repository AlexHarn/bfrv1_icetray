/*
 * copyright  (C) 2011
 * The Icecube Collaboration
 *
 * $Id$
 *
 * @version $Revision$
 * @date $LastChangedDate$
 * @author Fabian Kislat <fabian.kislat@desy.de> Last changed by: $LastChangedBy$
 */

#ifndef TPX_CONVERTER_CONVERT_I3ICETOPBASELINE_H_INCLUDED
#define TPX_CONVERTER_CONVERT_I3ICETOPBASELINE_H_INCLUDED

#include <tableio/I3TableRowDescription.h>
#include <tableio/I3TableRow.h>
#include <tpx/I3IceTopBaseline.h>

namespace convert {

  struct I3IceTopBaseline {
    typedef ::I3IceTopBaseline booked_type;

    void AddFields(I3TableRowDescriptionPtr desc, const booked_type& = booked_type());
    void FillSingleRow(const booked_type& dl, I3TableRowPtr row);
  };

}

#endif // TPX_CONVERTER_CONVERT_I3ICETOPBASELINE_H_INCLUDED
