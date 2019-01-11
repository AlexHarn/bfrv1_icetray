#ifndef DIPLOPIA_COINCIDENT_I3READER_SERVICE_H_INLCUDED
#define DIPLOPIA_COINCIDENT_I3READER_SERVICE_H_INLCUDED

#include <icetray/I3TrayHeaders.h>
#include <icetray/I3Logging.h>
#include <sim-services/I3GeneratorService.h>
#include <polyplopia/PolyplopiaUtils.h>
#include <dataclasses/physics/I3MCTree.h>
#include <dataio/I3FrameSequence.h>
#include <dataio/I3FileStager.h>

#include <icetray/I3Frame.h>
#include <icetray/open.h>

/**
 * copyright  (C) 2006
 * The IceCube Collaboration
 *
 * @date $Date:$
 * @author Juan Carlos Diaz Velez
 *                                                                       
 * @brief Utilities for merging MCTrees MCInfo and MCHitSeriesMaps in separate 
 *	events to produce coincident in-ice events.
 *
 */

class CoincidentI3ReaderService: public I3GeneratorService
{
  public:
  
    CoincidentI3ReaderService(const I3Context& ctx);
    CoincidentI3ReaderService(const CoincidentI3ReaderService&);
    CoincidentI3ReaderService(const std::string& filename);
    ~CoincidentI3ReaderService();

    virtual I3MCTreePtr GetNextEvent() override;
    virtual I3FramePtr GetNextFrame() override;

    virtual double GetRate() override;

    void Configure() override;

    bool Open(std::string path);

  private:
	
    int Init();
  
    dataio::I3FrameSequence reader_;
    std::string path_;
    I3::dataio::shared_filehandle file_ref_;
    double rate_;
    bool configured_;
    boost::shared_ptr<I3FileStager> stager_;
};

I3_POINTER_TYPEDEFS(CoincidentI3ReaderService);

#endif //DIPLOPIA_COINCIDENT_I3READER_SERVICE_H_INLCUDED
