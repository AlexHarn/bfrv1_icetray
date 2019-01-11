/**
 * copyright  (C) 2010
 * The Icecube Collaboration
 *
 * $Id$
 *
 * @version $Revision$
 * @date $LastChangedDate$
 * @author Katherine Rawlins <krawlins@uaa.alaska.edu>, last changed by $LastChangedBy$
 */

#include "SnowCorrectionDiagnosticsConverter.h" 

I3TableRowDescriptionPtr SnowCorrectionDiagnosticsConverter::CreateDescription(const SnowCorrectionDiagnostics& params)
{
  I3TableRowDescriptionPtr desc(new I3TableRowDescription());
  
  // The stage of shower evolution
  desc->AddField<double> ("tstage", "",  "Estimated stage of shower evolution");
  desc->AddField<double> ("tstage_r", "",  "Estimated stage of shower evolution (restricted to be not too wild)");
  
  // Fraction of signal which is EM
  desc->AddField<double> ("fEM_30m", "", "Fraction of signal expected to be EM at 30m");
  desc->AddField<double> ("fEM_50m", "", "Fraction of signal expected to be EM at 50m");
  desc->AddField<double> ("fEM_80m", "", "Fraction of signal expected to be EM at 80m");
  desc->AddField<double> ("fEM_100m", "", "Fraction of signal expected to be EM at 100m");
  desc->AddField<double> ("fEM_125m", "", "Fraction of signal expected to be EM at 125m");
  desc->AddField<double> ("fEM_150m", "", "Fraction of signal expected to be EM at 150m");
  desc->AddField<double> ("fEM_200m", "", "Fraction of signal expected to be EM at 200m");
  desc->AddField<double> ("fEM_300m", "", "Fraction of signal expected to be EM at 300m");
  desc->AddField<double> ("fEM_500m", "", "Fraction of signal expected to be EM at 500m");
  desc->AddField<double> ("fEM_1000m", "", "Fraction of signal expected to be EM at 1000m");

  // Effective lambda's: EM only
  desc->AddField<double> ("lambda_EM_30m", "m", "EM attenuation length at 30m");
  desc->AddField<double> ("lambda_EM_50m", "m", "EM attenuation length at 50m");
  desc->AddField<double> ("lambda_EM_80m", "m", "EM attenuation length at 80m");
  desc->AddField<double> ("lambda_EM_100m", "m", "EM attenuation length at 100m");
  desc->AddField<double> ("lambda_EM_125m", "m", "EM attenuation length at 125m");
  desc->AddField<double> ("lambda_EM_150m", "m", "EM attenuation length at 150m");
  desc->AddField<double> ("lambda_EM_200m", "m", "EM attenuation length at 200m");
  desc->AddField<double> ("lambda_EM_300m", "m", "EM attenuation length at 300m");
  desc->AddField<double> ("lambda_EM_500m", "m", "EM attenuation length at 500m");
  desc->AddField<double> ("lambda_EM_1000m", "m", "EM attenuation length at 1000m");
  desc->AddField<double> ("lambda_EM_30m_r", "m", "EM attenuation length at 30m (restricted)");
  desc->AddField<double> ("lambda_EM_50m_r", "m", "EM attenuation length at 50m (restricted)");
  desc->AddField<double> ("lambda_EM_80m_r", "m", "EM attenuation length at 80m (restricted)");
  desc->AddField<double> ("lambda_EM_100m_r", "m", "EM attenuation length at 100m (restricted)");
  desc->AddField<double> ("lambda_EM_125m_r", "m", "EM attenuation length at 125m (restricted)");
  desc->AddField<double> ("lambda_EM_150m_r", "m", "EM attenuation length at 150m (restricted)");
  desc->AddField<double> ("lambda_EM_200m_r", "m", "EM attenuation length at 200m (restricted)");
  desc->AddField<double> ("lambda_EM_300m_r", "m", "EM attenuation length at 300m (restricted)");
  desc->AddField<double> ("lambda_EM_500m_r", "m", "EM attenuation length at 500m (restricted)");
  desc->AddField<double> ("lambda_EM_1000m_r", "m", "EM attenuation length at 1000m (restricted)");

  /* not yet
  desc->AddField<double> ("lambda_eff_30m", "m", "Overall effective attenuation length at 30m");
  desc->AddField<double> ("lambda_eff_50m", "m", "Overall effective attenuation length at 50m");
  desc->AddField<double> ("lambda_eff_80m", "m", "Overall effective attenuation length at 80m");
  desc->AddField<double> ("lambda_eff_100m", "m", "Overall effective attenuation length at 100m");
  desc->AddField<double> ("lambda_eff_125m", "m", "Overall effective attenuation length at 125m");
  desc->AddField<double> ("lambda_eff_150m", "m", "Overall effective attenuation length at 150m");
  desc->AddField<double> ("lambda_eff_200m", "m", "Overall effective attenuation length at 200m");
  desc->AddField<double> ("lambda_eff_300m", "m", "Overall effective attenuation length at 300m");
  desc->AddField<double> ("lambda_eff_500m", "m", "Overall effective attenuation length at 500m");
  desc->AddField<double> ("lambda_eff_1000m", "m", "Overall effective attenuation length at 1000m");
  */

  // The key snow depths
  desc->AddField<double> ("snowdepth_39B", "m", "Snow depth of tank 39B");
  desc->AddField<double> ("snowdepth_44A", "m", "Snow depth of tank 44A");
  desc->AddField<double> ("snowdepth_59A", "m", "Snow depth of tank 59A");
  desc->AddField<double> ("snowdepth_74A", "m", "Snow depth of tank 74A");


  return desc;
}
    
size_t SnowCorrectionDiagnosticsConverter::FillRows(const SnowCorrectionDiagnostics& params,
						I3TableRowPtr rows)
{
  // The stage of shower evolution
  rows->Set<double> ("tstage", params.tstage);
  rows->Set<double> ("tstage_r", params.tstage_restricted);
  
  // Fraction of signal which is EM
  rows->Set<double> ("fEM_30m", params.fEM_30m);
  rows->Set<double> ("fEM_50m", params.fEM_50m);
  rows->Set<double> ("fEM_80m", params.fEM_80m);
  rows->Set<double> ("fEM_100m", params.fEM_100m);
  rows->Set<double> ("fEM_125m", params.fEM_125m);
  rows->Set<double> ("fEM_150m", params.fEM_150m);
  rows->Set<double> ("fEM_200m", params.fEM_200m);
  rows->Set<double> ("fEM_300m", params.fEM_300m);
  rows->Set<double> ("fEM_500m", params.fEM_500m);
  rows->Set<double> ("fEM_1000m", params.fEM_1000m);

  // Effective lambda's: EM only
  rows->Set<double> ("lambda_EM_30m", params.lambda_EM_30m);
  rows->Set<double> ("lambda_EM_50m", params.lambda_EM_50m);
  rows->Set<double> ("lambda_EM_80m", params.lambda_EM_80m);
  rows->Set<double> ("lambda_EM_100m", params.lambda_EM_100m);
  rows->Set<double> ("lambda_EM_125m", params.lambda_EM_125m);
  rows->Set<double> ("lambda_EM_150m", params.lambda_EM_150m);
  rows->Set<double> ("lambda_EM_200m", params.lambda_EM_200m);
  rows->Set<double> ("lambda_EM_300m", params.lambda_EM_300m);
  rows->Set<double> ("lambda_EM_500m", params.lambda_EM_500m);
  rows->Set<double> ("lambda_EM_1000m", params.lambda_EM_1000m);
  rows->Set<double> ("lambda_EM_30m_r", params.lambda_EM_30m_restricted);
  rows->Set<double> ("lambda_EM_50m_r", params.lambda_EM_50m_restricted);
  rows->Set<double> ("lambda_EM_80m_r", params.lambda_EM_80m_restricted);
  rows->Set<double> ("lambda_EM_100m_r", params.lambda_EM_100m_restricted);
  rows->Set<double> ("lambda_EM_125m_r", params.lambda_EM_125m_restricted);
  rows->Set<double> ("lambda_EM_150m_r", params.lambda_EM_150m_restricted);
  rows->Set<double> ("lambda_EM_200m_r", params.lambda_EM_200m_restricted);
  rows->Set<double> ("lambda_EM_300m_r", params.lambda_EM_300m_restricted);
  rows->Set<double> ("lambda_EM_500m_r", params.lambda_EM_500m_restricted);
  rows->Set<double> ("lambda_EM_1000m_r", params.lambda_EM_1000m_restricted);

  /* not yet
  rows->Set<double> ("lambda_eff_30m", params.lambda_eff_30m);
  rows->Set<double> ("lambda_eff_50m", params.lambda_eff_50m);
  rows->Set<double> ("lambda_eff_80m", params.lambda_eff_80m);
  rows->Set<double> ("lambda_eff_100m", params.lambda_eff_100m);
  rows->Set<double> ("lambda_eff_125m", params.lambda_eff_125m);
  rows->Set<double> ("lambda_eff_150m", params.lambda_eff_150m);
  rows->Set<double> ("lambda_eff_200m", params.lambda_eff_200m);
  rows->Set<double> ("lambda_eff_300m", params.lambda_eff_300m);
  rows->Set<double> ("lambda_eff_500m", params.lambda_eff_500m);
  rows->Set<double> ("lambda_eff_1000m", params.lambda_eff_1000m);
  */

  // The key snow depths:
  rows->Set<double> ("snowdepth_39B", params.snowdepth_39B);
  rows->Set<double> ("snowdepth_44A", params.snowdepth_44A);
  rows->Set<double> ("snowdepth_59A", params.snowdepth_59A);
  rows->Set<double> ("snowdepth_74A", params.snowdepth_74A);

  return 1;
}


