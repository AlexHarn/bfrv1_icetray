/**
 *  copyright  (C) 2004
 *  the icecube collaboration
 *  $Id$
 *
 *  @file
 *  @version $Revision$
 *  @date $Date$
 *  @author David Boersma <boersma@icecube.wisc.edu>
 */

#include "gulliver/I3LogLikelihoodFit.h"

const std::string I3LogLikelihoodFit::PARTICLE_SUFFIX = "";
const std::string I3LogLikelihoodFit::NONSTD_SUFFIX = "Params";
const std::string I3LogLikelihoodFit::FITPARAMS_SUFFIX = "FitParams";
const std::string I3LogLikelihoodFit::PARTICLEVECT_SUFFIX = "Vect";
const std::string I3LogLikelihoodFit::FITPARAMSVECT_SUFFIX = "FitParamsVect";
const std::string I3LogLikelihoodFit::NONSTDVECT_SUFFIX = "ParamsVect";

// comparison operators, useful for sorting solutions
bool operator<(const I3LogLikelihoodFit& lhs, const I3LogLikelihoodFit& rhs){
      return lhs.fitparams_->rlogl_ < rhs.fitparams_->rlogl_;
}
bool operator>(const I3LogLikelihoodFit& lhs, const I3LogLikelihoodFit& rhs){
      return lhs.fitparams_->rlogl_ > rhs.fitparams_->rlogl_;
}
