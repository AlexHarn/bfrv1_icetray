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

#ifndef TPX_I3ICETOPBASELINE_H_INCLUDED
#define TPX_I3ICETOPBASELINE_H_INCLUDED

#include <dataclasses/physics/I3Waveform.h>
#include <ostream>


static const unsigned i3icetopbaseline_version_ = 0;


/**
 * I3IceTopBaseline stores information about the IceTop-style baseline of a waveform.
 * These parameters are calculated by I3IceTopBaselineModule.
 **/
struct I3IceTopBaseline {
  I3IceTopBaseline();
  I3IceTopBaseline(I3Waveform::Source source_, uint8_t channel_, uint8_t sourceID_);
  I3IceTopBaseline(I3Waveform::Source source_, uint8_t channel_, uint8_t sourceID_,
		   float baseline_, float slope_, float rms_)
    : source(source_), channel(channel_), sourceID(sourceID_),
      baseline(baseline_), slope(slope_), rms(rms_)
  {}

  I3Waveform::Source source;  //!< Waveform source (ATWD or FADC)
  uint8_t channel;            //!< ATWD channel, undefined in case of FADC
  uint8_t sourceID;           //!< ATWD chip ID, undefined in case of FADC

  float baseline;             //!< Average baseline (voltage)
  float slope;                //!< Baseline slope (voltage/time)
  float rms;                  //!< Variation of waveform baseline (voltage)

  // needed by pybindings for vector
  bool operator==(const I3IceTopBaseline &rhs) const {
    return source == rhs.source &&
      channel == rhs.channel &&
      sourceID == rhs.sourceID &&
      baseline == rhs.baseline &&
      slope == rhs.slope &&
      rms == rhs.rms;
  }

private:
  //serialization routine
  friend class icecube::serialization::access;         //provide boost access to my privates
  template <class Archive> void serialize(Archive& ar, unsigned version);

};


std::ostream& operator<<(std::ostream &stream, const I3IceTopBaseline &bl);


I3_CLASS_VERSION(I3IceTopBaseline, i3icetopbaseline_version_);

typedef std::vector<I3IceTopBaseline> I3IceTopBaselineSeries;
typedef I3Map<OMKey, I3IceTopBaselineSeries> I3IceTopBaselineSeriesMap;

I3_POINTER_TYPEDEFS(I3IceTopBaselineSeriesMap);

#endif // TPX_I3ICETOPBASELINE_H_INCLUDED
