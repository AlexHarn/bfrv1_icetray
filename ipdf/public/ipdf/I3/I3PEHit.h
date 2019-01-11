#ifndef IPDF_I3PEHit_H
#define IPDF_I3PEHit_H

/**
    copyright  (C) 2004
    the icecube collaboration
    $Id$

    @file I3PEHit.h
    @version $Revision: 1.3 $
    @date $Date$
    @author S Robbins <robbins@physik.uni-wuppertal.de>
*/

#include <iosfwd>
#include <cassert>
#include <ipdf/I3/I3OmReceiver.h>

namespace IPDF {

class I3OmReceiver;

  /**
   * @brief Icecube framework implementation of a photo-electron hit on a single OM.
   *
    Provides access to the event specific info 
    regarding a single "pulse" on an OM: leading edge time, 
    and amplitude.
   */
class I3PEHit {
public:
  typedef double time_result;
  typedef double peamplitude_result;
  typedef I3OmReceiver OmReceiver;

  /**
   * @brief Ctor taking ipdf native I3OmReceiver and hit time
   *
   * @param omr is the OMReceiver on which this hit occured
   * @param leading_edge is the time in nano-seconds for the given photo-electron hit
   * @param photo_electron_amplitude is the amplitude of the pulse in single photo-electrons
   */
  I3PEHit(const I3OmReceiver& omr, 
	  double leading_edge, 
	  double photo_electron_amplitude = 1.)
      : omr_(omr), le_(leading_edge), pe_(photo_electron_amplitude) {
    assert(std::isfinite(leading_edge) && "I2PEHit ctor: input leading edge not finite");
  }

  /// @brief Access the OmReceiver which was hit
  const I3OmReceiver& getOmReceiver() const { return omr_; }

  /// @brief Change the time (in nano-seconds) of this hit
  void setLeTime(time_result newLE) {le_ = newLE;}; 
  /// @brief Access the time (in nano-seconds) of this hit
  time_result        getLeTime() const {return le_;}; 
  /// @brief Access the amplitude of the pulse in single photo-electrons
  peamplitude_result getPeAmplitude() const {return pe_;}; 

  friend std::ostream& operator<<(std::ostream&, const I3PEHit&);

private:
  const I3OmReceiver& omr_;
  double le_;
  const double pe_;
};

std::ostream& operator<<(std::ostream& os, 
    const IPDF::I3PEHit&);

} // namespace IPDF

#endif // IPDF_I3PEHit_H
