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


#include "tpx/I3IceTopBaseline.h"
#include <icetray/I3Units.h>
#include <cassert>
#include <cmath>


I3IceTopBaseline::I3IceTopBaseline()
  : source(I3Waveform::ATWD), channel(0), sourceID(0),
    baseline(NAN), slope(NAN), rms(NAN)
{}

I3IceTopBaseline::I3IceTopBaseline(I3Waveform::Source source_, uint8_t channel_,
				   uint8_t sourceID_)
  : source(source_), channel(channel_), sourceID(sourceID_),
    baseline(NAN), slope(NAN), rms(NAN)
{}


// store information more efficiently
union packed_waveform_source_information {
  struct {
#if BYTE_ORDER == BIG_ENDIAN
    uint8_t slop    : 2;
    uint8_t id      : 1;
    uint8_t channel : 2;
    uint8_t source  : 3;
#else
    uint8_t source  : 3; /* Source ID (ATWD/FADC/...) */
    uint8_t channel : 2; /* ATWD channel */
    uint8_t id      : 1; /* Source id (ATWD Chip) */
    uint8_t slop    : 2; /* Unused space */
#endif
  } fields;
  uint8_t bits;
} __attribute((packed));


template <class Archive>
void I3IceTopBaseline::serialize(Archive& ar, unsigned version)
{
  if (version > i3icetopbaseline_version_)
    log_fatal("Attempting to read version %u from file but running version %u "
	      "of I3IceTopBaseline class.", version, i3icetopbaseline_version_);

  assert((uint8_t)source < (2<<3));  // using 3 bits
  assert(channel < (2<<2));          // using 2 bits
  assert(sourceID < 2);              // using 1 bit 

  packed_waveform_source_information packed_source;
  packed_source.fields.source = source;
  packed_source.fields.channel = channel;
  packed_source.fields.id = sourceID;
  
  ar & make_nvp("source", packed_source.bits);

  source = (I3Waveform::Source)(unsigned)packed_source.fields.source;
  channel = packed_source.fields.channel;
  sourceID = packed_source.fields.id;

  ar & make_nvp("baseline", baseline);
  ar & make_nvp("slope", slope);
  ar & make_nvp("rms", rms);
}

I3_SERIALIZABLE(I3IceTopBaseline);
I3_SERIALIZABLE(I3IceTopBaselineSeriesMap);


std::ostream& operator<<(std::ostream &stream, const I3IceTopBaseline &bl)
{
  stream << "I3IceTopBaseline for "
	 << (bl.source == I3Waveform::ATWD ? "ATWD" : "FADC");
  if (bl.source == I3Waveform::ATWD) stream << bl.sourceID;
  stream << '\n';
  stream << "  Baseline: " << bl.baseline << '\n';
  stream << "  Slope:    " << bl.slope*I3Units::ns/I3Units::mV << " mV/ns\n";
  stream << "  RMS:      " << bl.rms/I3Units::mV << " mV" << std::endl;

  return stream;
}
