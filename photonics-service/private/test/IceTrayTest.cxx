/**
 *@file
 *@brief Test photonics-service in icetray framework 
 *
 *@author Kevin Meagher
 *@version $Id$
 *@date $Date$
 *(c) the IceCube Collaboration
 */
#include <I3Test.h>
#include "boost/filesystem.hpp"

#include "icetray/test/ConstructorTest.h"
#include "icetray/I3Tray.h"
#include "photonics-service/I3PhotonicsServiceFactory.h"


TEST_GROUP(IceTrayTests);

TEST(icetray_inspect){
 clean_constructor_test<I3PhotonicsServiceFactory>();
}

namespace bff = boost::filesystem;

TEST(LoadPhotorecTables)
{
  I3Tray tray;

  std::string I3_TESTDATA;
  if(getenv("I3_TESTDATA") != NULL) {
    I3_TESTDATA = std::string(getenv("I3_TESTDATA"));
  }else{
    ENSURE( false, "I3_TESTDATA is defined!");
  }

  std::string tables =  I3_TESTDATA + "/photonics-tables/";
  ENSURE( bff::exists(bff::path(tables)), "tables directory do not exist.");

  tray.AddService("I3PhotonicsServiceFactory", "PhotonicsService")
    ("PhotonicsTopLevelDirectory",tables)
    ("DriverFileDirectory",tables)
    ("PhotonicsLevel2DriverFile","I3CS_z40a20_level2_muon_gauss_degenerate_photorec.list")
    ("PhotonicsTableSelection",2)
    ;

  tray.AddModule("I3InfiniteSource","streams");
  

  tray.Execute(1);
  
}

TEST(LoadDummyTables)
{
  I3Tray tray;

  tray.AddService("I3PhotonicsServiceFactory", "PhotonicsService")
    ("UseDummyService",true)
    ;

  tray.AddModule("I3InfiniteSource","streams");
  

  tray.Execute(1);
  
}
