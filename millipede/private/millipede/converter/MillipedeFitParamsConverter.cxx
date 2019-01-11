/**
 * copyright  (C) 2011
 * The Icecube Collaboration
 *
 * $Id$
 *
 * @version $Revision$
 * @date $LastChangedDate$
 * @author Jakob van Santen <vansanten@wisc.edu> $LastChangedBy$
 */

#include "MillipedeFitParamsConverter.h"

I3TableRowDescriptionPtr MillipedeFitParamsConverter::CreateDescription(const MillipedeFitParams& params) {
    I3TableRowDescriptionPtr desc(new I3TableRowDescription());
    desc->AddField<double>("logl" , "", "negative log likelihood");
    desc->AddField<double>("rlogl", "", "reduced negative log likelihood (logl/(ndof))");
    desc->AddField<int32_t>("ndof", "", "number of degrees of freedom (i.e. time bins)");
    desc->AddField<int32_t>("nmini", "", "number of times that the likelihood function was evaluated by the minimizer");
    desc->AddField<double>("qtotal",           "PE", "Total charge of pulses considered in the likelihood.");
    desc->AddField<double>("predicted_qtotal", "PE", "Mean total charge predicted by the fit hypothesis.");
    desc->AddField<double>("squared_residuals", "PE^2", "Sum of squares of the differences between expected and actual DOM charges");
    desc->AddField<double>("chi_squared", "PE", "Chi-squared value of the fit");
    desc->AddField<double>("chi_squared_dof", "",   "Effective number of degrees of freedom in the fit (i.e. number of fit particles)");
    desc->AddField<double>("logl_ratio", "",   "log of the ratio of the maximum possible likelihood on the data to the best-fit likelihood");
    return desc;
}
    
size_t MillipedeFitParamsConverter::FillRows(const MillipedeFitParams& params,
                                     I3TableRowPtr rows) {
    rows->Set<double>("logl" ,  params.logl_ );
    rows->Set<double>("rlogl",  params.rlogl_);
    rows->Set<int32_t>("ndof",  params.ndof_);
    rows->Set<int32_t>("nmini", params.nmini_);
    rows->Set<double>("qtotal" ,           params.qtotal);
    rows->Set<double>("predicted_qtotal",  params.predicted_qtotal);
    rows->Set<double>("squared_residuals", params.squared_residuals);
    rows->Set<double>("chi_squared",       params.chi_squared);
    rows->Set<double>("chi_squared_dof",   params.chi_squared_dof);
    rows->Set<double>("logl_ratio",        params.logl_ratio);
    return 1;
}
