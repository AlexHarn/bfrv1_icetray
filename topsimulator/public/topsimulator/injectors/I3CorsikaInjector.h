/**
 * Copyright (C) 2009
 * The IceCube collaboration
 * ID: $Id: $
 *
 * @file I3CorsikaInjector.h
 * @version $Rev: $
 * @date $Date: $
 * @authors Tilo Waldenmaier, Alessio Tamburro
 */


#ifndef _TOPSIMULATOR_I3CORSIKAINJECTOR_H_
#define _TOPSIMULATOR_I3CORSIKAINJECTOR_H_


#include <topsimulator/interface/I3InjectorService.h>
#include <topsimulator/injectors/I3CorsikaReader.h>
#include <topsimulator/ExtendedI3Particle.h>
#include <topsimulator/injectors/SparseHistogram.h>
#include <phys-services/I3RandomService.h>
#include <dataclasses/TankKey.h>

#include <boost/shared_ptr.hpp>

#include <map>
#include <string>

#ifdef I3_USE_ROOT
#include <TFile.h>
#include <TH2D.h>
#endif


/**
 * \brief InjectorService that scans through a CORSIKA file and iterates over the particles
 *
 * It randomly choses the the core position within a circular
 * sampling area and resamples the same shower several times. It also performs the
 * un-thinning of thinned CORSIKA files and can optionally insert high pT muons.
 */

class I3CorsikaInjector: public I3InjectorService
{
public:
  I3CorsikaInjector(const I3Context& context);
  ~I3CorsikaInjector();

  void Configure();

  bool NextEvent(int& runID, int& evtID, I3Particle& primary, I3FrameConstPtr frame);

  bool NextParticle(ExtendedI3Particle& primary);

  std::map<std::string, int> GetAirShowerComponentNameMap() const;

  I3FrameObjectConstPtr GetEventInfo();

  void Finish();

private:

  void ClearBuffers();

  void CalculateOnRegions(const I3Particle& primary);

  void CalculateArrayFootprint(const I3Particle& primary);

  void ComputeOptDistances();

  void GenerateClones(const I3Particle& particle, const double& weight);

  void SetClonePosition(double &x, double &y, double &z, const I3Particle& particle, const double height, const double radius);

  bool InsertHighPtMuon(I3Particle& particle);

  AirShowerComponent GetAirShowerComponent(const I3Particle& p, double hadgen) const;

  // -------

  std::vector<std::string> corsikaFiles_;
  I3CorsikaReader reader_;

  int relocationStation_;
  std::vector<TankKey> tankKeys_;
  double relocationX_;
  double relocationY_;
  double relocationR_;
  double shiftX_;
  double shiftY_;

  int currentRunID_;
  int currentEvtID_;
  int numSamples_;
  int sampleIndex_;

  I3Particle primary_;

  int numHpTMuons_;
  int hpTMuonCounter_;
  double hpTMuTotalMomentum_;
  double hpTMuTransMomentum_;

  std::map<TankKey, double> optDistMap_;
  std::vector<I3Particle> cloneBuffer_;
  double dcheck_;
  bool alreadyWarned_;

  // All data members related to importance sampling
#ifdef I3_USE_ROOT
  boost::shared_ptr<TFile> pDistr_;
  TH2D* debugHistogram_;
  void ConvertToROOT(const topsim::SparseHistogram& sparse, TH2D* h);
#endif
  boost::shared_ptr<topsim::SparseHistogram> phDistr_;
  boost::shared_ptr<topsim::SparseHistogram> particleDistr_;
  boost::shared_ptr<topsim::SparseHistogram> arrayFootprint_;

  double samplingWeight_;
  bool weightedRandomSampling_;
  double samplingRegionSide_;
  double onRegionSide_;
  double arraySide_;
  double tankSampleDistance_;
  double raiseObsLevel_;  // For hacking when the ObsLevel is set too low.
  bool autoObsLevelHack_;

  std::vector<double> relocsX_;
  std::vector<double> relocsY_;
  int nominalNumSamples_;

  I3RandomServicePtr randomService_;

  std::vector<int> ignoreTypes_;

  SET_LOGGER("I3CorsikaInjector");
};


#endif
