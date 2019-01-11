/**
 * copyright  (C) 2010
 * The Icecube Collaboration
 *
 * $Id$
 *
 * @version $Revision$
 * @date $LastChangedDate$
 * @author Eike Middell <eike.middell@desy.de> $LastChangedBy$
 */

#include "I3LogLikelihoodFitParamsConverter.h"

I3TableRowDescriptionPtr I3LogLikelihoodFitParamsConverter::CreateDescription(const I3LogLikelihoodFitParams& params) {
    I3TableRowDescriptionPtr desc(new I3TableRowDescription());
    desc->AddField<double>("logl" , "", "negative log likelihood");
    desc->AddField<double>("rlogl", "", "reduced negative log likelihood (logl/(ndof))");
    desc->AddField<int32_t>("ndof", "", "number of degrees of freedom (e.g nchannel,npulses... - nparams)");
    desc->AddField<int32_t>("nmini", "", "number of times that the likelihood function was evaluated by the minimizer");
    return desc;
}

size_t I3LogLikelihoodFitParamsConverter::FillRows(const I3LogLikelihoodFitParams& params,
                                     I3TableRowPtr rows) {
    rows->Set<double>("logl" ,  params.logl_ );
    rows->Set<double>("rlogl",  params.rlogl_);
    rows->Set<int32_t>("ndof",  params.ndof_);
    rows->Set<int32_t>("nmini", params.nmini_);

    return 1;
}
