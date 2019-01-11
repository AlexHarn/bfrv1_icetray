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

#include "convert_I3IceTopBaseline.h"
#include <icetray/I3Units.h>
#include <tableio/converter/I3MapConverter.h>

namespace convert {

  void I3IceTopBaseline::AddFields(I3TableRowDescriptionPtr desc, const booked_type&)
  {
    MAKE_ENUM_VECTOR(source, ::I3Waveform, Source, I3WAVEFORM_H_I3Waveform_Source);
    desc->AddEnumField< ::I3Waveform::Source >("source", source, "", "Is this an ATWD or an FADC baseline?");
    desc->AddField<uint8_t>("channel", "", "ATWD channel");
    desc->AddField<uint8_t>("source_id", "", "Source index of the waveform, e.g. ATWD chip ID");
    desc->AddField<double>("baseline", "mV", "Baseline value");  // people expect doubles
    desc->AddField<double>("slope", "mV/ns", "Baseline slope");
    desc->AddField<double>("rms", "mV", "Baseline RMS variation");
  }

  void I3IceTopBaseline::FillSingleRow(const booked_type& bl, I3TableRowPtr row) 
  {
    row->Set< ::I3Waveform::Source >("source", bl.source);
    row->Set<uint8_t>("channel", bl.channel);
    row->Set<uint8_t>("source_id", bl.sourceID);
    row->Set<double>("baseline", bl.baseline/I3Units::mV);
    row->Set<double>("slope", bl.slope/(I3Units::mV/I3Units::ns));
    row->Set<double>("rms", bl.rms/I3Units::mV);
  }

}
