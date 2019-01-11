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

#include "I3TopPulseInfoConverter.h"
#include <tableio/I3TableRow.h>
#include <tableio/I3TableRowDescription.h>
#include <icetray/I3Units.h>

namespace convert {

  void I3TopPulseInfo::AddFields(I3TableRowDescriptionPtr desc, const booked_type&)
  {
    MAKE_ENUM_VECTOR(status, ::I3TopPulseInfo, Status, I3TOPPULSEINFO_H_I3TopPulseInfo_Status);

    desc->AddField<double>("amplitude", "mV", "Amplitude of the IceTop waveform");
    desc->AddField<double>("rise_time", "ns", "Rise time of the waveform");
    desc->AddField<double>("trailing_edge", "ns", "Trailing edge of the waveform");
    desc->AddEnumField< ::I3TopPulseInfo::Status >("status", status, "", "");
    desc->AddField<uint8_t>("channel", "", "ATWD channel");
    desc->AddField<uint8_t>("source_id", "", "Source index of the waveform, e.g. ATWD chip ID");
  }

  void I3TopPulseInfo::FillSingleRow(const booked_type& pulseInfo, I3TableRowPtr row)
  {
    row->Set<double>("amplitude", pulseInfo.amplitude/I3Units::mV);
    row->Set<double>("rise_time", pulseInfo.risetime/I3Units::ns);
    row->Set<double>("trailing_edge", pulseInfo.trailingEdge/I3Units::ns);
    row->Set< ::I3TopPulseInfo::Status >("status", pulseInfo.status);
    row->Set<uint8_t>("channel", pulseInfo.channel);
    row->Set<uint8_t>("source_id", pulseInfo.sourceID);
  }

}
