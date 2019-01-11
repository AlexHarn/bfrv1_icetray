
#ifndef MILLIPEDE_FIT_PARAMS_CONVERTER_H
#define MILLIPEDE_FIT_PARAMS_CONVERTER_H

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

#include <tableio/I3Converter.h>
#include <millipede/Millipede.h>

class MillipedeFitParamsConverter : public I3ConverterImplementation<MillipedeFitParams > {
private:
    I3TableRowDescriptionPtr CreateDescription(const MillipedeFitParams & params); 
    size_t FillRows(const MillipedeFitParams& params, I3TableRowPtr rows);
};
    
#endif // MILLIPEDE_FIT_PARAMS_CONVERTER_H
