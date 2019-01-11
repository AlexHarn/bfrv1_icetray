/**
    copyright  (C) 2007
    the icecube collaboration

    @date $Date: 2007-07-30 $
    @author juancarlos@icecube.wisc.edu

    @brief I3Module that reads an I3DSTHeader and subsequent I3DST object from
    frame and does one or more fo the following:
        <ul>
        <li>Creates a TDST object with extracted values that can be used by tableio.
        <li>partially recreates original I3Frame objects from which it extracted information.
        </ul>
*/


#ifndef DST_I3DSTEXTRACTOR_16_H_INCLUDED
#define DST_I3DSTEXTRACTOR_16_H_INCLUDED

#include <fstream>
#include <string>
#include <set>
#include <icetray/I3ConditionalModule.h>
#include <phys-services/I3Splitter.h>
#include <icetray/I3TrayHeaders.h>
#include <icetray/I3Logging.h>
#include <icetray/I3Units.h>
#include <dataclasses/I3Direction.h>
#include <dataclasses/I3Position.h>
#include <cmath>
#include <assert.h>
#include <dst/extractor/I3DSTExtractor.h>
#include <recclasses/I3DST16.h>
#include <recclasses/I3DSTHeader16.h>
#include <dst/HealPixCoordinates.h>
#include <dst/extractor/TDST.h>
#include <stdlib.h>
#include "dataclasses/physics/I3TriggerHierarchy.h"
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/zlib.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/device/file.hpp>
#include <deque>

using namespace std;

class I3DSTExtractor16: public I3ConditionalModule, public I3Splitter
{
    /**
     *Constructor
     */
    I3DSTExtractor16();
    I3DSTExtractor16(const I3DSTExtractor16 &);

    /**
     *Assignment operator
     */
    I3DSTExtractor16 &operator=(const I3DSTExtractor16 &);


    /**
      * Add appropiate I3Triggers to I3TriggerHierarchy based on the dst
      * triggertag.
      *
      * @param triggers I3TriggerHierarchy to which trigger will be added
      * @param dst pointer to the dst
      */
    I3MapStringBoolPtr SetTrigger( 
                    I3TriggerHierarchyPtr &triggers, 
                    I3DST16Ptr dst, 
                    I3FramePtr frame);


    /**
     * private data elements
     */


    // HealPixCoordinate object with sky map
    HealPixCoordinate dstcoord_;
    double dtheta0_;
    double dphi0_;
    int coordDigits_;
    I3PositionPtr detectorCenter_;
    I3RandomServicePtr rand_;

    // name of I3DST16 object in frame
    std::string dstName_;
    // name of I3DSTFrame object in frame
    std::string dstHeaderName_;
    // path to output file
    std::string path_;

    // (optional) name of weightmap
    std::string weightMapName_;

    vector<std::string> i3recoList_;

    // boolean that determines if I3DSTExtractor16 should recreate original
    // I3Frame objects
    bool extractToFrame_;
    // boolean that determines if outputfile is of type .dst or .zdst
    
    bool writeDSTFormat_;
    // boolean indicates that header has been written to file
    bool headerWritten_;

    map<unsigned, TriggerKey> triggerkeys_;


    std::string ndom_name_;
    std::string nhit_name_;
    std::string trigger_name_;

    // name to store new eventheader in I3Frame
    std::string eventheader_name_;

    uint64_t startTimeDST_;
    bool     startTimeSet_;
    double   time_;
    double   offsetTime_;      // time to subtract from each event

    // cartesian coordinates of center of detector
    double centerX_;
    double centerY_;
    double centerZ_;
    double zenithHi_;
    double zenithLo_;
    bool cut_data_;
    std::vector<unsigned int> keepTriggers_;

    I3DSTHeader16Ptr dstheader_;

    unsigned int eventCounter_;

    // output iostream
    boost::iostreams::filtering_ostream ofs_;

    map<unsigned, unsigned> trigg_c;
    TDSTPtr tdst;

    // queue of dst events
    std::deque<I3FramePtr> buffer_;

public:

    /**
     *Constructor
     */
    I3DSTExtractor16(const I3Context &ctx);

    /**
     *Destructor
     */
    virtual ~I3DSTExtractor16() { }

    /**
     *Reads the filename, determines the archive type,
     *and opens the file.  Exits if unsuccessful.
     */
    void Configure();

    /**
     *Closes the file.
     */
    void Finish();

    /**
     * Enqueues each frame until it finds an EventHeader then it dequeues each
     * buffered frame and pushes it to the outbox
     * @param frame the current frame to process
     */
    void DAQ(I3FramePtr frame);

    /**
     * Extracts DSTHeader info from EventHeader and writes a DST for each event
     *
     * @param frame the current frame to process
     */
    void ProcessFrame(I3FramePtr frame);

    bool ProcessDST(I3FramePtr frame, I3DST16Ptr dst, int reco_count);

    SET_LOGGER("I3DSTExtractor16");
};

#endif
