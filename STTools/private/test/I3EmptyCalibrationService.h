#ifndef I3EMPTYCALIBRATIONSERVICE_H
#define I3EMPTYCALIBRATIONSERVICE_H

#include <interfaces/I3CalibrationService.h>
#include <dataclasses/calibration/I3Calibration.h>

class I3EmptyCalibrationService : public I3CalibrationService
{
 public:
  I3EmptyCalibrationService(){
    calibration_ = I3CalibrationPtr(new I3Calibration());
    calibration_->startTime = I3Time(0,0);
    calibration_->endTime = I3Time(3000,0);    
  }

  virtual ~I3EmptyCalibrationService(){};
  
  I3CalibrationConstPtr GetCalibration(I3Time time){
    return calibration_;
  }
  
 private:

  I3CalibrationPtr calibration_;
};

#endif
