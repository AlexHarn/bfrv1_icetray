#ifndef I3EMPTYDETECTORSTATUSSERVICE_H
#define I3EMPTYDETECTORSTATUSSERVICE_H

#include <interfaces/I3DetectorStatusService.h>
#include <dataclasses/status/I3DetectorStatus.h>

class I3EmptyDetectorStatusService : public I3DetectorStatusService
{
 public:
  I3EmptyDetectorStatusService(){
    status_ = I3DetectorStatusPtr(new I3DetectorStatus());
    status_->startTime = I3Time(0,0);
    status_->endTime = I3Time(3000,0);
  };
  
  virtual ~I3EmptyDetectorStatusService(){};

  I3DetectorStatusConstPtr GetDetectorStatus(I3Time time){
    return status_;
  }
  
 private:

  I3DetectorStatusPtr status_;
};

#endif
