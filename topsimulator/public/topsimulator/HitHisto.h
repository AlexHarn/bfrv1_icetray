/**
 * Copyright (C) 2010
 * The IceCube collaboration
 * ID: $Id: $
 *
 * @file HitHisto.h
 * @version $Rev: $
 * @date $Date: $
 * @author Tilo Waldenmaier, Hans Dembinski
 */

#ifndef _TOPSIMULATOR_HITHISTO_H_
#define _TOPSIMULATOR_HITHISTO_H_

#include <map>
#include <string>
#include <valarray>
#include <simclasses/I3MCPE.h>
#include <dataclasses/I3Map.h>
#include <dataclasses/physics/I3MCHit.h>
#include <dataclasses/physics/I3RecoPulse.h>
#include <dataclasses/physics/I3Particle.h>
#include <topsimulator/ExtendedI3Particle.h>

/**
 * \brief HitHisto groups photo-electrons that arrive in a short time-window.
 *
 * It provides a simple histogram class that bins PEs according to their time.
 * Depending on the constructor arguments, it can also keep track of the origin of the PEs.
 * The origin of the PEs can be traced at two levels of granularity:
 *
 *  \li high granularity: the ID of each I3MCPE is set to the ID of the I3Particle that produced it. This can potentially produce hundreds of thousands of PEs.
 *  \li low granularity: the ID of each * I3MCPE is not a true particle ID. The major ID is set to zero and * the minor ID is set to the air shower component in the * ExtendedI3Particle that produced it.
 *
 * It also provides a set of functions to fill standard frame object
 * containers (usually I3MCPESeries, but there are comvenience methods
 * for I3MCHitSeries, I3RecoPulseSeries).
 *
 * For more detailed comments, look into the source code.
 */

class HitHisto
{
  friend class HitHistoCollection;

  struct NPE: public std::valarray<uint32_t> {
    NPE(int n): std::valarray<uint32_t>(n) {}
  };
  typedef std::map<int, NPE> Histo;

  const double binWidth_;
  I3MCPESeries* peSeries_;
  std::vector<int> components_;
  std::map<int,int> index_; // the "inverse" of components_
  Histo histo_;
  unsigned npesum_; // count all PEs

  /// The main filling method. All others call this.
  void FillBin(int bin, unsigned npe, const ExtendedI3Particle& p);

  // these pass internal data to HitHistoCollection
  /// Fill an I3MCPESeries (all PEs if peSeries_ is not NULL or weighted PEs according to their time otherwise)
  void FillObject(I3MCPESeries&) const;
  /// Convenience method to fill a container of the old I3MCHit. Will be deprecated.
  void FillObject(I3MCHitSeries&) const;
  ///
  void FillObject(I3RecoPulseSeries&) const;
  /// Fill an I3MCPESeries with PEs weighted according to their time and shower component. Will fail if peSeries_ is not NULL and will be empty if components_ is empty.
  void FillWithClassifiedPE(I3MCPESeries&) const;

public:
  /**
   * Constructor specifying the width of the time bin, a pointer to I3MCPESeries and air shower components. Note that series should not be non-NULL if components is not-empty and viceversa.
   *
   * \param binWidth specifies the width of the time bins (in nanosecond)
   * \param series is a pointer to an I3MCPESeries. If it is not NULL (high granularity tracking of PE origin), every time a Fill method is called, an I3MCPE will be added to this (with ID set to the particle that produced it). HitHisto does not own this pointer.
   * \param components contains the enum values of the different possible air shower components. If it is not empty (low granularity tracking of PE origin), I3MCPEs will be grouped by the air shower component of the particle that produced them.
   */
  HitHisto(double binWidth, I3MCPESeries* series, std::vector<int> components);

  double GetBinWidth() const { return binWidth_; }

  /// don't use this anymore, use one of the overloads below
  void Fill(double time, unsigned npe)
  __attribute__ ((deprecated))
  {
    Fill(time, npe, ExtendedI3Particle());
  }

  /// use this for the cherenkov histogram only
  void Fill(double time, unsigned npe, const ExtendedI3Particle& p);

  /// use this for the pe histogram, it allows HitHisto to merge I3MCPEs
  /// generated from the same particle at the same time effectively;
  /// note: we are passing a vector non-const, because it is sorted internally
  void Fill(std::vector<double>& times, const ExtendedI3Particle& p);

  bool HasHits() const { return GetNumHits() > 0; }

  unsigned GetNumHits() const { return npesum_; }

  void Scale(double factor);

  /// these are internal details and should be private...
  int GetBin(double time) const { return static_cast<int>(time / binWidth_); }

  double GetBinCenter(int bin) const { return (bin + 0.5) * binWidth_; }

  double GetBinLowEdge(int bin) const { return bin * binWidth_; }
};


/**
 * \brief Collection of the HitHisto instances per tank (OMKey)
 *
 * It provides a simple histogram class that bins PEs according to their time.
 * Depending on the constructor arguments, it can also keep track of the origin of the PEs.
 * The origin of the PEs can be traced at two levels of granularity:
 *
 *  \li high granularity: the ID of each I3MCPE is set to the ID of the I3Particle that produced it. This can potentially produce hundreds of thousands of PEs.
 *  \li low granularity: the ID of each * I3MCPE is not a true particle ID. The major ID is set to zero and * the minor ID is set to the air shower component in the * ExtendedI3Particle that produced it.
 *
 * It also provides a set of functions to fill standard frame object
 * containers (usually I3MCPESeries, but there are comvenience methods
 * for I3MCHitSeries, I3RecoPulseSeries).
 *
 */

class HitHistoCollection
{
  typedef std::map<OMKey, HitHisto> HistoMap;

  const double binWidth_;
  I3MCPESeriesMapPtr peSeriesMap_; // not null only if compressPEs==0
  HistoMap histoMap_;
  std::vector<int> components_; // non-empty only if compressPEs==1

public:
  /**
   * Construct by specifying , the type of book-keeping to use when handling air shower components, and the enumeration values of the air shower components.
   *
   * The compressPEs parameter controls whether the origin of the I3MCPEs is tracked:
   *   \li 0: I3MCPEs are binned according to time and to the particle that produced them (largest number of I3MCPE objects),
   *   \li 1: I3MCPEs are binned according to time and air shower components specified in \c components (medium number of I3MCPE objects),
   *   \li 2: I3MCPE origin is discarded and all I3MCPEs are binned according to time only} (least number of I3MCPEs)
   *
   * \param binWidth specifies the width of the time bins (in nanosecond)
   * \param compressPEs can take the values {0,1,2}.
   * \param components the air shower component enum values.
   */
  HitHistoCollection(double binWidth, bool compressPEs, std::vector<int> components);

  /// Get the HitHisto for a given OMKey
  HitHisto& GetHitHisto(const OMKey& omKey);
  bool Empty() const { return histoMap_.empty(); }
  int GetEntries() const { return histoMap_.size(); }

  /**
   * Generate frame objects of different types (I3RecoPulseSeries, I3MCHitSeries, I3MCPESeries). This is usually called at the end of I3TopSimulator::DAQ.
   */
  template<typename T>
  boost::shared_ptr<I3Map<OMKey, T> > GenerateMap() const
  {
    typedef I3Map<OMKey, T> OMap;
    boost::shared_ptr<OMap> map(new OMap());
    for (HistoMap::const_iterator
           iter = histoMap_.begin(), end = histoMap_.end();
         iter != end; ++iter)
    {
      if (iter->second.HasHits())
        iter->second.FillObject((*map)[iter->first]);
    }
    return map;
  }
  /**
   * Generate I3MCPESeries frame object. This is the same as GenerateMap, but it is only used for I3MCPEs and only if compressPEs==1.
   */
  I3MCPESeriesMapPtr GeneratePEClassMap() const;
};


// template specialization for I3MCPESeries
template<>
I3MCPESeriesMapPtr HitHistoCollection::GenerateMap<I3MCPESeries>() const;


#endif
