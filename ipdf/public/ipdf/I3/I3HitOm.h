/**
    copyright  (C) 2004
    the icecube collaboration
    $Id$

    @file I3HitOm.h
    @version $Revision: 1.2 $
    @date $Date$
    @author S Robbins <robbins@physik.uni-wuppertal.de>
*/

#ifndef IPDF_I3HitOm_H
#define IPDF_I3HitOm_H

#include "ipdf/I3/I3PEHit.h"

#include "icetray/I3Units.h"
#include "dataclasses/physics/I3AMANDAAnalogReadout.h"
#include "dataclasses/physics/I3RecoHit.h"

#include <vector>
#include <algorithm>
#include <iosfwd>

namespace IPDF {

class I3OmReceiver;
  
  /**
   * @brief Implementation of an hit OM in the Icecube framework.
   */
class I3HitOm {
public:
  typedef double npe_result;
  typedef I3PEHit value_type;  // conform to STL
  typedef std::vector<I3PEHit*>::const_iterator const_iterator;
  typedef I3OmReceiver OmReceiver;

  /// @brief Ctor from ipdf native OmReceiver and vector of PEHits
  explicit I3HitOm(const I3OmReceiver& omr, std::vector<I3PEHit*> hits, bool deleteomr=0)
      : omr_(omr), deleteomr_(deleteomr), photon_hits_(hits),
	num_photo_electrons_(-1.) { }

  /// @brief Ctor from ipdf native OmReceiver and a single PEHit
  explicit I3HitOm(const I3OmReceiver& omr, I3PEHit* hit, bool deleteomr=0)
      : omr_(omr), deleteomr_(deleteomr), photon_hits_(1,hit),
	num_photo_electrons_(-1.)  { }

  /// @brief Ctor from icetray native HitType (e.g. I3RecoHit, I3RecoPulse, etc)
  template<class HitVector>
  explicit I3HitOm(const I3OmReceiver& omr, const HitVector& recohits, bool deleteomr=0)
  : omr_(omr),deleteomr_(deleteomr),
  num_photo_electrons_(0)
  {
    typename HitVector::const_iterator hiter = recohits.begin();
    const typename HitVector::const_iterator hend = recohits.end();
    for(; hiter!=hend; ++hiter) {
      this->insert_hit(*hiter);
    }
    // if no valid LEs then this empty HitOm is ignored by likelihoods (e.g. SPEAll).
  }

  /// @brief Push a new hit into the hit vector
  ///
  /// This method is intended for internal use.  
  /// If you use it from client code _you_ must ensure that the hit is on 
  /// the same OmReceiver as this HitOm.
  template<typename HitType>
  void insert_hit(const HitType& hit) {
    double le = hit.GetTime()/I3Units::nanosecond;
    double charge = hit.GetCharge();
    if ( std::isfinite(le) && (charge>0)){
      log_trace("I3HitOm::insert_hit: leading edge %fns and charge: %f pe",
                le,charge);
      photon_hits_.push_back(new I3PEHit(omr_,le,charge) );
      num_photo_electrons_ += charge;
    } else {
      log_error("I3HitOm::insert_hit: invalid leading edge and/or charge "
                "le=%g, q=%g in DOM at (x,y,z)=(%.1f,%.1f,%.1f)",
                le,charge,omr_.getX(),omr_.getY(),omr_.getZ());
    }
  }

  ~I3HitOm();

  /// @brief Access the OmReceiver on which this hit occurred
  const I3OmReceiver& getOmReceiver() const { return omr_; }

  /// @brief Access total charge measured in photo-electrons
  npe_result getTotalPe() const {
    if( num_photo_electrons_ < 0. ) {
      num_photo_electrons_=0.;
      I3HitOm::const_iterator iter;
      for(iter=this->begin();iter!=this->end();++iter){
        //ignoring hits with nan amplitude
        double peamp = (*iter)->getPeAmplitude();
        if ( peamp > 0. ){
	  num_photo_electrons_+=(*iter)->getPeAmplitude();
        } else if ( std::isnan(peamp) || (peamp < 0.) ){
            log_warn( "got anomalous pulse charge charge=%g PE",
                      peamp );
            log_warn( "this probably means there is a calibration problem." );
        }
      }
    }
    if( num_photo_electrons_ > INT_MAX ) {
      // the INT_MAX limit is chosen because that's the value where the
      // the PdfMath::Nint function will stop working correctly.
      log_warn( "got anomalously large charge=%g PE "
                "in OM at (x,y,z)=(%.1f,%.1f,%.1f)",
                num_photo_electrons_, omr_.getX(), omr_.getY(), omr_.getZ() );
      log_warn( "this probably means there is a calibration problem." );
      log_warn( "For now the charge for this DOM is just set to the "
                "number of pulses..." );
      num_photo_electrons_ = photon_hits_.size();
      if ( num_photo_electrons_ > INT_MAX ) {
        log_warn( "... which did not help either. Forcing it to 1.0 for now." );
        num_photo_electrons_ = 1.0;
      }
    }
    return num_photo_electrons_;
  }

  /// @brief Access to all the photo-electron hits on this OM in this event
  const std::vector<I3PEHit*> getPEHits() const {
    return photon_hits_;
  }

  /// @brief Access an I3PEHit via a simple integer key
  const I3PEHit& operator[](unsigned int ihit) const {
    assert((ihit<photon_hits_.size()) && "ihit out-of-bounds");
    return *photon_hits_[ihit];
  }

  /// @brief STL compliant iterator
  const_iterator begin() const { return photon_hits_.begin(); }
  /// @brief STL compliant iterator
  const_iterator end() const { return photon_hits_.end(); }
  /// @brief STL compliance
  bool empty() const { return photon_hits_.empty(); }
  /// @brief STL compliance
  size_t size() const { return photon_hits_.size(); }

  friend std::ostream& operator<<(std::ostream&, const I3HitOm&);

  SET_LOGGER( "ipdf" );
  
private:
  typedef std::vector<I3PEHit*>::iterator iterator;
  const I3OmReceiver& omr_;
  const bool deleteomr_;
  std::vector<I3PEHit*> photon_hits_;
  mutable double num_photo_electrons_;
};

std::ostream& operator<<(std::ostream& os, 
    const IPDF::I3HitOm&);

// I3AMANDAAnalogReadouts are strange
template<>
inline I3HitOm::I3HitOm(const I3OmReceiver& omr,
			const I3AMANDAAnalogReadout& amaread,
			bool deleteomr)
: omr_(omr), deleteomr_(deleteomr),num_photo_electrons_(amaread.GetAmplitude())
{
  typedef std::vector<double> VectorLE;
  VectorLE leading_edges = amaread.GetLEs();
  for(VectorLE::const_iterator le=leading_edges.begin();
      le!=leading_edges.end(); ++le) {
    if( std::isfinite(*le) ) {
      photon_hits_.push_back(new I3PEHit(omr,*le));
    } else {
      log_debug("I3HitOm ctor: invalid leading edge: %f",(*le));
    }
  }
  // An I3HitOm should never be empty.
  // If a HitOm contains only invalid hits then the likelihoods will probably
  // crash on it. Invalid hits (INF or NAN le time) are a sign that something
  // is fundamentally wrong with your hit/pulse definition or calibration.
  // So crashing is the appropriate response: you really *have* to fix it.
  // We don't want to soothe you into thinking that everything runs fine
  // while producing bogus fits.
  // Maybe we should already crash here, in case of a non-finite LE.
}

// I3RecoHits have no amplitude
template<>
inline void I3HitOm::insert_hit(const I3RecoHit& hit) {
  double le = hit.GetTime()/I3Units::nanosecond;
  if( std::isfinite(le) ) {
    photon_hits_.push_back(new I3PEHit(omr_,le));
    num_photo_electrons_ += 1.; // No amplitude in I3RecoHit
  } else {
    log_debug("I3HitOm::insert_hit<I3RecoHit>: invalid leading edge: %f",(le));
  }
  // An I3HitOm should never be empty.
  // If a HitOm contains only invalid hits then the likelihoods will probably
  // crash on it. Invalid hits (INF or NAN le time) are a sign that something
  // is fundamentally wrong with your hit/pulse definition or calibration.
  // So crashing is the appropriate response: you really *have* to fix it.
  // We don't want to soothe you into thinking that everything runs fine
  // while producing bogus fits.
  // Maybe we should already crash here, in case of a non-finite LE.
}

} // namespace IPDF

#endif // IPDF_I3HitOm_H
