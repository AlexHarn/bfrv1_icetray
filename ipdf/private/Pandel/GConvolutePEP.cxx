/**
    copyright  (C) 2004
    the icecube collaboration
    $Id$

    @version $Revision: 1.3 $
    @date $Date$
    @author S Robbins <robbins@physik.uni-wuppertal.de>
*/

#include "ipdf/Pandel/GConvolutePEP.h"

#include "ipdf/Utilities/IPdfException.h"
#include "ipdf/Utilities/IPdfConstants.h"
#include "ipdf/Utilities/PdfMath.h"
#include <cmath> // M_PI and friends

#include <gsl/gsl_integration.h> // for numrerical integration
#include <gsl/gsl_errno.h> // for error handling

using namespace IPDF::Pandel;
