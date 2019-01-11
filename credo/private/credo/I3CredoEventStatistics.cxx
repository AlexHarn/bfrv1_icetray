/*
 *  $Id$
 *  @version $Revision$
 *  @date $Date$
 *  @author emiddell
*/

#include <icetray/serialization.h>
#include "credo/I3CredoEventStatistics.h"

/******************************************************************************/

I3CredoEventStatistics::I3CredoEventStatistics() { 
    Reset(); 
}

/******************************************************************************/

I3CredoEventStatistics::~I3CredoEventStatistics() {}

/******************************************************************************/

void I3CredoEventStatistics::Reset() {
    nCh_hit = nCh_selected = nCh_minball = 0;
    npe_all = npe_selected_all = npe_selected_poisson = npe_max = 0;
    nPulses_all_good = nPulses_all_bad = nPulses_selected_poisson = nPulses_selected_gaussian = 0;
    skipped_saturated = skipped_mincharge = skipped_noStatus = skipped_noHV = 0;
    failed_photorec_calls = 0;

    start_time = +HUGE_VAL;
    end_time   = -HUGE_VAL;
}

/******************************************************************************/

template <class Archive>
void I3CredoEventStatistics::serialize(Archive& ar, unsigned version) {
  ar & make_nvp("nCh_hit",  nCh_hit );
  ar & make_nvp("nCh_selected",  nCh_selected );
  ar & make_nvp("nCh_minball",  nCh_minball );
  ar & make_nvp("npe_all",  npe_all );
  ar & make_nvp("npe_selected_all",  npe_selected_all );
  ar & make_nvp("npe_selected_poisson",  npe_selected_poisson );
  ar & make_nvp("npe_max",  npe_max );
  ar & make_nvp("nPulses_all_good",  nPulses_all_good );
  ar & make_nvp("nPulses_all_bad",  nPulses_all_bad );
  ar & make_nvp("nPulses_all_bad",  nPulses_all_bad );
  ar & make_nvp("nPulses_selected_poisson",  nPulses_selected_poisson );
  ar & make_nvp("nPulses_selected_gaussian",  nPulses_selected_gaussian );
  ar & make_nvp("skipped_saturated",  skipped_saturated );
  ar & make_nvp("skipped_mincharge",  skipped_mincharge );
  ar & make_nvp("skipped_noStatus",  skipped_noStatus );
  ar & make_nvp("skipped_noHV",  skipped_noHV );
  ar & make_nvp("failed_photorec_calls",  failed_photorec_calls );
  
  ar & make_nvp("start_time",  start_time );
  ar & make_nvp("end_time",  end_time );
}

/******************************************************************************/

I3_CLASS_VERSION(I3CredoEventStatistics, 0);
I3_SERIALIZABLE(I3CredoEventStatistics);
