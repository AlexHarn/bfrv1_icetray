#ifndef FILLRATIO_I3FILLRATIO2STRINGDOUBLEMAP_H_INCLUDED
#define FILLRATIO_I3FILLRATIO2STRINGDOUBLEMAP_H_INCLUDED

#include "icetray/I3Module.h"
#include "icetray/I3Context.h"
#include "icetray/I3Frame.h"
#include "icetray/I3Logging.h"

class I3FillRatio2StringDoubleMap : public I3Module
{
 public:
  
  I3FillRatio2StringDoubleMap(const I3Context& context);
  ~I3FillRatio2StringDoubleMap();
  void Physics(I3FramePtr frame);
  void Configure();

 private:

  I3FillRatio2StringDoubleMap();
  I3FillRatio2StringDoubleMap(const I3FillRatio2StringDoubleMap&);
  I3FillRatio2StringDoubleMap& operator=(const I3FillRatio2StringDoubleMap&);

  std::string fillratio_output_name_;
  std::string fillratiobox_;

  double  meanDistance_;
  double  rmsDistance_;
  double  nchDistance_;
  double  energyDistance_;

  double  fillRadius_;
  double  fillRadiusFromMean_;
  double  fillRadiusFromEnergy_;
  double  fillRadiusFromNCh_;
  double  fillRadiusFromMeanPlusRMS_;

  double  fillRatio_;
  double  fillRatioFromMean_;
  double  fillRatioFromMeanPlusRMS_;
  double  fillRatioFromNCh_;
  double  fillRatioFromEnergy_;

  double  hitCount_;
  
  SET_LOGGER("I3FillRatio2StringDoubleMap");
};
#endif // FILLRATIO_I3FILLRATIO2STRINGDOUBLEMAP_H_INCLUDED
