#ifndef RANDOMINITSPECS_INCLUDED
#define RANDOMINITSPECS_INCLUDED

#include "phys-services/I3RandomService.h"

/******************************************************\
 * Convenience function: create random initialization *
\******************************************************/

typedef std::vector<I3FitParameterInitSpecs> specs_t;
specs_t RandomInitSpecs( unsigned int dim, double scale, I3RandomServicePtr rndptr, double stepfactor = 0.1, bool bounds=false ){
    specs_t specs;
    I3FitParameterInitSpecs newspecs("test");
    for ( unsigned int d = 0; d < dim; ++d ){
        std::ostringstream name;
        name << "p" << d;
        newspecs.name_ = name.str();
        newspecs.initval_ = rndptr->Uniform(-scale,+scale);
        newspecs.stepsize_ = stepfactor * rndptr->Uniform(0.5*scale,scale);
        if (bounds){
            // choose bounds at least one step away from the actual minimum
            newspecs.minval_ = -1*(1+stepfactor)*scale;
            newspecs.maxval_ = +1*(1+stepfactor)*scale;
        } else {
            newspecs.minval_ = NAN;
            newspecs.maxval_ = NAN;
        }
        specs.push_back( newspecs );
    }
    return specs;
};

#endif /* RANDOMINITSPECS_INCLUDED */

