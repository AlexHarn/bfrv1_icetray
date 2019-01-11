#ifndef SNOWCORRECTIONDIAGNOSTICSCONVERTER_H_INCLUDED
#define SNOWCORRECTIONDIAGNOSTICSCONVERTER_H_INCLUDED

/**
 * copyright  (C) 2010
 * The Icecube Collaboration
 *
 * $Id$
 *
 * @version $Revision$
 * @date $LastChangedDate$
 * @author Katherine Rawlins <krawlins@uaa.alaska.edu> $LastChangedBy$
 */

#include "tableio/I3Converter.h"
#include "toprec/SnowCorrectionDiagnostics.h"

class SnowCorrectionDiagnosticsConverter : public I3ConverterImplementation<SnowCorrectionDiagnostics> {
private:
    I3TableRowDescriptionPtr CreateDescription(const SnowCorrectionDiagnostics& params); 
    size_t FillRows(const SnowCorrectionDiagnostics& params, I3TableRowPtr rows);
};
    

#endif  // SNOWCORRECTIONDIAGNOSTICSCONVERTER_H_INCLUDED
