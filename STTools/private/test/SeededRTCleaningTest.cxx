/**
 * Copyright (C) 2013
 * Martin Wolf <martin.wolf@icecube.wisc.edu>
 * and the IceCube Collaboration <http://www.icecube.wisc.edu>
 *
 * @version $Id$
 * @file STTools/private/test/SeededRTCleaningTest.cxx
 * @date $Date$
 * @brief This file contains the implementation of the SeededRT hit cleaning
 *        test of STTools.
 *
 *        ----------------------------------------------------------------------
 *        This file is free software; you can redistribute it and/or modify
 *        it under the terms of the GNU General Public License as published by
 *        the Free Software Foundation; either version 3 of the License, or
 *        (at your option) any later version.
 *
 *        This program is distributed in the hope that it will be useful,
 *        but WITHOUT ANY WARRANTY; without even the implied warranty of
 *        MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *        GNU General Public License for more details.
 *
 *        You should have received a copy of the GNU General Public License
 *        along with this program.  If not, see <http://www.gnu.org/licenses/>
 */
#include <I3Test.h>

#include <algorithm>
#include <iostream>

#include <boost/shared_ptr.hpp>
#include <boost/python.hpp>

#include <icetray/init.h>
#include <icetray/I3Tray.h>
#include <icetray/I3Frame.h>
#include <icetray/I3ServiceFactory.h>

#include <dataclasses/geometry/I3Geometry.h>
#include <dataclasses/physics/I3DOMLaunch.h>
#include <dataclasses/physics/I3RecoPulse.h>

#include <interfaces/I3GeometryService.h>

#include <STTools/algorithms/seededRT/I3SeededRTConfiguration.h>
#include <STTools/algorithms/seededRT/I3SeededRTConfigurationService.h>
#include <STTools/algorithms/seededRT/I3PySeededRTConfigurationService.h>

#include "I3EmptyCalibrationService.h"
#include "I3EmptyDetectorStatusService.h"

TEST_GROUP(SeededRTCleaningTest);

namespace bp = boost::python;

bp::object
load_STTools_seededRT_python_module()
{
    I3::init_icetray_lib();
    return bp::import("icecube.STTools.seededRT");
}

I3VectorOMKey GetIceCubeGeometryOMKeys()
{
    I3VectorOMKey doms;
    doms.push_back(OMKey(1,2));  // DOM1
    doms.push_back(OMKey(1,3));  // DOM2
    doms.push_back(OMKey(1,6));  // DOM3
    doms.push_back(OMKey(1,7));  // DOM4
    doms.push_back(OMKey(2,3));  // DOM5
    doms.push_back(OMKey(2,5));  // DOM6
    doms.push_back(OMKey(2,9));  // DOM7
    doms.push_back(OMKey(3,4));  // DOM8
    doms.push_back(OMKey(3,5));  // DOM9
    doms.push_back(OMKey(3,10)); // DOM10
    doms.push_back(OMKey(4,1));  // DOM11
    doms.push_back(OMKey(4,3));  // DOM12
    doms.push_back(OMKey(4,6));  // DOM13
    return doms;
}

I3VectorOMKey GetDeepCoreGeometryOMKeys()
{
    I3VectorOMKey doms;
    doms.push_back(OMKey(81,3)); // DOM14
    doms.push_back(OMKey(81,4)); // DOM15
    return doms;
}

I3VectorOMKey GetGeometryOMKeys()
{
    I3VectorOMKey icdoms = GetIceCubeGeometryOMKeys();
    I3VectorOMKey dcdoms = GetDeepCoreGeometryOMKeys();
    I3VectorOMKey doms;
    doms.resize(icdoms.size()+dcdoms.size());
    std::copy(icdoms.begin(), icdoms.end(), doms.begin());
    std::copy(dcdoms.begin(), dcdoms.end(), doms.begin()+icdoms.size());
    return doms;
}

class I3SeededRTCleaningTestGeometryService : public I3GeometryService
{
    I3GeometryPtr geometry_;

    public:
    I3SeededRTCleaningTestGeometryService()
    {
        log_debug("Creating test geometry.");

        geometry_ = I3GeometryPtr(new I3Geometry());
        geometry_->startTime = I3Time(0,0);
        geometry_->endTime = I3Time(3000,0);

        I3VectorOMKey doms = GetGeometryOMKeys();
        I3OMGeoMap &geo = geometry_->omgeo;
        geo[doms[0]].position = I3Position(0,0,150);
        geo[doms[1]].position = I3Position(0,0,100);
        geo[doms[2]].position = I3Position(0,0,-100);
        geo[doms[3]].position = I3Position(0,0,-150);
        geo[doms[4]].position = I3Position(0,125,100);
        geo[doms[5]].position = I3Position(0,125,0);
        geo[doms[6]].position = I3Position(0,125,-200);
        geo[doms[7]].position = I3Position(0,250,50);
        geo[doms[8]].position = I3Position(0,250,0);
        geo[doms[9]].position = I3Position(0,250,-250);
        geo[doms[10]].position = I3Position(0,375,200);
        geo[doms[11]].position = I3Position(0,375,100);
        geo[doms[12]].position = I3Position(0,375,-50);
        geo[doms[13]].position = I3Position(0,187.5,75);
        geo[doms[14]].position = I3Position(0,187.5,50);
    }

    virtual ~I3SeededRTCleaningTestGeometryService() {}

    I3GeometryConstPtr GetGeometry(I3Time time)
    {
        log_debug("Getting geometry");
        return geometry_;
    }
};

template <class Response>
inline void
SetStartTime(Response &r, double time);

template <>
inline void
SetStartTime(I3DOMLaunch &r, double time)
{
    r.SetStartTime(time);
}

template <>
inline void
SetStartTime(I3RecoPulse &r, double time)
{
    r.SetTime(time);

    // NaN members make I3RecoPulse::operator== always false!
    r.SetCharge(1.0);
    r.SetWidth(1.0);
}

template <class Response>
inline void
SetLCBit(Response &r, bool lc);

template <>
inline void
SetLCBit(I3DOMLaunch &r, bool lc)
{
    r.SetLCBit(lc);
}

template <>
inline void
SetLCBit(I3RecoPulse &r, bool lc)
{
    if (lc)
        r.SetFlags(r.GetFlags() | I3RecoPulse::LC);
    else
        r.SetFlags(r.GetFlags() & ~I3RecoPulse::LC);
}

template <class Response>
class I3SeededRTCleaningTestEventService
{
    bool done_;
    typedef boost::shared_ptr<I3Map<OMKey, std::vector<Response> > > ResponseSeriesMapPtr;

  public:
    I3SeededRTCleaningTestEventService() : done_(false) {}

    virtual ~I3SeededRTCleaningTestEventService() {}

    bool MoreEvents()
    {
        return (done_ ? false : true);
    }

    I3Time PopEvent(I3Frame& frame, bool keep_slc = true)
    {
        ResponseSeriesMapPtr rp(new typename ResponseSeriesMapPtr::element_type);
        Response p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14;
        SetStartTime(p0,   100); SetLCBit(p0,  1); // kept in hlc // kept in COG     // kept in HLCcore
        SetStartTime(p1,   300); SetLCBit(p1,  1); // kept in hlc // kept in COG     // kept in HLCcore
        SetStartTime(p2,  1000); SetLCBit(p2,  1); // kept in hlc // isolated in COG // isolated in HLCcore (spatial)
        SetStartTime(p3,  1900); SetLCBit(p3,  1); // kept in hlc // isolated in COG // isolated in HLCcore (spatial)
        SetStartTime(p4,   500); SetLCBit(p4,  0); // kept in hlc // kept in COG     // kept in HLCcore
        SetStartTime(p5,   750); SetLCBit(p5,  0); // kept in hlc // kept in COG     // kept in HLCcore
        SetStartTime(p6,   250); SetLCBit(p6,  0); // isolated in all 3 cases (spatial)
        SetStartTime(p7,   900); SetLCBit(p7,  1); // kept in hlc // kept in COG     // kept in HLCcore
        SetStartTime(p8,   950); SetLCBit(p8,  1); // kept in hlc // kept in COG     // kept in HLCcore
        SetStartTime(p9,   700); SetLCBit(p9,  0); // isolated in all 3 cases (spatial)
        SetStartTime(p10,  760); SetLCBit(p10, 0); // isolated in all 3 cases (spatial)
        SetStartTime(p11, 2400); SetLCBit(p11, 0); // isolated in all 3 cases (time)
        SetStartTime(p12, 1300); SetLCBit(p12, 0); // kept in hlc // kept in COG     // kept in HLCcore
        SetStartTime(p13,  700); SetLCBit(p13, 1); // kept in hlc // kept in COG     // kept in HLCcore
        SetStartTime(p14,  850); SetLCBit(p14, 1); // kept in hlc // kept in COG     // kept in HLCcore

        I3VectorOMKey doms = GetGeometryOMKeys();

        (*rp)[doms[0]].push_back(p0);
        (*rp)[doms[1]].push_back(p1);
        (*rp)[doms[2]].push_back(p2);
        (*rp)[doms[3]].push_back(p3);
        if (keep_slc) (*rp)[doms[4]].push_back(p4);
        if (keep_slc) (*rp)[doms[5]].push_back(p5);
        if (keep_slc) (*rp)[doms[6]].push_back(p6);
        (*rp)[doms[7]].push_back(p7);
        (*rp)[doms[8]].push_back(p8);
        if (keep_slc) (*rp)[doms[9]].push_back(p9);
        if (keep_slc) (*rp)[doms[10]].push_back(p10);
        if (keep_slc) (*rp)[doms[11]].push_back(p11);
        if (keep_slc) (*rp)[doms[12]].push_back(p12);
        (*rp)[doms[13]].push_back(p13);
        (*rp)[doms[14]].push_back(p14);
        frame.Put("test_launches",rp);

        done_=true;
        return I3Time(2006,0);
    }
};

template <class Response>
class I3SeededRTCleaningTestEventModule
  : public I3Module
  , public I3SeededRTCleaningTestEventService<Response>
{
  public:
    bool keep_slc_;
    I3SeededRTCleaningTestEventModule(const I3Context& ctx)
      : I3Module(ctx)
    {
        AddOutBox("OutBox");
        AddParameter("SLC", "", true);
    }

    void Configure()
    {
        GetParameter("SLC", keep_slc_);
    }

    void Process()
    {
        I3FramePtr frame(new I3Frame(I3Frame::Physics));
        if(this->MoreEvents())
        {
            I3Time t = this->PopEvent(*frame, keep_slc_);
            frame->Put("DrivingTime", I3TimePtr(new I3Time(t)));
            this->PushFrame(frame);
        }
        else
        {
            this->RequestSuspension();
        }
    }
};

I3_MODULE(I3SeededRTCleaningTestEventModule<I3RecoPulse>);
I3_MODULE(I3SeededRTCleaningTestEventModule<I3DOMLaunch>);

class I3SeededRTCleaningTestGCDFactory : public I3ServiceFactory
{
    boost::shared_ptr<I3GeometryService> geometries_;
    boost::shared_ptr<I3CalibrationService> calibrations_;
    boost::shared_ptr<I3DetectorStatusService> status_;

  public:
    I3SeededRTCleaningTestGCDFactory(const I3Context& context)
      : I3ServiceFactory(context)
    {}

    void Configure()
    {
        geometries_   = boost::shared_ptr<I3GeometryService>(new I3SeededRTCleaningTestGeometryService);
        calibrations_ = boost::shared_ptr<I3CalibrationService>(new I3EmptyCalibrationService);
        status_       = boost::shared_ptr<I3DetectorStatusService>(new I3EmptyDetectorStatusService);
    }

    bool InstallService(I3Context& services)
    {
        bool success = true;
        success *= services.Put<I3GeometryService>(geometries_);
        success *= services.Put<I3CalibrationService>(calibrations_);
        success *= services.Put<I3DetectorStatusService>(status_);
        return success;
    }
};

I3_SERVICE_FACTORY(I3SeededRTCleaningTestGCDFactory);

template <class Response>
class I3SeededRTHitCleaningTestClient
  : public I3Module
{
  public:
    std::string mode_;
    std::string input_;
    unsigned nframes_;
    typedef I3Map<OMKey, std::vector<Response> > ResponseSeriesMap;
    I3_POINTER_TYPEDEFS(ResponseSeriesMap);

    I3SeededRTHitCleaningTestClient(const I3Context& context)
      : I3Module(context)
      , nframes_(0)
    {
        AddParameter("Mode", "Mode", mode_);
        AddParameter("Input", "Input", input_);

        AddOutBox("OutBox");
    }

    virtual ~I3SeededRTHitCleaningTestClient() {}

    void Configure()
    {
        GetParameter("Mode", mode_);
        GetParameter("Input", input_);
    }

    void Physics(I3FramePtr frame)
    {
        log_debug("Physics-ing Test");

        const ResponseSeriesMap &launchMap = frame->Get<ResponseSeriesMap>(input_);

        std::vector<OMKey> omnumbers;
        typename ResponseSeriesMap::const_iterator iter;
        for(iter = launchMap.begin() ; iter != launchMap.end() ; iter++)
        {
            omnumbers.push_back(iter->first);
            //std::cout << "OMKey: " << iter->first << std::endl << std::flush;
        }

        I3VectorOMKey doms = GetGeometryOMKeys();

        if(mode_ == "HLC")
        {
            ENSURE(omnumbers.size() == 11, "SeededRTHitCleaningModule (Seed HLC) didn't leave the right number of launches");
            ENSURE(omnumbers[0]  == doms[0],   "1st surviving launch after RTHitCleaningModule should have been on dom1");
            ENSURE(omnumbers[1]  == doms[1],   "2nd surviving launch after RTHitCleaningModule should have been on dom2");
            ENSURE(omnumbers[2]  == doms[2],   "3rd surviving launch after RTHitCleaningModule should have been on dom3");
            ENSURE(omnumbers[3]  == doms[3],   "4th surviving launch after RTHitCleaningModule should have been on dom4");
            ENSURE(omnumbers[4]  == doms[4],   "5th surviving launch after RTHitCleaningModule should have been on dom5");
            ENSURE(omnumbers[5]  == doms[5],   "6th surviving launch after RTHitCleaningModule should have been on dom6");
            ENSURE(omnumbers[6]  == doms[7],   "7th surviving launch after RTHitCleaningModule should have been on dom8");
            ENSURE(omnumbers[7]  == doms[8],   "8th surviving launch after RTHitCleaningModule should have been on dom9");
            ENSURE(omnumbers[8]  == doms[12],  "9th surviving launch after RTHitCleaningModule should have been on dom13");
            ENSURE(omnumbers[9]  == doms[13], "10th surviving launch after RTHitCleaningModule should have been on dom14");
            ENSURE(omnumbers[10] == doms[14], "11th surviving launch after RTHitCleaningModule should have been on dom15");
        }
        else if((mode_ == "HLCcore") || (mode_=="COG"))
        {
            ENSURE(omnumbers.size()==9,"SeededRTHitCleaningModule (Seed HLC) didn't leave the right number of launches");
            ENSURE(omnumbers[0] == doms[0],  "1st surviving launch after RTHitCleaningModule should have been on dom1");
            ENSURE(omnumbers[1] == doms[1],  "2nd surviving launch after RTHitCleaningModule should have been on dom2");
            ENSURE(omnumbers[2] == doms[4],  "3rd surviving launch after RTHitCleaningModule should have been on dom5");
            ENSURE(omnumbers[3] == doms[5],  "4th surviving launch after RTHitCleaningModule should have been on dom6");
            ENSURE(omnumbers[4] == doms[7],  "5th surviving launch after RTHitCleaningModule should have been on dom8");
            ENSURE(omnumbers[5] == doms[8],  "6th surviving launch after RTHitCleaningModule should have been on dom9");
            ENSURE(omnumbers[6] == doms[12], "7th surviving launch after RTHitCleaningModule should have been on dom13");
            ENSURE(omnumbers[7] == doms[13], "8th surviving launch after RTHitCleaningModule should have been on dom14");
            ENSURE(omnumbers[8] == doms[14], "9th surviving launch after RTHitCleaningModule should have been on dom15");
        }
        else if(mode_ == "noSLC")
        {
            ENSURE(omnumbers.size()==8,"SeededRTHitCleaningModule (Seed HLC) didn't leave the right number of launches");
            ENSURE(omnumbers[0] == doms[0],  "1st surviving launch after RTHitCleaningModule should have been on dom1");
            ENSURE(omnumbers[1] == doms[1],  "2nd surviving launch after RTHitCleaningModule should have been on dom2");
            ENSURE(omnumbers[2] == doms[2],  "3rd surviving launch after RTHitCleaningModule should have been on dom3");
            ENSURE(omnumbers[3] == doms[3],  "4th surviving launch after RTHitCleaningModule should have been on dom4");
            ENSURE(omnumbers[4] == doms[7],  "5th surviving launch after RTHitCleaningModule should have been on dom8");
            ENSURE(omnumbers[5] == doms[8],  "6th surviving launch after RTHitCleaningModule should have been on dom9");
            ENSURE(omnumbers[6] == doms[13], "7th surviving launch after RTHitCleaningModule should have been on dom14");
            ENSURE(omnumbers[7] == doms[14], "8th surviving launch after RTHitCleaningModule should have been on dom15");
        }
        else if(mode_ == "cyl")
        {
            ENSURE(omnumbers.size()==14,"SeededRTHitCleaningModule (Seed HLC) didn't leave the right number of launches");
            ENSURE(omnumbers[0]  == doms[0],   "1st surviving launch after RTHitCleaningModule should have been on dom1");
            ENSURE(omnumbers[1]  == doms[1],   "2nd surviving launch after RTHitCleaningModule should have been on dom2");
            ENSURE(omnumbers[2]  == doms[2],   "3rd surviving launch after RTHitCleaningModule should have been on dom3");
            ENSURE(omnumbers[3]  == doms[3],   "4th surviving launch after RTHitCleaningModule should have been on dom4");
            ENSURE(omnumbers[4]  == doms[4],   "5th surviving launch after RTHitCleaningModule should have been on dom5");
            ENSURE(omnumbers[5]  == doms[5],   "6th surviving launch after RTHitCleaningModule should have been on dom6");
            ENSURE(omnumbers[6]  == doms[6],   "7th surviving launch after RTHitCleaningModule should have been on dom7");
            ENSURE(omnumbers[7]  == doms[7],   "8th surviving launch after RTHitCleaningModule should have been on dom8");
            ENSURE(omnumbers[8]  == doms[8],   "9th surviving launch after RTHitCleaningModule should have been on dom9");
            ENSURE(omnumbers[9]  == doms[9],  "10th surviving launch after RTHitCleaningModule should have been on dom10");
            ENSURE(omnumbers[10] == doms[10], "11th surviving launch after RTHitCleaningModule should have been on dom11");
            ENSURE(omnumbers[11] == doms[12], "12th surviving launch after RTHitCleaningModule should have been on dom13");
            ENSURE(omnumbers[12] == doms[13], "13th surviving launch after RTHitCleaningModule should have been on dom14");
            ENSURE(omnumbers[13] == doms[14], "14th surviving launch after RTHitCleaningModule should have been on dom15");
        }
        else if(mode_ == "cylIter")
        {
            ENSURE(omnumbers.size()==13,"SeededRTHitCleaningModule (Seed HLC) didn't leave the right number of launches");
            ENSURE(omnumbers[0]  == doms[0],   "1st surviving launch after RTHitCleaningModule should have been on dom1");
            ENSURE(omnumbers[1]  == doms[1],   "2nd surviving launch after RTHitCleaningModule should have been on dom2");
            ENSURE(omnumbers[2]  == doms[2],   "3rd surviving launch after RTHitCleaningModule should have been on dom3");
            ENSURE(omnumbers[3]  == doms[3],   "4th surviving launch after RTHitCleaningModule should have been on dom4");
            ENSURE(omnumbers[4]  == doms[4],   "5th surviving launch after RTHitCleaningModule should have been on dom5");
            ENSURE(omnumbers[5]  == doms[5],   "6th surviving launch after RTHitCleaningModule should have been on dom6");
            ENSURE(omnumbers[6]  == doms[6],   "7th surviving launch after RTHitCleaningModule should have been on dom7");
            ENSURE(omnumbers[7]  == doms[7],   "8th surviving launch after RTHitCleaningModule should have been on dom8");
            ENSURE(omnumbers[8]  == doms[8],   "9th surviving launch after RTHitCleaningModule should have been on dom9");
            ENSURE(omnumbers[9]  == doms[10], "10th surviving launch after RTHitCleaningModule should have been on dom11");
            ENSURE(omnumbers[10] == doms[12], "11th surviving launch after RTHitCleaningModule should have been on dom13");
            ENSURE(omnumbers[11] == doms[13], "12th surviving launch after RTHitCleaningModule should have been on dom14");
            ENSURE(omnumbers[12] == doms[14], "13th surviving launch after RTHitCleaningModule should have been on dom15");
        }
        else if(mode_ == "DeepCore")
        {
            ENSURE(omnumbers.size()==9,"SeededRTHitCleaningModule (Seed HLC) didn't leave the right number of launches");
            ENSURE(omnumbers[0] == doms[0], "1st surviving launch after RTHitCleaningModule should have been on dom1");
            ENSURE(omnumbers[1] == doms[1], "2nd surviving launch after RTHitCleaningModule should have been on dom2");
            ENSURE(omnumbers[2] == doms[2], "3rd surviving launch after RTHitCleaningModule should have been on dom3");
            ENSURE(omnumbers[3] == doms[3], "4th surviving launch after RTHitCleaningModule should have been on dom4");
            ENSURE(omnumbers[4] == doms[4], "5th surviving launch after RTHitCleaningModule should have been on dom5");
            ENSURE(omnumbers[5] == doms[7], "6th surviving launch after RTHitCleaningModule should have been on dom8");
            ENSURE(omnumbers[6] == doms[8], "7th surviving launch after RTHitCleaningModule should have been on dom9");
            ENSURE(omnumbers[7] == doms[13],"8th surviving launch after RTHitCleaningModule should have been on dom14");
            ENSURE(omnumbers[8] == doms[14],"9th surviving launch after RTHitCleaningModule should have been on dom15");
        }
        else
        {
            log_fatal("Unknown mode %s", mode_.c_str());
        }

        PushFrame(frame);
        nframes_++;
    }

    void Finish()
    {
        ENSURE(nframes_ > 0, "Module actually ran.");
    }
};

I3_MODULE(I3SeededRTHitCleaningTestClient<I3DOMLaunch>);
I3_MODULE(I3SeededRTHitCleaningTestClient<I3RecoPulse>);

boost::shared_ptr<sttools::seededRT::I3PySeededRTConfigurationService>
createI3SeededRTConfigurationService(bool allowSelfCoincidence)
{
    bp::object seededRT = load_STTools_seededRT_python_module();
    bp::object seededRT_ns = seededRT.attr("__dict__");

    boost::shared_ptr<sttools::seededRT::I3PySeededRTConfigurationService> stConfigService;

    try
    {
        // Create an I3SeededRTConfigurationService through the Python
        // interpreter. This is neccessary, because the I3Module parameter
        // requires a Python object.
        if(allowSelfCoincidence)
        {
            bp::object ignored = bp::exec(
                  "stConfigService = I3SeededRTConfigurationService(True, False, float('nan'), float('nan'))"
                , seededRT_ns, seededRT_ns);
            stConfigService = bp::extract< boost::shared_ptr<sttools::seededRT::I3PySeededRTConfigurationService> >(seededRT_ns["stConfigService"]);
        }
        else
        {
            bp::object ignored = bp::exec(
                  "stConfigService = I3SeededRTConfigurationService(False, False, float('nan'), float('nan'))"
                , seededRT_ns, seededRT_ns);
            stConfigService = bp::extract< boost::shared_ptr<sttools::seededRT::I3PySeededRTConfigurationService> >(seededRT_ns["stConfigService"]);
        }
    }
    catch(bp::error_already_set const &)
    {
        PyErr_Print();
        throw;
    }

    return stConfigService;
}

template <class Response>
void
RunTray_Vanilla(const std::string &module_key)
{
    // Setup the ST configuration service.
    std::vector<std::string> strings;
    std::vector<std::string> oms;
    strings.push_back("1-4");
    strings.push_back("81");
    oms.push_back("1-10");
    oms.push_back("1-10");
    I3VectorOMKeyLinkSet omKeyLinkSets;
    omKeyLinkSets.push_back(OMKeyLinkSet(OMKeySet(strings, oms), OMKeySet(strings, oms)));
    I3SeededRTConfiguration stConfig(
        /*name=*/"SphTestSTConfig",
        /*omKeyLinkSets=*/omKeyLinkSets,
        /*rtCoordSys=*/I3SeededRTConfiguration::Sph,
        /*rtTime=*/1000*I3Units::ns,
        /*rtRadius=*/150*I3Units::m,
        /*rtHeight=*/NAN
    );
    I3VectorSeededRTConfigurationPtr stConfigVecPtr(new I3VectorSeededRTConfiguration());
    stConfigVecPtr->push_back(stConfig);

    boost::shared_ptr<sttools::seededRT::I3PySeededRTConfigurationService> stConfigService =
        createI3SeededRTConfigurationService(/*allowSelfCoincidence=*/false);
    stConfigService->SetSTConfigVecPtr(stConfigVecPtr);
    stConfigService->FreezeSTConfiguration();

    I3Tray tray;

    tray.AddService("I3SeededRTCleaningTestGCDFactory", "gcdsource");

    tray.AddModule<I3SeededRTCleaningTestEventModule<Response> >("eventsource")
        ("SLC", true);
    tray.AddModule("I3MetaSynth", "add_geo");

    tray.AddModule(module_key, "isolatedhitscleaningHLC")
        ("InputHitSeriesMapName",  "test_launches")
        ("OutputHitSeriesMapName", "cleaned_launches_HLC")
        ("STConfigService",        stConfigService)
        ("SeedProcedure",          "AllHLCHits")
        ;
// The HLCCOGSTHits seed procedure is not supported anymore.
//     tray.AddModule(module_key, "isolatedhitscleaningCOG")
//         ("InputHitSeriesMapName",  "test_launches")
//         ("OutputHitSeriesMapName", "cleaned_launches_COG")
//         ("STConfigService",        stConfigService)
//         ("SeedProcedure",          "HLCCOGSTHits")
//         ;
    tray.AddModule(module_key, "isolatedhitscleaningHLCcore")
        ("InputHitSeriesMapName",  "test_launches")
        ("OutputHitSeriesMapName", "cleaned_launches_HLCcore")
        ("STConfigService",        stConfigService)
        ("SeedProcedure",          "HLCCoreHits")
        ;

    tray.AddModule<I3SeededRTHitCleaningTestClient<Response> >("clientHLC")
        ("Mode", "HLC")
        ("Input", "cleaned_launches_HLC");
// The HLCCOGSTHits seed procedure is not supported anymore.
//     tray.AddModule<I3SeededRTHitCleaningTestClient<Response> >("clientCOG")
//         ("Mode", "COG")
//         ("Input", "cleaned_launches_COG");

    tray.AddModule<I3SeededRTHitCleaningTestClient<Response> >("clientHLCcore")
        ("Mode", "HLCcore")
        ("Input", "cleaned_launches_HLCcore");

    
    tray.Execute();
    
}

template <class Response>
void
RunTray_NoSLC(const std::string &module_key)
{
    // Setup the ST configuration service.
    std::vector<std::string> strings;
    std::vector<std::string> oms;
    strings.push_back("1-4");
    strings.push_back("81");
    oms.push_back("1-10");
    oms.push_back("1-10");
    I3VectorOMKeyLinkSet omKeyLinkSets;
    omKeyLinkSets.push_back(OMKeyLinkSet(OMKeySet(strings, oms), OMKeySet(strings, oms)));
    I3SeededRTConfiguration stConfig(
        /*name=*/"SphTestSTConfig",
        /*omKeyLinkSets=*/omKeyLinkSets,
        /*rtCoordSys=*/I3SeededRTConfiguration::Sph,
        /*rtTime=*/1000*I3Units::ns,
        /*rtRadius=*/150*I3Units::m,
        /*rtHeight=*/NAN
    );
    I3VectorSeededRTConfigurationPtr stConfigVecPtr(new I3VectorSeededRTConfiguration());
    stConfigVecPtr->push_back(stConfig);

    boost::shared_ptr<sttools::seededRT::I3PySeededRTConfigurationService> stConfigService =
        createI3SeededRTConfigurationService(/*allowSelfCoincidence=*/false);
    stConfigService->SetSTConfigVecPtr(stConfigVecPtr);
    stConfigService->FreezeSTConfiguration();

    I3Tray tray;

    tray.AddService("I3SeededRTCleaningTestGCDFactory", "gcdsource");

    tray.AddModule<I3SeededRTCleaningTestEventModule<Response> >("eventsource")
        ("SLC", false);
    tray.AddModule("I3MetaSynth", "add_geo");

    tray.AddModule(module_key, "isolatedhitscleaningHLC")
        ("InputHitSeriesMapName",  "test_launches")
        ("OutputHitSeriesMapName", "cleaned_launches_HLC")
        ("STConfigService",        stConfigService)
        ("SeedProcedure",          "AllHLCHits")
        ;

    tray.AddModule<I3SeededRTHitCleaningTestClient<Response> >("clientnoSLC")
        ("Mode", "noSLC")
        ("Input", "cleaned_launches_HLC");

    
    tray.Execute();
    
}

template <class Response>
void
RunTray_CylinderIterative(const std::string &module_key)
{
    // Setup the ST configuration service.
    std::vector<std::string> strings;
    std::vector<std::string> oms;
    strings.push_back("1-4");
    strings.push_back("81");
    oms.push_back("1-10");
    oms.push_back("1-10");
    I3VectorOMKeyLinkSet omKeyLinkSets;
    omKeyLinkSets.push_back(OMKeyLinkSet(OMKeySet(strings, oms), OMKeySet(strings, oms)));
    I3SeededRTConfiguration stConfig(
        /*name=*/"CylTestSTConfig",
        /*omKeyLinkSets=*/omKeyLinkSets,
        /*rtCoordSys=*/I3SeededRTConfiguration::Cyl,
        /*rtTime=*/1000*I3Units::ns,
        /*rtRadius=*/150*I3Units::m,
        /*rtHeight=*/150*I3Units::m
    );
    I3VectorSeededRTConfigurationPtr stConfigVecPtr(new I3VectorSeededRTConfiguration());
    stConfigVecPtr->push_back(stConfig);

    boost::shared_ptr<sttools::seededRT::I3PySeededRTConfigurationService> stConfigService =
        createI3SeededRTConfigurationService(/*allowSelfCoincidence=*/false);
    stConfigService->SetSTConfigVecPtr(stConfigVecPtr);
    stConfigService->FreezeSTConfiguration();

    I3Tray tray;

    tray.AddService("I3SeededRTCleaningTestGCDFactory", "gcdsource");

    tray.AddModule<I3SeededRTCleaningTestEventModule<Response> >("eventsource")
        ("SLC", true);
    tray.AddModule("I3MetaSynth", "add_geo");

    tray.AddModule(module_key, "isolatedhitscleaningCyl")
        ("InputHitSeriesMapName",  "test_launches")
        ("OutputHitSeriesMapName", "cleaned_launches_cyl")
        ("STConfigService",        stConfigService)
        ("SeedProcedure",          "AllHLCHits")
        ;

    tray.AddModule(module_key, "isolatedhitscleaningCylIter")
        ("InputHitSeriesMapName",  "test_launches")
        ("OutputHitSeriesMapName", "cleaned_launches_cylit")
        ("STConfigService",        stConfigService)
        ("SeedProcedure",          "AllHLCHits")
        ("MaxNIterations",         1)
        ;

    tray.AddModule<I3SeededRTHitCleaningTestClient<Response> >("clientCyl")
        ("Mode",  "cyl")
        ("Input", "cleaned_launches_cyl");

    tray.AddModule<I3SeededRTHitCleaningTestClient<Response> >("clientCyliter")
        ("Mode",  "cylIter")
        ("Input", "cleaned_launches_cylit");

    
    tray.Execute();
    
}

template <class Response>
void
RunTray_DeepCore(const std::string &module_key)
{
    // Setup the ST configuration service.
    std::vector<std::string> ic_strings;
    std::vector<std::string> ic_oms;
    std::vector<std::string> dc_strings;
    std::vector<std::string> dc_oms;
    ic_strings.push_back("1-4");
    ic_oms.push_back("1-10");
    dc_strings.push_back("81");
    dc_oms.push_back("3-4");
    I3VectorOMKeyLinkSet omKeyLinkSets1;
    omKeyLinkSets1.push_back(OMKeyLinkSet(OMKeySet(ic_strings, ic_oms), OMKeySet(ic_strings, ic_oms)));
    I3VectorOMKeyLinkSet omKeyLinkSets2;
    omKeyLinkSets2.push_back(OMKeyLinkSet(OMKeySet(dc_strings, dc_oms), OMKeySet(dc_strings, dc_oms)));
    omKeyLinkSets2.push_back(OMKeyLinkSet(OMKeySet(ic_strings, ic_oms), OMKeySet(dc_strings, dc_oms)));
    I3SeededRTConfiguration icecubeSTConfig(
        /*name=*/"SphIceCubeTestSTConfig",
        /*omKeyLinkSets=*/omKeyLinkSets1,
        /*rtCoordSys=*/I3SeededRTConfiguration::Sph,
        /*rtTime=*/1000*I3Units::ns,
        /*rtRadius=*/90*I3Units::m,
        /*rtHeight=*/NAN
    );
    I3SeededRTConfiguration deepcoreSTConfig(
        /*name=*/"SphDeepCoreTestSTConfig",
        /*omKeyLinkSets=*/omKeyLinkSets2,
        /*rtCoordSys=*/I3SeededRTConfiguration::Sph,
        /*rtTime=*/500*I3Units::ns,
        /*rtRadius=*/75*I3Units::m,
        /*rtHeight=*/NAN
    );
    I3VectorSeededRTConfigurationPtr stConfigVecPtr(new I3VectorSeededRTConfiguration());
    stConfigVecPtr->push_back(icecubeSTConfig);
    stConfigVecPtr->push_back(deepcoreSTConfig);

    boost::shared_ptr<sttools::seededRT::I3PySeededRTConfigurationService> stConfigService =
        createI3SeededRTConfigurationService(/*allowSelfCoincidence=*/false);
    stConfigService->SetSTConfigVecPtr(stConfigVecPtr);
    stConfigService->FreezeSTConfiguration();

    I3Tray tray;

    tray.AddService("I3SeededRTCleaningTestGCDFactory", "gcdsource");

    tray.AddModule<I3SeededRTCleaningTestEventModule<Response> >("eventsource")
        ("SLC", true);
    tray.AddModule("I3MetaSynth", "add_geo");

    tray.AddModule(module_key, "isolatedhitscleaningDC")
        ("InputHitSeriesMapName",  "test_launches")
        ("OutputHitSeriesMapName", "cleaned_launches_DC")
        ("STConfigService",        stConfigService)
        ("SeedProcedure",          "AllHLCHits")
        ;

    tray.AddModule<I3SeededRTHitCleaningTestClient<Response> >("clientDC")
        ("Mode",  "DeepCore")
        ("Input", "cleaned_launches_DC");

    
    tray.Execute();
    
}

TEST(I3SeededRTCleaningModuleTest_I3DOMLaunch)
{
    RunTray_Vanilla<I3DOMLaunch>("I3SeededRTCleaning_DOMLaunch_Module");
}

TEST(I3SeededRTCleaningModuleTest_I3RecoPulse)
{
    RunTray_Vanilla<I3RecoPulse>("I3SeededRTCleaning_RecoPulse_Module");
}

TEST(I3SeededRTCleaningModuleNoSLCTest_I3DOMLaunch)
{
    RunTray_NoSLC<I3DOMLaunch>("I3SeededRTCleaning_DOMLaunch_Module");
}

TEST(I3SeededRTCleaningModuleNoSLCTest_I3RecoPulse)
{
    RunTray_NoSLC<I3RecoPulse>("I3SeededRTCleaning_RecoPulse_Module");
}

TEST(I3SeededRTCleaningModuleCylinderIterativeTest_I3DOMLaunch)
{
    RunTray_CylinderIterative<I3DOMLaunch>("I3SeededRTCleaning_DOMLaunch_Module");
}

TEST(I3SeededRTCleaningModuleCylinderIterativeTest_I3RecoPulse)
{
    RunTray_CylinderIterative<I3RecoPulse>("I3SeededRTCleaning_RecoPulse_Module");
}

TEST(I3SeededRTCleaningModuleDeepCoreTest_I3DOMLaunch)
{
    RunTray_DeepCore<I3DOMLaunch>("I3SeededRTCleaning_DOMLaunch_Module");
}

TEST(I3SeededRTHitCleaningModuleDeepCoreTest_I3RecoPulse)
{
    RunTray_DeepCore<I3RecoPulse>("I3SeededRTCleaning_RecoPulse_Module");
}
