#include "I3Test.h"

#include "finiteReco/probability/PhPnhProbBase.h"
#include "finiteReco/probability/PhPnhMCDist.h"
#include "finiteReco/probability/PhPnhParam.h"
#include "finiteReco/probability/PhPnhPhotorec.h"
#include "finiteReco/probability/PhPnhPhotorecCone.h"
#include "dataclasses/physics/I3Particle.h"
#include "dataclasses/I3Position.h"
#include "photonics-service/I3PhotonicsTableService.h"

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>

TEST_GROUP(finiteRecoProbabilityTest);

static void DoTest(PhPnhProbBase* probCalc){  
  I3Position omPos1(0,20,100);
  I3Particle track;
  track.SetPos(0,0,0);
  track.SetDir(0,0,1);
  track.SetShape(I3Particle::ContainedTrack);
  track.SetLength(200);
  double prob1 = probCalc->GetHitProb(track,omPos1);
  ENSURE(prob1 >= 0,"Probabilities must be greater equal 0");
  ENSURE(prob1 <= 1,"Probabilities must be smaller equal 1");
  
  double prob =  1;
  for(int i = 2;i<40;i++){
    I3Position omPos(0,10*i,200);
    double prob2 = probCalc->GetHitProb(track,omPos);
    ENSURE(prob2 <= prob,"Probability should decrease with distance from track");
    prob = prob2;
  }
  
  prob =  1;
  for(int i = 2;i<40;i++){
    I3Position omPos(0,50,200 + i*10);
    double prob2 = probCalc->GetHitProb(track,omPos);
    ENSURE(prob2 <= prob,"Probability should decrease behind the end of the track");
    prob = prob2;
  }  
  
  prob =  1;
  for(int i = 2;i<40;i++){
    I3Position omPos(0,50,-i*10);
    double prob2 = probCalc->GetHitProb(track,omPos);
    ENSURE(prob2 <= prob,"Probability should decrease before the start of the track");
    prob = prob2;
  }
  
  I3Position omPos2(0,20,300);
  ENSURE(probCalc->GetHitProb(track,omPos1) > probCalc->GetHitProb(track,omPos2),"The probability before the end is lower then behind");
}


TEST(PhPnhParam)
{
  PhPnhProbBase* probCalc = new PhPnhParam();
  DoTest(probCalc);
}

TEST(PhPnhMCDist)
{
  char* workdirPtr = getenv("I3_SRC");
  ENSURE(workdirPtr, "I3_SRC needs to be set correctly for the unit tests to work");
  string workdir(workdirPtr);
  
  PhPnhProbBase* probCalc = new PhPnhMCDist((workdir+"/finiteReco/resources/table/probVSchdist_LogE1-8.lst").c_str());
  DoTest(probCalc);
}

TEST(PhPnhPhotorec)
{
  char* path  = getenv("PHOTONICSPATH");
  if(path!=NULL){ 
    string pathString(path);
    log_info("Found photonics at: %s",pathString.c_str());
    string driveFile = "tables/photorec_listfiles_AHA07v2h2/I3Coord_I3Span_z80_a60/level2_muon.list";
    if(boost::filesystem::exists((pathString+driveFile).c_str())){
      log_info("Using DriverFile: %s",driveFile.c_str());
      
      I3PhotonicsTableServicePtr photonics(new I3PhotonicsTableService());
      boost::filesystem::path cwd = boost::filesystem::current_path();
      ENSURE_EQUAL(chdir(pathString.c_str()), 0, "Can't access photonics path!");
      photonics->LoadTables(2,pathString+driveFile,pathString);
      ENSURE_EQUAL(chdir(cwd.string().c_str()), 0, "Can't go back to former current path!"); // mainly to soothe the compiler
      
      PhPnhProbBase* probCalc = new PhPnhPhotorec(photonics);
      
      DoTest(probCalc);
    } else{
      log_error("DriverFile: %s",driveFile.c_str());
    }
  } else{
    log_error("Photonics not found! PhPnhPhotorec will not work");
  }
}

TEST(PhPnhPhotorecCone)
{
  char* path  = getenv("PHOTONICSPATH");
  if(path!=NULL){ 
    string pathString(path);
    log_info("Found photonics at: %s",pathString.c_str());
    if(boost::filesystem::exists((pathString+"driverFileDir/listfiles_AHA07v1ice/I3Coord_I3Span_z80_a60/level2_muon_gauss_photorec.list").c_str())){
      log_info("Using DriverFile: listfiles_AHA07v1ice/I3Coord_I3Span_z80_a60/level2_muon_gauss_photorec.list");
      
      I3PhotonicsTableServicePtr photonics(new I3PhotonicsTableService());
      boost::filesystem::path cwd = boost::filesystem::current_path();
      ENSURE_EQUAL(chdir(pathString.c_str()), 0, "Can't access photonics path!");
      photonics->LoadTables(2,pathString+"driverFileDir/listfiles_AHA07v1ice/I3Coord_I3Span_z80_a60/level2_muon_gauss_photorec.list",pathString);
      ENSURE_EQUAL(chdir(cwd.string().c_str()), 0, "Can't go back to former current path!"); // mainly to soothe the compiler
      
      PhPnhProbBase* probCalc = new PhPnhPhotorecCone(photonics);
      
      DoTest(probCalc);
    } else{
      log_error("DriverFile: 'listfiles_AHA07v1ice/I3Coord_I3Span_z80_a60/level2_muon_gauss_photorec.list' is not found.");
    }
  } else{
    log_error("Photonics not found! PhPnhPhotorec will not work");
  }
}
