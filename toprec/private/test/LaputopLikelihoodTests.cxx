/**
    copyright  (C) 2004
    the icecube collaboration
    $Id: I3CalculatorTest.cxx 9161 2005-06-14 16:44:58Z pretz $

    @version $Revision: 1.2 $
    @date $Date: 2005-06-14 12:44:58 -0400 (Tue, 14 Jun 2005) $
    @author dule

    @todo
*/

#include <I3Test.h>
#include <stdio.h>

//#include <I3Db/I3DbCalibrationService.h>
//#include <I3Db/I3DbDetectorStatusService.h>
//#include <I3Db/I3DbGeometryService.h>
//#include <I3Db/I3DbOMKey2MBID.h>

#include "interfaces/I3GeometryService.h"
#include "interfaces/I3CalibrationService.h"
#include "interfaces/I3DetectorStatusService.h"

#include "dataclasses/geometry/I3Geometry.h"
#include "dataclasses/calibration/I3Calibration.h"
#include "dataclasses/status/I3DetectorStatus.h"
#include "phys-services/source/I3GCDFileService.h"

#include "toprec/I3LaputopLikelihood.h"

// Let's create a hypothetical pulseseries!
// I stole these from an IT73 event, from my little testfile "Level2a_Laputop_IT73_0718_116195.i3"
// The pulseseries IceTopVEMPulses_0 in the first event.
// It contains 18 pulses.
I3RecoPulseSeriesMapPtr testPSM_tweaked() {
  I3RecoPulseSeriesMapPtr psm(new I3RecoPulseSeriesMap);
  OMKey key;
  I3RecoPulse pulse;
  I3RecoPulseSeries ps;
  key.SetString(43);  key.SetOM(61);
  pulse.SetTime(10572.57);  pulse.SetCharge(3.0177093);  pulse.SetWidth(153.87358);  pulse.SetFlags(3);
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  key.SetString(43);  key.SetOM(63);
  pulse.SetTime(10611.964);  pulse.SetCharge(0.31476855);  pulse.SetWidth(128.18964);  pulse.SetFlags(3);
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  key.SetString(52);  key.SetOM(61);
  pulse.SetTime(10569.023);  pulse.SetCharge(1.2578508);  pulse.SetWidth(222.78705);  pulse.SetFlags(3);
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  key.SetString(52);  key.SetOM(63);
  pulse.SetTime(10589.027);  pulse.SetCharge(0.45404792);  pulse.SetWidth(133.80554);  pulse.SetFlags(3);
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  key.SetString(53);  key.SetOM(61);
  pulse.SetTime(10326.345);  pulse.SetCharge(2.4605699);  pulse.SetWidth(153.76886);  pulse.SetFlags(3);
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  key.SetString(53);  key.SetOM(63);
  pulse.SetTime(10342.751);  pulse.SetCharge(1.076555);  pulse.SetWidth(155.85727);  pulse.SetFlags(3);
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  key.SetString(54);  key.SetOM(61);
  pulse.SetTime(10197.892);  pulse.SetCharge(0.1957562);  pulse.SetWidth(156.05147);  pulse.SetFlags(3);
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  key.SetString(54);  key.SetOM(63);
  pulse.SetTime(10191.37);  pulse.SetCharge(0.58932847);  pulse.SetWidth(134.95946);  pulse.SetFlags(3);
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  key.SetString(61);  key.SetOM(61);
  pulse.SetTime(10305.588);  pulse.SetCharge(6.5178876);  pulse.SetWidth(132.24896);  pulse.SetFlags(3);
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  key.SetString(61);  key.SetOM(63);
  pulse.SetTime(10319.602);  pulse.SetCharge(2.2484291);  pulse.SetWidth(152.80693);  pulse.SetFlags(3);
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  key.SetString(62);  key.SetOM(61);
  pulse.SetTime(10143.537);  pulse.SetCharge(27.503639);  pulse.SetWidth(142.91077);  pulse.SetFlags(3);
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  key.SetString(62);  key.SetOM(64);   // This one is high-gain...
  // Make some alterations!
  // Make this LG one saturated (it's the only LG one)
  //pulse.SetTime(10145.445);  pulse.SetCharge(24.468243);  pulse.SetWidth(139.28784);  pulse.SetFlags(3);
  pulse.SetTime(10145.445);  pulse.SetCharge(100000);  pulse.SetWidth(139.28784);  pulse.SetFlags(3);
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  key.SetString(63);  key.SetOM(61);
  pulse.SetTime(9872.1543);  pulse.SetCharge(0.84486663);  pulse.SetWidth(129.31296);  pulse.SetFlags(3);
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  key.SetString(63);  key.SetOM(63);
  pulse.SetTime(9899.5029);  pulse.SetCharge(0.37883031);  pulse.SetWidth(166.07344);  pulse.SetFlags(3);
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  key.SetString(69);  key.SetOM(61);
  pulse.SetTime(10129.423);  pulse.SetCharge(0.35165673);  pulse.SetWidth(219.39558);  pulse.SetFlags(3);
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  key.SetString(69);  key.SetOM(63);
  pulse.SetTime(10134.228);  pulse.SetCharge(0.42961508);  pulse.SetWidth(178.56526);  pulse.SetFlags(3);
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  key.SetString(75);  key.SetOM(61);
  // Alter this one for testing... a NAN charge on a HG because there's no LG for whatever reason
  //pulse.SetTime(10126.95);  pulse.SetCharge(0.69002074);  pulse.SetWidth(130.26392);  pulse.SetFlags(3);
  pulse.SetTime(10126.95);  pulse.SetCharge(NAN);  pulse.SetWidth(130.26392);  pulse.SetFlags(3);
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  key.SetString(75);  key.SetOM(63);
  pulse.SetTime(10110.982);  pulse.SetCharge(0.62996495);  pulse.SetWidth(156.70247);  pulse.SetFlags(3);
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  return psm;
}


TEST_GROUP(FillInput);

TEST(FakeFill)
{
  printf("Init! \n");
  std::string gcd(getenv("I3_TESTDATA"));
  gcd = gcd+"/sim/GeoCalibDetectorStatus_IC79.55380_corrected.i3.gz";
  // Create a fake frame, and put some stuff in it
  I3FramePtr frame(new I3Frame(I3Frame::Physics));
  I3Time time1(2010,171195332233603703ULL ); // first event in my test file
  // G
  I3GeometryServicePtr geoservice(new I3GCDFileGeometryService(gcd));
  assert (geoservice);
  I3GeometryConstPtr geometry1 = geoservice->GetGeometry(time1);
  assert (geometry1);
  frame->Put("I3Geometry", geometry1, I3Frame::Geometry);
  // C
  I3CalibrationServicePtr calibservice(new I3GCDFileCalibrationService(gcd));
  assert (calibservice);
  I3CalibrationConstPtr calibration1 = calibservice->GetCalibration(time1);
  assert (calibration1);
  frame->Put("I3Calibration", calibration1, I3Frame::Calibration);
  // D
  I3DetectorStatusServicePtr detstatservice(new I3GCDFileDetectorStatusService(gcd));
  assert (detstatservice);
  I3DetectorStatusConstPtr detstat1 = detstatservice->GetDetectorStatus(time1);
  assert (detstat1);
  frame->Put("I3DetectorStatus", detstat1, I3Frame::DetectorStatus);

  // Fake pulses
  printf("About to call.. \n");
  I3RecoPulseSeriesMapConstPtr psmptr = testPSM_tweaked();
  ENSURE(psmptr->size() == 18);

  //I3RecoPulse test = (psmptr->find(OMKey(62, 64))->second).at(0);
  //I3RecoPulse test2 = test.at(0);
  //test.SetCharge(100000);
  //(psmptr->find(OMKey(62, 64))->second).at(0).SetCharge(100000);

  frame->Put("FakePulseSeriesMap",psmptr);

  
  
  // Create the service
  I3LaputopLikelihood lservice("Laputop");
  lservice.SetReadoutName("FakePulseSeriesMap");
  // Call the function
  int nstation = lservice.FillInput(*frame);
  ENSURE(nstation == 9);
  std::vector<tankPulse> pnorm = lservice.GetInputData();
  std::vector<tankPulse> pempty = lservice.GetInputEmptyData();
  std::vector<tankPulse> psat = lservice.GetInputSaturatedData();

  printf("pnorm %zu\n", pnorm.size());
  printf("pempty %zu\n", pempty.size());
  printf("psat %zu\n", psat.size());

  ENSURE(pnorm.size() == 16);  // tanks
  ENSURE(pempty.size() == 63);  // stations  -- adds up to 72 because 39-61 is bad
  ENSURE(psat.size() == 2);

  // Test some particulars
  //---------------------------

  // A regular pulse:
  tankPulse regularpulse = pnorm[0];
  ENSURE(regularpulse.omkey.GetString() == 43);
  ENSURE(regularpulse.omkey.GetOM() == 61);
  ENSURE_DISTANCE(regularpulse.x, -260.01, 0.001);
  ENSURE_DISTANCE(regularpulse.y, 38.74, 0.001);
  ENSURE_DISTANCE(regularpulse.z, 1945.16, 0.001);
  ENSURE_DISTANCE(regularpulse.t, 10572.57, 0.001);
  ENSURE_DISTANCE(regularpulse.width, 153.87358, 0.000001);
  ENSURE_DISTANCE(regularpulse.logvem, log10(3.0177093), 0.00000001);
  //ENSURE_DISTANCE(regularpulse.snowdepth, 0.22458, 0.000001);
  ENSURE_DISTANCE(regularpulse.snowdepth, 0.130, 0.000001);
  ENSURE(regularpulse.usepulsetime);  
  
  // A not-hit pulse:
  tankPulse nopulse = pempty[0];
  ENSURE(nopulse.omkey.GetString() == 2);
  ENSURE(nopulse.omkey.GetOM() == 61);
  ENSURE_DISTANCE(nopulse.x, -135.4975, 0.001);  // Average XYZ of two tanks
  ENSURE_DISTANCE(nopulse.y, -477.170, 0.001);
  ENSURE_DISTANCE(nopulse.z, 1945.230, 0.001);
  ENSURE(nopulse.t != nopulse.t);
  ENSURE(nopulse.width != nopulse.width);
  ENSURE(nopulse.logvem != nopulse.logvem);
  ENSURE_DISTANCE(nopulse.snowdepth, 0, 0.000001);
  ENSURE(!nopulse.usepulsetime);

  // The two saturated pulses
  tankPulse satpulse1 = psat[0];
  ENSURE(satpulse1.omkey.GetString() == 62);
  ENSURE(satpulse1.omkey.GetOM() == 64);
  ENSURE_DISTANCE(satpulse1.x, -209.01, 0.001);
  ENSURE_DISTANCE(satpulse1.y, 241.43, 0.001);
  ENSURE_DISTANCE(satpulse1.z, 1945.99, 0.001); 
  ENSURE_DISTANCE(satpulse1.t, 10145.445, 0.001);
  ENSURE_DISTANCE(satpulse1.width, 139.28784, 0.00001);
  ENSURE_DISTANCE(satpulse1.logvem, 2.96, 0.001);  // --- reset to max LG saturation
  ENSURE_DISTANCE(satpulse1.snowdepth, 0.26, 0.001);
  ENSURE(satpulse1.usepulsetime);
  tankPulse satpulse2 = psat[1];
  ENSURE(satpulse2.omkey.GetString() == 75);
  ENSURE(satpulse2.omkey.GetOM() == 61);
  ENSURE_DISTANCE(satpulse2.x, -352.69, 0.001);
  ENSURE_DISTANCE(satpulse2.y, 421.15, 0.001);
  ENSURE_DISTANCE(satpulse2.z, 1946.01, 0.001);
  ENSURE_DISTANCE(satpulse2.t, 10126.95, 0.001);
  ENSURE_DISTANCE(satpulse2.width, 130.26392, 0.00001);
  ENSURE_DISTANCE(satpulse2.logvem, 1.266, 0.001);  // --- reset to max HG saturation
  ENSURE_DISTANCE(satpulse2.snowdepth, 0.30, 0.001);
  ENSURE(satpulse2.usepulsetime);

}

TEST(BadStationBadTank)
{
  // This is another test of the "FillInput" function.  Does it successfully remove stations when one
  // of the two tanks is in a "Bad Tank List" and the other is not hit?
  printf("Init Bad Station/Tank test! \n");
  std::string gcd(getenv("I3_TESTDATA"));
  gcd = gcd+"/sim/GeoCalibDetectorStatus_IC79.55380_corrected.i3.gz";
  // Create a fake frame, and put some stuff in it
  I3FramePtr frame(new I3Frame(I3Frame::Physics));
  I3Time time1(2010,171195332233603703ULL ); // first event in my test file
  // G
  I3GeometryServicePtr geoservice(new I3GCDFileGeometryService(gcd));
  assert (geoservice);
  I3GeometryConstPtr geometry1 = geoservice->GetGeometry(time1);
  assert (geometry1);
  frame->Put("I3Geometry", geometry1, I3Frame::Geometry);
  // C
  I3CalibrationServicePtr calibservice(new I3GCDFileCalibrationService(gcd));
  assert (calibservice);
  I3CalibrationConstPtr calibration1 = calibservice->GetCalibration(time1);
  assert (calibration1);
  frame->Put("I3Calibration", calibration1, I3Frame::Calibration);
  // D
  I3DetectorStatusServicePtr detstatservice(new I3GCDFileDetectorStatusService(gcd));
  assert (detstatservice);
  I3DetectorStatusConstPtr detstat1 = detstatservice->GetDetectorStatus(time1);
  assert (detstat1);
  frame->Put("I3DetectorStatus", detstat1, I3Frame::DetectorStatus);

  // Fake pulses
  printf("About to call.. \n");
  I3RecoPulseSeriesMapConstPtr psmptr = testPSM_tweaked();
  ENSURE(psmptr->size() == 18);
  frame->Put("FakePulseSeriesMap",psmptr);

  // OLD: A bad-station list
  //I3VectorInt bad;  
  //bad.push_back(71);  // this one was not hit
  ////bad.push_back(75);  // this one was hit -- should log_fatal
  //I3VectorIntConstPtr badcp(new I3VectorInt(bad));
  //assert (badcp);
  //frame->Put("BadStationList", badcp);

  // NEW: A bad-tank list
  I3VectorTankKey bad;
  bad.push_back(TankKey(71,TankKey::TankA));  // station not hit
  //bad.push_back(TankKey(75,TankKey::TankA));  // station WAS hit -- should log_fatal
  I3VectorTankKeyConstPtr badcp(new I3VectorTankKey(bad));
  assert (badcp);
  frame->Put("BadTankList", badcp);


  // Create the service
  I3LaputopLikelihood lservice("Laputop");
  lservice.SetReadoutName("FakePulseSeriesMap");
  // Call the function
  int nstation = lservice.FillInput(*frame);
  ENSURE(nstation == 9);
  std::vector<tankPulse> pnorm = lservice.GetInputData();
  std::vector<tankPulse> pempty = lservice.GetInputEmptyData();
  std::vector<tankPulse> psat = lservice.GetInputSaturatedData();

  printf("pnorm %zu\n", pnorm.size());
  printf("pempty %zu\n", pempty.size());
  printf("psat %zu\n", psat.size());

  ENSURE(pnorm.size() == 16);  // tanks
  ENSURE(pempty.size() == 62);  // stations  -- adds up to 71 because 39-61 is bad, and one in "BadStationList"
  ENSURE(psat.size() == 2);

}
