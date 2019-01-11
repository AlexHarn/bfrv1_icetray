#ifndef I3LOGLIKELIHOODFITPARAMSCONVERTER_H_INCLUDED
#define I3LOGLIKELIHOODFITPARAMSCONVERTER_H_INCLUDED
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

#include "tableio/I3Converter.h"
#include "gulliver/I3LogLikelihoodFitParams.h"

class I3LogLikelihoodFitParamsConverter : public I3ConverterImplementation<I3LogLikelihoodFitParams > {
private:
    I3TableRowDescriptionPtr CreateDescription(const I3LogLikelihoodFitParams & params);
    size_t FillRows(const I3LogLikelihoodFitParams& params, I3TableRowPtr rows);
};


#endif  // I3LOGLIKELIHOODFITPARAMSCONVERTER_H_INCLUDED
