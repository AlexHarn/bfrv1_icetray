#ifndef I3PARABOLOIDFITPARAMSCONVERTER_H_INCLUDED
#define I3PARABOLOIDFITPARAMSCONVERTER_H_INCLUDED

/**
 * copyright  (C) 2010
 * The Icecube Collaboration
 *
 * $Id$
 *
 * @version $Revision$
 * @date $LastChangedDate$
 * @author Fabian Kislat <fabian.kislat@desy.de> $LastChangedBy$
 */

#include <tableio/I3Converter.h>
#include <paraboloid/I3ParaboloidFitParams.h>

class I3ParaboloidFitParamsConverter : public I3ConverterImplementation<I3ParaboloidFitParams> {
private:
	I3TableRowDescriptionPtr CreateDescription(const I3ParaboloidFitParams &p);
	size_t FillRows(const I3ParaboloidFitParams &p, I3TableRowPtr rows);
};

#endif  // I3PARABOLOIDFITPARAMSCONVERTER_H_INCLUDED
