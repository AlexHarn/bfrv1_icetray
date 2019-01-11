/**
 * @author Jan Luenemann <jan.luenemann@uni-mainz.de>,Kai Schatto
 *
 * @class CramerRao
 * @brief calculates estimator
 *
 * This module calculates an estimator for track resolution using Cramer-Rao inequality. Further informations can be found on http://butler.physik.uni-mainz.de/I3Wiki/index.php/Estimation_of_resolution_and_likelihood_as_cut_parameter
 *
 * options: 
 *
 *  InputResponse: Name of PulseSeries or HitSeries
 *
 *  InputTrack: Name of InputTrack
 *
 *  OutputResult: Name of output
 *
 *  AllHits: if this option is selected DOMs are weighted with number of hits
 *
 *  z_dependent_scatter: if this option is selected a depth dependent scatterlenght is used
 *
 *  PathToTable: If I3_SRC is set at your system, you don't need this. Else you have to copy the tables that are used for calculation somewhere and than set the path directory. The tables are in resources/data/.
 *
 *  DoubleOutput: if this option is selected the estimated standard deviations "cramer_rao_theta" and "cramer_rao_phi" are written out as doubles. These can be easily included into flat-ntuples.
 */

#ifndef CRAMERRAO_H
#define CRAMERRAO_H

#include <icetray/I3ConditionalModule.h>
#include <dataclasses/I3Map.h>
#include <dataclasses/physics/I3RecoPulse.h>

class CramerRaoParams;
class I3Geometry;
class I3Particle;

class CramerRao : public I3ConditionalModule
{
 public:
  CramerRao(const I3Context& context);
  void Configure();
  void Physics(I3FramePtr frame);
  
 private:
  CramerRaoParams GetParams(const I3Geometry& , const I3RecoPulseSeriesMap& , const I3Particle& );
    
  std::string inputHits_;
  std::string inputTrack_;
  std::string outputResult_;
  std::string pathToTable_;
  bool allHits_;
  bool doubleOutput_;
  bool z_dependent_scatter_;
  double gamma_value_[400][122];

  SET_LOGGER( "CramerRao" );
};

#endif
