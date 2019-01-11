#ifndef IPDF_SimplePEHit_H
#define IPDF_SimplePEHit_H

/**
    copyright  (C) 2004
    the icecube collaboration
    $Id$

    @file SimplePEHit.h
    @version $Revision: 1.3 $
    @date $Date$
    @author S Robbins <robbins@physik.uni-wuppertal.de>
*/

#include <iosfwd>

namespace IPDF {

class SimpleOmReceiver;

/**
    @brief Basic implementation of a photo-electron hit on a 
    single OM.  Provides access to the event specific info 
    regarding a single "pulse" on an OM: leading edge time, 
    and amplitude.
 */
class SimplePEHit {
public:
  typedef double time_result;
  typedef double peamplitude_result;
  typedef SimpleOmReceiver OmReceiver;

/**
 * @brief Ctor taking ipdf native OmReceiver and hit time
 *
 * @param omr is the OMReceiver on which this hit occured
 * @param leading_edge is the time in nano-seconds for the given photo-electron hit
 * @param photo_electron_amplitude is the amplitude of the pulse in single photo-electrons
 */
  SimplePEHit(const SimpleOmReceiver& omr, 
	      double leading_edge, 
	      double photo_electron_amplitude = 1.)
      : omr_(omr), le_(leading_edge), pe_(photo_electron_amplitude)  { }

  /// @brief Access the OmReceiver which was hit
  const SimpleOmReceiver& getOmReceiver() const { return omr_; }

  /// @brief Change the time (in nano-seconds) of this hit
  void        setLeTime(time_result newLE) {le_ = newLE;}; 
  /// @brief Access the time (in nano-seconds) of this hit
  time_result        getLeTime() const {return le_;}; 
  /// @brief Access the amplitude of the pulse in single photo-electrons
  peamplitude_result getPeAmplitude() const {return pe_;}; 

  /// @brief useful helper function (not manditory)
  friend std::ostream& operator<<(std::ostream&, const SimplePEHit&);

private:
  const SimpleOmReceiver& omr_;
  double le_;
  const double pe_;
};

std::ostream& operator<<(std::ostream& os, 
    const IPDF::SimplePEHit& pehit);

} // namespace IPDF

#endif // IPDF_SimplePEHit_H
