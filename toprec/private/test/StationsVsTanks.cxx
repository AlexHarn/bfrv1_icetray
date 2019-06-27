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

#include "interfaces/I3GeometryService.h"
#include "interfaces/I3CalibrationService.h"
#include "interfaces/I3DetectorStatusService.h"

#include "dataclasses/geometry/I3Geometry.h"
#include "dataclasses/calibration/I3Calibration.h"
#include "dataclasses/status/I3DetectorStatus.h"
#include "phys-services/source/I3GCDFileService.h"

#include "toprec/I3LaputopLikelihood.h"

// Let's create a hypothetical pulseseries!
// Harvested from data using: find_nice_test_events.py
// This is the 3rd surviving event from 119221 (out of 4 total)
// With a few modifications to test BadTank logic and other things
I3RecoPulseSeriesMapPtr testPSM_HLC() {
  I3RecoPulseSeriesMapPtr psm(new I3RecoPulseSeriesMap);
  OMKey key;
  I3RecoPulse pulse;
  I3RecoPulseSeries ps;
  key.SetString( 4 );  key.SetOM( 61 );
  pulse.SetTime( 10371.3623047 );  pulse.SetCharge( 0.302982091904 );  pulse.SetWidth( 300.935333252 );  pulse.SetFlags( 3 );
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  key.SetString( 4 );  key.SetOM( 63 );
  pulse.SetTime( 10307.9707031 );  pulse.SetCharge( 0.400370806456 );  pulse.SetWidth( 338.201416016 );  pulse.SetFlags( 3 );
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  key.SetString( 9 );  key.SetOM( 61 );
  pulse.SetTime( 10140.0898438 );  pulse.SetCharge( 1.90385448933 );  pulse.SetWidth( 250.869750977 );  pulse.SetFlags( 3 );
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  key.SetString( 9 );  key.SetOM( 63 );
  pulse.SetTime( 10159.4658203 );  pulse.SetCharge( 2.08018326759 );  pulse.SetWidth( 317.275146484 );  pulse.SetFlags( 3 );
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  key.SetString( 10 );  key.SetOM( 61 );
  pulse.SetTime( 10198.1298828 );  pulse.SetCharge( 2.13855099678 );  pulse.SetWidth( 387.164276123 );  pulse.SetFlags( 3 );
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  key.SetString( 10 );  key.SetOM( 63 );
  pulse.SetTime( 10187.4414062 );  pulse.SetCharge( 2.39899730682 );  pulse.SetWidth( 397.494415283 );  pulse.SetFlags( 3 );
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  key.SetString( 11 );  key.SetOM( 61 );
  pulse.SetTime( 10268.0136719 );  pulse.SetCharge( 0.555866479874 );  pulse.SetWidth( 419.492431641 );  pulse.SetFlags( 3 );
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  // Let's try removing one of the HLC's, and adding it to the "BadTankList"
  //  key.SetString( 11 );  key.SetOM( 63 );
  //  pulse.SetTime( 10254.6025391 );  pulse.SetCharge( 1.74658000469 );  pulse.SetWidth( 252.123092651 );  pulse.SetFlags( 3 );
  //  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  key.SetString( 12 );  key.SetOM( 61 );
  pulse.SetTime( 10349.40625 );  pulse.SetCharge( 0.688918113708 );  pulse.SetWidth( 382.268859863 );  pulse.SetFlags( 3 );
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  key.SetString( 12 );  key.SetOM( 63 );
  pulse.SetTime( 10368.4570312 );  pulse.SetCharge( 0.440664023161 );  pulse.SetWidth( 340.602416992 );  pulse.SetFlags( 3 );
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  key.SetString( 16 );  key.SetOM( 61 );
  pulse.SetTime( 10062.4726562 );  pulse.SetCharge( 2.83853626251 );  pulse.SetWidth( 404.765808105 );  pulse.SetFlags( 3 );
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  key.SetString( 16 );  key.SetOM( 63 );
  pulse.SetTime( 10063.1396484 );  pulse.SetCharge( 2.6821436882 );  pulse.SetWidth( 304.691558838 );  pulse.SetFlags( 3 );
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  key.SetString( 17 );  key.SetOM( 61 );
  pulse.SetTime( 10093.8212891 );  pulse.SetCharge( 14.0648956299 );  pulse.SetWidth( 209.242462158 );  pulse.SetFlags( 3 );
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  key.SetString( 17 );  key.SetOM( 63 );
  pulse.SetTime( 10103.0224609 );  pulse.SetCharge( 14.1426038742 );  pulse.SetWidth( 324.090820312 );  pulse.SetFlags( 3 );
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  key.SetString( 18 );  key.SetOM( 61 );
  pulse.SetTime( 10143.3837891 );  pulse.SetCharge( 15.4911203384 );  pulse.SetWidth( 216.65145874 );  pulse.SetFlags( 3 );
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  key.SetString( 18 );  key.SetOM( 63 );
  pulse.SetTime( 10145.6074219 );  pulse.SetCharge( 17.7346229553 );  pulse.SetWidth( 178.208328247 );  pulse.SetFlags( 3 );
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  key.SetString( 19 );  key.SetOM( 61 );
  pulse.SetTime( 10218.6855469 );  pulse.SetCharge( 3.37583971024 );  pulse.SetWidth( 371.871795654 );  pulse.SetFlags( 3 );
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  key.SetString( 19 );  key.SetOM( 63 );
  pulse.SetTime( 10215.3798828 );  pulse.SetCharge( 3.43845891953 );  pulse.SetWidth( 281.838531494 );  pulse.SetFlags( 3 );
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  key.SetString( 20 );  key.SetOM( 61 );
  pulse.SetTime( 10367.9980469 );  pulse.SetCharge( 0.684911906719 );  pulse.SetWidth( 417.395935059 );  pulse.SetFlags( 3 );
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  key.SetString( 20 );  key.SetOM( 63 );
  pulse.SetTime( 10344.4521484 );  pulse.SetCharge( 0.803650021553 );  pulse.SetWidth( 393.686950684 );  pulse.SetFlags( 3 );
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  key.SetString( 24 );  key.SetOM( 61 );
  pulse.SetTime( 10014.6992188 );  pulse.SetCharge( 1.49845087528 );  pulse.SetWidth( 374.204162598 );  pulse.SetFlags( 3 );
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  key.SetString( 24 );  key.SetOM( 63 );
  pulse.SetTime( 9996.44726562 );  pulse.SetCharge( 2.68368220329 );  pulse.SetWidth( 362.269866943 );  pulse.SetFlags( 3 );
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  key.SetString( 25 );  key.SetOM( 61 );
  pulse.SetTime( 10020.9482422 );  pulse.SetCharge( 16.4603595734 );  pulse.SetWidth( 161.421173096 );  pulse.SetFlags( 3 );
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  key.SetString( 25 );  key.SetOM( 64 );
  pulse.SetTime( 10030.3173828 );  pulse.SetCharge( 21.305305481 );  pulse.SetWidth( 151.109085083 );  pulse.SetFlags( 3 );
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  key.SetString( 26 );  key.SetOM( 61 );
  pulse.SetTime( 10056.078125 );  pulse.SetCharge( 1064.88122559 );  pulse.SetWidth( 264.883209229 );  pulse.SetFlags( 3 );
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  key.SetString( 26 );  key.SetOM( 64 );
  pulse.SetTime( 10063.2109375 );  pulse.SetCharge( 646.33404541 );  pulse.SetWidth( 223.839660645 );  pulse.SetFlags( 3 );
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  key.SetString( 27 );  key.SetOM( 61 );
  pulse.SetTime( 10113.7890625 );  pulse.SetCharge( 18.9297599792 );  pulse.SetWidth( 166.151168823 );  pulse.SetFlags( 3 );
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  key.SetString( 27 );  key.SetOM( 63 );
  pulse.SetTime( 10124.6269531 );  pulse.SetCharge( 18.2197437286 );  pulse.SetWidth( 168.899047852 );  pulse.SetFlags( 3 );
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  key.SetString( 28 );  key.SetOM( 61 );
  pulse.SetTime( 10159.9570312 );  pulse.SetCharge( 4.61646270752 );  pulse.SetWidth( 327.201324463 );  pulse.SetFlags( 3 );
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  key.SetString( 28 );  key.SetOM( 63 );
  pulse.SetTime( 10179.8955078 );  pulse.SetCharge( 2.52673912048 );  pulse.SetWidth( 377.63369751 );  pulse.SetFlags( 3 );
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  key.SetString( 29 );  key.SetOM( 61 );
  pulse.SetTime( 11030.1103516 );  pulse.SetCharge( 0.0792507454753 );  pulse.SetWidth( 106.308097839 );  pulse.SetFlags( 3 );
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  key.SetString( 29 );  key.SetOM( 63 );
  pulse.SetTime( 10810.0683594 );  pulse.SetCharge( 0.276218652725 );  pulse.SetWidth( 301.320343018 );  pulse.SetFlags( 3 );
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  key.SetString( 34 );  key.SetOM( 61 );
  pulse.SetTime( 9957.78515625 );  pulse.SetCharge( 4.72101211548 );  pulse.SetWidth( 240.76701355 );  pulse.SetFlags( 3 );
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  key.SetString( 34 );  key.SetOM( 63 );
  pulse.SetTime( 9964.09375 );  pulse.SetCharge( 9.18498706818 );  pulse.SetWidth( 252.610687256 );  pulse.SetFlags( 3 );
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  key.SetString( 35 );  key.SetOM( 61 );
  pulse.SetTime( 9975.19433594 );  pulse.SetCharge( 14.8808946609 );  pulse.SetWidth( 230.772628784 );  pulse.SetFlags( 3 );
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  key.SetString( 35 );  key.SetOM( 63 );
  pulse.SetTime( 9977.1640625 );  pulse.SetCharge( 16.9226760864 );  pulse.SetWidth( 161.260177612 );  pulse.SetFlags( 3 );
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  key.SetString( 36 );  key.SetOM( 62 );
  pulse.SetTime( 10037.0009766 );  pulse.SetCharge( 136.356552124 );  pulse.SetWidth( 183.923522949 );  pulse.SetFlags( 3 );
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  key.SetString( 36 );  key.SetOM( 64 );
  pulse.SetTime( 10029.8125 );  pulse.SetCharge( 125.778060913 );  pulse.SetWidth( 176.93309021 );  pulse.SetFlags( 3 );
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  key.SetString( 37 );  key.SetOM( 61 );
  pulse.SetTime( 10095.6005859 );  pulse.SetCharge( 8.68883895874 );  pulse.SetWidth( 261.036804199 );  pulse.SetFlags( 3 );
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  key.SetString( 37 );  key.SetOM( 63 );
  pulse.SetTime( 10097.1533203 );  pulse.SetCharge( 10.7304430008 );  pulse.SetWidth( 247.81401062 );  pulse.SetFlags( 3 );
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  key.SetString( 43 );  key.SetOM( 61 );
  pulse.SetTime( 9877.62304688 );  pulse.SetCharge( 2.91671967506 );  pulse.SetWidth( 367.417663574 );  pulse.SetFlags( 3 );
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  key.SetString( 43 );  key.SetOM( 63 );
  pulse.SetTime( 9924.84667969 );  pulse.SetCharge( 1.11868441105 );  pulse.SetWidth( 347.339813232 );  pulse.SetFlags( 3 );
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  key.SetString( 45 );  key.SetOM( 61 );
  pulse.SetTime( 9974.84082031 );  pulse.SetCharge( 5.95918798447 );  pulse.SetWidth( 162.978973389 );  pulse.SetFlags( 3 );
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  key.SetString( 45 );  key.SetOM( 63 );
  pulse.SetTime( 9961.68457031 );  pulse.SetCharge( 5.3907623291 );  pulse.SetWidth( 167.729324341 );  pulse.SetFlags( 3 );
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  key.SetString( 46 );  key.SetOM( 61 );
  pulse.SetTime( 10014.6191406 );  pulse.SetCharge( 2.96145391464 );  pulse.SetWidth( 396.507537842 );  pulse.SetFlags( 3 );
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  key.SetString( 46 );  key.SetOM( 63 );
  pulse.SetTime( 10027.8808594 );  pulse.SetCharge( 5.84685516357 );  pulse.SetWidth( 191.795974731 );  pulse.SetFlags( 3 );
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  key.SetString( 53 );  key.SetOM( 61 );
  pulse.SetTime( 9875.49511719 );  pulse.SetCharge( 1.07368755341 );  pulse.SetWidth( 360.133483887 );  pulse.SetFlags( 3 );
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  key.SetString( 53 );  key.SetOM( 63 );
  pulse.SetTime( 9914.9375 );  pulse.SetCharge( 1.12383019924 );  pulse.SetWidth( 389.31918335 );  pulse.SetFlags( 3 );
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  key.SetString( 54 );  key.SetOM( 61 );
  pulse.SetTime( 9891.02734375 );  pulse.SetCharge( 7.97609758377 );  pulse.SetWidth( 188.371627808 );  pulse.SetFlags( 3 );
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  key.SetString( 54 );  key.SetOM( 63 );
  pulse.SetTime( 9925.46191406 );  pulse.SetCharge( 1.06079113483 );  pulse.SetWidth( 390.564880371 );  pulse.SetFlags( 3 );
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  key.SetString( 55 );  key.SetOM( 61 );
  pulse.SetTime( 9975.26660156 );  pulse.SetCharge( 2.96080613136 );  pulse.SetWidth( 216.466430664 );  pulse.SetFlags( 3 );
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  key.SetString( 55 );  key.SetOM( 63 );
  pulse.SetTime( 9972.09179688 );  pulse.SetCharge( 1.09213602543 );  pulse.SetWidth( 270.871063232 );  pulse.SetFlags( 3 );
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  // Here's another pulse we'll remove by hand... but NOT put it in the BadTankList.
  // So the code will treat it like a not-hit tank
  //key.SetString( 56 );  key.SetOM( 61 );
  //pulse.SetTime( 10026.5712891 );  pulse.SetCharge( 0.597805023193 );  pulse.SetWidth( 240.91633606 );  pulse.SetFlags( 3 );
  //ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  key.SetString( 56 );  key.SetOM( 63 );
  pulse.SetTime( 10074.5556641 );  pulse.SetCharge( 2.18732500076 );  pulse.SetWidth( 129.927627563 );  pulse.SetFlags( 3 );
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  key.SetString( 62 );  key.SetOM( 61 );
  pulse.SetTime( 10009.4179688 );  pulse.SetCharge( 0.50829410553 );  pulse.SetWidth( 248.346130371 );  pulse.SetFlags( 3 );
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  key.SetString( 62 );  key.SetOM( 63 );
  pulse.SetTime( 9876.38867188 );  pulse.SetCharge( 0.436066627502 );  pulse.SetWidth( 248.234664917 );  pulse.SetFlags( 3 );
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  key.SetString( 64 );  key.SetOM( 61 );
  pulse.SetTime( 9968.76367188 );  pulse.SetCharge( 1.53991556168 );  pulse.SetWidth( 330.754608154 );  pulse.SetFlags( 3 );
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  key.SetString( 64 );  key.SetOM( 63 );
  pulse.SetTime( 10092.1044922 );  pulse.SetCharge( 1.42447507381 );  pulse.SetWidth( 246.594909668 );  pulse.SetFlags( 3 );
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  key.SetString( 79 );  key.SetOM( 62 );
  pulse.SetTime( 10035.2626953 );  pulse.SetCharge( 875.270263672 );  pulse.SetWidth( 256.006530762 );  pulse.SetFlags( 3 );
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  key.SetString( 79 );  key.SetOM( 64 );
  pulse.SetTime( 10029.8740234 );  pulse.SetCharge( 989.108947754 );  pulse.SetWidth( 267.191253662 );  pulse.SetFlags( 3 );
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  key.SetString( 80 );  key.SetOM( 62 );
  pulse.SetTime( 10056.5693359 );  pulse.SetCharge( 34.6104545593 );  pulse.SetWidth( 159.129272461 );  pulse.SetFlags( 3 );
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  key.SetString( 80 );  key.SetOM( 64 );
  pulse.SetTime( 10061.6806641 );  pulse.SetCharge( 36.3963432312 );  pulse.SetWidth( 193.952545166 );  pulse.SetFlags( 3 );
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  key.SetString( 81 );  key.SetOM( 61 );
  pulse.SetTime( 10029.2138672 );  pulse.SetCharge( 11.7851791382 );  pulse.SetWidth( 286.66607666 );  pulse.SetFlags( 3 );
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  key.SetString( 81 );  key.SetOM( 63 );
  pulse.SetTime( 10038.5390625 );  pulse.SetCharge( 4.25775289536 );  pulse.SetWidth( 249.525466919 );  pulse.SetFlags( 3 );
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  return psm;
} // 64 pulses (32 stations) minus one removed by hand

I3RecoPulseSeriesMapPtr testPSM_SLC() {
  I3RecoPulseSeriesMapPtr psm(new I3RecoPulseSeriesMap);
  OMKey key;
  I3RecoPulse pulse;
  I3RecoPulseSeries ps;
  key.SetString( 5 );  key.SetOM( 63 );
  pulse.SetTime( 10345.5449219 );  pulse.SetCharge( 2.28712701797 );  pulse.SetWidth( 425.246917725 );  pulse.SetFlags( 2 );
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  key.SetString( 7 );  key.SetOM( 61 );
  pulse.SetTime( 10136.015625 );  pulse.SetCharge( 0.305686295033 );  pulse.SetWidth( 425.825836182 );  pulse.SetFlags( 2 );
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  key.SetString( 14 );  key.SetOM( 63 );
  pulse.SetTime( 10084.9414062 );  pulse.SetCharge( 1.20318210125 );  pulse.SetWidth( 425.368865967 );  pulse.SetFlags( 2 );
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  key.SetString( 15 );  key.SetOM( 63 );
  pulse.SetTime( 10074.8837891 );  pulse.SetCharge( 2.54540038109 );  pulse.SetWidth( 423.455047607 );  pulse.SetFlags( 2 );
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  key.SetString( 21 );  key.SetOM( 63 );
  pulse.SetTime( 10403.6289062 );  pulse.SetCharge( 1.37525260448 );  pulse.SetWidth( 425.862091064 );  pulse.SetFlags( 2 );
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  key.SetString( 22 );  key.SetOM( 61 );
  pulse.SetTime( 9962.0234375 );  pulse.SetCharge( 0.453648984432 );  pulse.SetWidth( 425.034118652 );  pulse.SetFlags( 2 );
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  key.SetString( 31 );  key.SetOM( 63 );
  pulse.SetTime( 9979.78027344 );  pulse.SetCharge( 1.00911295414 );  pulse.SetWidth( 424.542938232 );  pulse.SetFlags( 2 );
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  key.SetString( 38 );  key.SetOM( 61 );
  pulse.SetTime( 10255.8789062 );  pulse.SetCharge( 0.593651890755 );  pulse.SetWidth( 425.330780029 );  pulse.SetFlags( 2 );
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  key.SetString( 41 );  key.SetOM( 61 );
  pulse.SetTime( 9918.44335938 );  pulse.SetCharge( 0.232704818249 );  pulse.SetWidth( 422.666351318 );  pulse.SetFlags( 2 );
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  key.SetString( 51 );  key.SetOM( 61 );
  pulse.SetTime( 9816.50488281 );  pulse.SetCharge( 0.916307210922 );  pulse.SetWidth( 423.258636475 );  pulse.SetFlags( 2 );
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  key.SetString( 57 );  key.SetOM( 61 );
  pulse.SetTime( 10100.1445312 );  pulse.SetCharge( 0.696353673935 );  pulse.SetWidth( 424.685394287 );  pulse.SetFlags( 2 );
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  key.SetString( 63 );  key.SetOM( 63 );
  pulse.SetTime( 9945.98046875 );  pulse.SetCharge( 0.633960664272 );  pulse.SetWidth( 424.989562988 );  pulse.SetFlags( 2 );
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  key.SetString( 65 );  key.SetOM( 63 );
  pulse.SetTime( 10000.6748047 );  pulse.SetCharge( 0.467993497849 );  pulse.SetWidth( 425.0652771 );  pulse.SetFlags( 2 );
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  key.SetString( 68 );  key.SetOM( 61 );
  pulse.SetTime( 10386.9472656 );  pulse.SetCharge( 0.727100610733 );  pulse.SetWidth( 425.433685303 );  pulse.SetFlags( 2 );
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  key.SetString( 69 );  key.SetOM( 61 );
  pulse.SetTime( 9942.24316406 );  pulse.SetCharge( 0.284849703312 );  pulse.SetWidth( 425.61328125 );  pulse.SetFlags( 2 );
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  key.SetString( 71 );  key.SetOM( 61 );
  pulse.SetTime( 9945.21972656 );  pulse.SetCharge( 1.06400334835 );  pulse.SetWidth( 425.039398193 );  pulse.SetFlags( 2 );
  ps.push_back(pulse); (*psm)[key] = ps;  ps.clear();
  return psm;
} // 16 pulses, from 16 stations

I3VectorTankKeyPtr testExcluded_HLC() {
  I3VectorTankKeyPtr exvector(new I3VectorTankKey);
  TankKey tkey;
  tkey.SetOMKey(OMKey( 2 ,61));
  exvector->push_back(tkey);
  tkey.SetOMKey(OMKey( 2 ,63));
  exvector->push_back(tkey);
  tkey.SetOMKey(OMKey( 33 ,61));
  exvector->push_back(tkey);
  tkey.SetOMKey(OMKey( 33 ,63));
  exvector->push_back(tkey);
  tkey.SetOMKey(OMKey( 44 ,61));
  exvector->push_back(tkey);
  tkey.SetOMKey(OMKey( 44 ,63));
  exvector->push_back(tkey);
  tkey.SetOMKey(OMKey( 58 ,61));
  exvector->push_back(tkey);
  tkey.SetOMKey(OMKey( 58 ,63));
  exvector->push_back(tkey);
  // This one I removed by hand, as if it were "bad":
  tkey.SetOMKey(OMKey( 11 ,63));
  exvector->push_back(tkey);
  //tkey.SetOMKey(OMKey( 56 ,61));  // <--temporary, to see whether inclusion in bad tank list
  //exvector->push_back(tkey);      //    matters or not, for a bad HLC.  (It doesn't)
  // This one is part of a "not-hit" station (either in HLC's or SLC's)
  tkey.SetOMKey(OMKey( 23 ,61));
  exvector->push_back(tkey);
  return exvector;
}
I3VectorTankKeyPtr testExcluded_SLC() {
  I3VectorTankKeyPtr exvector(new I3VectorTankKey);
  TankKey tkey;
  tkey.SetOMKey(OMKey( 79 ,63));
  exvector->push_back(tkey);
  return exvector;
}

I3EventHypothesisConstPtr testLaputopHypoth() {
  I3Position pos( 1.00873615384 , -108.506076471 , 1945.61887463 );
  I3Direction dir( 0.148907585703 , -0.132194839485 , -0.979975027913 );
  I3ParticlePtr tr(new I3Particle);
  tr->SetPos(pos);
  tr->SetDir(dir);
  tr->SetTime( 10034.7879417 );
  tr->SetType(I3Particle::unknown);
  tr->SetShape(I3Particle::InfiniteTrack);
  tr->SetFitStatus(I3Particle::OK);
  I3LaputopParamsPtr p(new I3LaputopParams);
  p->SetValue(Laputop::Parameter::Log10_S125, log10(19.674998338897719));
  p->SetValue(Laputop::Parameter::Beta, 3.2844713975800599);
  I3EventHypothesisConstPtr ehp(new I3EventHypothesis(tr, p));
  return ehp;
}

/*
// Other stuff from dataio-shovel... that should come out similar but not identical
  <Llh>-127.73298976475697</Llh>
  <Llh_silent>nan</Llh_silent>
  <Chi2>1.0997584611793274</Chi2>
  <Ndf>118</Ndf>
  <RLogL>-1.08248296410811</RLogL>
  <NMini>34</NMini>
  <Chi2_time>5.6165534168932298</Chi2_time>
*/

void fix_snowdepths_NovSnow(I3GeometryPtr g) {
  // Test originally written to use a GCD with something close to right amount of snow in it:
  //std::string gcd = "/wasabi/gcd/GeoCalibDetectorStatus_IC86.55697_corrected_V2_NovSnow.i3.gz";
  // ...but this is not in I3_TESTDATA, so we'll take a similar GCD  and fix it to match:
  // I know this is a kludge, but it'll work and not depend on anything on local disks...
  (g->stationgeo)[1][0].snowheight = 0.287; 
  (g->stationgeo)[1][1].snowheight = 0.207;
  (g->stationgeo)[2][0].snowheight = 0.043;
  (g->stationgeo)[2][1].snowheight = 0.073;
  (g->stationgeo)[3][0].snowheight = 0.029;
  (g->stationgeo)[3][1].snowheight = 0.103;
  (g->stationgeo)[4][0].snowheight = 0.337;
  (g->stationgeo)[4][1].snowheight = 0.130;
  (g->stationgeo)[5][0].snowheight = 0.139;
  (g->stationgeo)[5][1].snowheight = 0.111;
  (g->stationgeo)[6][0].snowheight = 0.357;
  (g->stationgeo)[6][1].snowheight = 0.295;
  (g->stationgeo)[7][0].snowheight = 0.190;
  (g->stationgeo)[7][1].snowheight = 0.049;
  (g->stationgeo)[8][0].snowheight = 0.466;
  (g->stationgeo)[8][1].snowheight = 0.393;
  (g->stationgeo)[9][0].snowheight = 0.248;
  (g->stationgeo)[9][1].snowheight = 0.117;
  (g->stationgeo)[10][0].snowheight = 0.177;
  (g->stationgeo)[10][1].snowheight = 0.157;
  (g->stationgeo)[11][0].snowheight = 0.280;
  (g->stationgeo)[11][1].snowheight = 0.268;
  (g->stationgeo)[12][0].snowheight = 0.085;
  (g->stationgeo)[12][1].snowheight = 0.063;
  (g->stationgeo)[13][0].snowheight = 0.510;
  (g->stationgeo)[13][1].snowheight = 0.617;
  (g->stationgeo)[14][0].snowheight = 0.304;
  (g->stationgeo)[14][1].snowheight = 0.446;
  (g->stationgeo)[15][0].snowheight = 0.412;
  (g->stationgeo)[15][1].snowheight = 0.518;
  (g->stationgeo)[16][0].snowheight = 0.401;
  (g->stationgeo)[16][1].snowheight = 0.300;
  (g->stationgeo)[17][0].snowheight = 0.309;
  (g->stationgeo)[17][1].snowheight = 0.177;
  (g->stationgeo)[18][0].snowheight = 0.170;
  (g->stationgeo)[18][1].snowheight = 0.179;
  (g->stationgeo)[19][0].snowheight = 0.171;
  (g->stationgeo)[19][1].snowheight = 0.161;
  (g->stationgeo)[20][0].snowheight = 0.128;
  (g->stationgeo)[20][1].snowheight = 0.081;
  (g->stationgeo)[21][0].snowheight = 1.695;
  (g->stationgeo)[21][1].snowheight = 1.674;
  (g->stationgeo)[22][0].snowheight = 0.337;
  (g->stationgeo)[22][1].snowheight = 0.396;
  (g->stationgeo)[23][0].snowheight = 0.265;
  (g->stationgeo)[23][1].snowheight = 0.232;
  (g->stationgeo)[24][0].snowheight = 0.557;
  (g->stationgeo)[24][1].snowheight = 0.568;
  (g->stationgeo)[25][0].snowheight = 0.877;
  (g->stationgeo)[25][1].snowheight = 0.695;
  (g->stationgeo)[26][0].snowheight = 1.522;
  (g->stationgeo)[26][1].snowheight = 1.053;
  (g->stationgeo)[27][0].snowheight = 0.386;
  (g->stationgeo)[27][1].snowheight = 0.316;
  (g->stationgeo)[28][0].snowheight = 0.420;
  (g->stationgeo)[28][1].snowheight = 0.671;
  (g->stationgeo)[29][0].snowheight = 2.359;
  (g->stationgeo)[29][1].snowheight = 1.924;
  (g->stationgeo)[30][0].snowheight = 1.140;
  (g->stationgeo)[30][1].snowheight = 1.314;
  (g->stationgeo)[31][0].snowheight = 0.392;
  (g->stationgeo)[31][1].snowheight = 0.333;
  (g->stationgeo)[32][0].snowheight = 0.386;
  (g->stationgeo)[32][1].snowheight = 0.409;
  (g->stationgeo)[33][0].snowheight = 0.436;
  (g->stationgeo)[33][1].snowheight = 0.619;
  (g->stationgeo)[34][0].snowheight = 0.343;
  (g->stationgeo)[34][1].snowheight = 0.318;
  (g->stationgeo)[35][0].snowheight = 0.590;
  (g->stationgeo)[35][1].snowheight = 0.638;
  (g->stationgeo)[36][0].snowheight = 0.332;
  (g->stationgeo)[36][1].snowheight = 0.349;
  (g->stationgeo)[37][0].snowheight = 0.000;
  (g->stationgeo)[37][1].snowheight = 0.066;
  (g->stationgeo)[38][0].snowheight = 1.766;
  (g->stationgeo)[38][1].snowheight = 1.874;
  (g->stationgeo)[39][0].snowheight = 1.689;
  (g->stationgeo)[39][1].snowheight = 1.769;
  (g->stationgeo)[40][0].snowheight = 1.399;
  (g->stationgeo)[40][1].snowheight = 1.274;
  (g->stationgeo)[41][0].snowheight = 0.676;
  (g->stationgeo)[41][1].snowheight = 0.475;
  (g->stationgeo)[42][0].snowheight = 0.304;
  (g->stationgeo)[42][1].snowheight = 0.746;
  (g->stationgeo)[43][0].snowheight = 0.477;
  (g->stationgeo)[43][1].snowheight = 0.463;
  (g->stationgeo)[44][0].snowheight = 0.937;
  (g->stationgeo)[44][1].snowheight = 0.747;
  (g->stationgeo)[45][0].snowheight = 1.286;
  (g->stationgeo)[45][1].snowheight = 1.217;
  (g->stationgeo)[46][0].snowheight = 1.381;
  (g->stationgeo)[46][1].snowheight = 1.420;
  (g->stationgeo)[47][0].snowheight = 1.691;
  (g->stationgeo)[47][1].snowheight = 1.898;
  (g->stationgeo)[48][0].snowheight = 1.626;
  (g->stationgeo)[48][1].snowheight = 1.377;
  (g->stationgeo)[49][0].snowheight = 1.304;
  (g->stationgeo)[49][1].snowheight = 1.445;
  (g->stationgeo)[50][0].snowheight = 1.343;
  (g->stationgeo)[50][1].snowheight = 1.303;
  (g->stationgeo)[51][0].snowheight = 0.428;
  (g->stationgeo)[51][1].snowheight = 0.558;
  (g->stationgeo)[52][0].snowheight = 0.640;
  (g->stationgeo)[52][1].snowheight = 0.760;
  (g->stationgeo)[53][0].snowheight = 0.970;
  (g->stationgeo)[53][1].snowheight = 0.934;
  (g->stationgeo)[54][0].snowheight = 1.286;
  (g->stationgeo)[54][1].snowheight = 1.248;
  (g->stationgeo)[55][0].snowheight = 1.246;
  (g->stationgeo)[55][1].snowheight = 1.072;
  (g->stationgeo)[56][0].snowheight = 1.705;
  (g->stationgeo)[56][1].snowheight = 1.593;
  (g->stationgeo)[57][0].snowheight = 1.363;
  (g->stationgeo)[57][1].snowheight = 0.670;
  (g->stationgeo)[58][0].snowheight = 1.300;
  (g->stationgeo)[58][1].snowheight = 1.328;
  (g->stationgeo)[59][0].snowheight = 1.322;
  (g->stationgeo)[59][1].snowheight = 1.547;
  (g->stationgeo)[60][0].snowheight = 0.786;
  (g->stationgeo)[60][1].snowheight = 0.809;
  (g->stationgeo)[61][0].snowheight = 0.615;
  (g->stationgeo)[61][1].snowheight = 0.648;
  (g->stationgeo)[62][0].snowheight = 0.749;
  (g->stationgeo)[62][1].snowheight = 0.661;
  (g->stationgeo)[63][0].snowheight = 0.814;
  (g->stationgeo)[63][1].snowheight = 0.728;
  (g->stationgeo)[64][0].snowheight = 1.481;
  (g->stationgeo)[64][1].snowheight = 1.266;
  (g->stationgeo)[65][0].snowheight = 1.269;
  (g->stationgeo)[65][1].snowheight = 1.300;
  (g->stationgeo)[66][0].snowheight = 1.229;
  (g->stationgeo)[66][1].snowheight = 1.435;
  (g->stationgeo)[67][0].snowheight = 1.203;
  (g->stationgeo)[67][1].snowheight = 1.193;
  (g->stationgeo)[68][0].snowheight = 0.890;
  (g->stationgeo)[68][1].snowheight = 0.841;
  (g->stationgeo)[69][0].snowheight = 0.591;
  (g->stationgeo)[69][1].snowheight = 0.643;
  (g->stationgeo)[70][0].snowheight = 0.794;
  (g->stationgeo)[70][1].snowheight = 0.869;
  (g->stationgeo)[71][0].snowheight = 1.124;
  (g->stationgeo)[71][1].snowheight = 1.106;
  (g->stationgeo)[72][0].snowheight = 0.904;
  (g->stationgeo)[72][1].snowheight = 0.928;
  (g->stationgeo)[73][0].snowheight = 1.231;
  (g->stationgeo)[73][1].snowheight = 1.069;
  (g->stationgeo)[74][0].snowheight = 1.182;
  (g->stationgeo)[74][1].snowheight = 1.170;
  (g->stationgeo)[75][0].snowheight = 0.777;
  (g->stationgeo)[75][1].snowheight = 0.805;
  (g->stationgeo)[76][0].snowheight = 0.737;
  (g->stationgeo)[76][1].snowheight = 0.850;
  (g->stationgeo)[77][0].snowheight = 1.198;
  (g->stationgeo)[77][1].snowheight = 1.012;
  (g->stationgeo)[78][0].snowheight = 0.791;
  (g->stationgeo)[78][1].snowheight = 0.871;
  (g->stationgeo)[79][0].snowheight = 0.171;
  (g->stationgeo)[79][1].snowheight = 0.414;
  (g->stationgeo)[80][0].snowheight = 0.158;
  (g->stationgeo)[80][1].snowheight = 0.050;
  (g->stationgeo)[81][0].snowheight = 0.000;
  (g->stationgeo)[81][1].snowheight = 0.000;
}


TEST_GROUP(Pnohit);


TEST(StationwiseLikelihood)
{
  printf("Init! \n");
  std::string gcd(getenv("I3_TESTDATA"));
  gcd = gcd+"/GCD/GeoCalibDetectorStatus_IC86.55697_corrected_V2.i3.gz";
  // Create a fake frame, and put some stuff in it
  I3FramePtr frame(new I3Frame(I3Frame::Physics));
  I3Time time1(2012,290881078948110 ); // first event in my test file
  // G
  I3GeometryServicePtr geoservice(new I3GCDFileGeometryService(gcd));
  assert (geoservice);
  I3GeometryConstPtr geometry1 = geoservice->GetGeometry(time1);
  assert (geometry1);
  I3GeometryPtr geometry2(new I3Geometry(*geometry1));  // make a modifyable copy so we can change it
  fix_snowdepths_NovSnow(geometry2);
  frame->Put("I3Geometry", geometry2, I3Frame::Geometry);
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

  // Fake pulses and excluded-tanks lists
  printf("Creating pulseseries \n");
  I3RecoPulseSeriesMapConstPtr hlcpulses = testPSM_HLC();
  I3RecoPulseSeriesMapConstPtr slcpulses = testPSM_SLC();
  ENSURE(hlcpulses->size() == 62);    // Originally 64, but two removed by hand
  ENSURE(slcpulses->size() == 16);
  frame->Put("FakeHLCPulseSeriesMap",hlcpulses);
  frame->Put("FakeSLCPulseSeriesMap",slcpulses);
  I3VectorTankKeyConstPtr badt = testExcluded_HLC();
  frame->Put("BadTankList", badt);

  // An event hypothesis (track and S125/beta)
  I3EventHypothesisConstPtr ehp = testLaputopHypoth();

  // Create the service
  I3LaputopLikelihood lservice("Laputop");
  lservice.SetReadoutName("FakeHLCPulseSeriesMap");
  lservice.SetFunction("dlp");
  lservice.SetCurvature("gausspar");
  // Call the function
  int nstation = lservice.FillInput(*frame);
  ENSURE(nstation == 32);
  std::vector<tankPulse> pnorm = lservice.GetInputData();
  std::vector<tankPulse> pempty = lservice.GetInputEmptyData();
  std::vector<tankPulse> psat = lservice.GetInputSaturatedData();
  //int mult = lservice.GetMultiplicity();

  printf("pnorm %zu\n", pnorm.size());
  printf("pempty %zu\n", pempty.size());
  printf("psat %zu\n", psat.size());
  //printf("multiplicity %d\n", mult);

  // Original event = 64 hit tanks, 3 of them saturated. (That's 32 stations.)
  // In this test, I remove one hit tank and label it "bad".
  ENSURE(pnorm.size() == 59);  // 61 normal hits, but one of them removed and marked "bad", and another removed (not marked bad)
  ENSURE(pempty.size() == 43);  // not-hit stations: originally 44, but one of them with one tank "bad"
  ENSURE(psat.size() == 3);
  // 81 stations - 4 stations (8 tanks) in the bad tank list = 76 stations total

  // Multiplicity?
  // 64 hit tanks, minus the two I removed.  Each has Q and T except 6 MaxIntraStationDiff failures
  // So 64 x 2 - 6 = 122
  // Not working without "SetEvent"!
  //ENSURE(mult == 122);   

  // Create a snowservice and set it in the Likelihood function
  I3SnowCorrectionServiceBasePtr ss = boost::shared_ptr<I3SnowCorrectionServiceBase>(new I3SimpleSnowCorrectionService("Simple225",2.25));
  lservice.SetSnowService(ss);


  // COMPUTE A LIKELIHOOD
  // Should be around -127 ?
  double llh = lservice.GetLogLikelihood(*ehp);
  //ENSURE_DISTANCE(llh, -127.73558, 0.00001);  // With no modifications
  //ENSURE_DISTANCE(llh, -125.56441, 0.00001);  // With one hit tank removed and marked "bad" (one fewer hit tank)
  //ENSURE_DISTANCE(llh, -124.26599, 0.00001);  // With one not-hit tank marked "bad" (one fewer not-hit station)
  ENSURE_DISTANCE(llh, -121.60269, 0.00001);  // With another hit tank removed, but NOT also marked "bad"


  }


