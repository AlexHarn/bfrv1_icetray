/**
 *  copyright  (C) 2004
 *  the icecube collaboration
 *  $Id: I3LogLikelihoodFitParams.cxx 38584 2007-11-09 18:53:37Z boersma $
 *
 *  @version $Revision: 38584 $
 *  @date $Date: 2007-11-09 13:53:37 -0500 (Fri, 09 Nov 2007) $
 *  @author David Boersma <boersma@icecube.wisc.edu>
 *
 */

#include <tableio/converter/pybindings.h>
#include <gulliver/I3LogLikelihoodFitParams.h>
#include <icetray/python/dataclass_suite.hpp>

#include "../gulliver/converter/I3LogLikelihoodFitParamsConverter.h"

using namespace boost::python;

void register_I3LogLikelihoodFitParams()
{

  class_<I3LogLikelihoodFitParams, bases<I3FrameObject>, I3LogLikelihoodFitParamsPtr>("I3LogLikelihoodFitParams")
    .def_readwrite("logl", &I3LogLikelihoodFitParams::logl_)
    .def_readwrite("rlogl", &I3LogLikelihoodFitParams::rlogl_)
    .def_readwrite("ndof", &I3LogLikelihoodFitParams::ndof_)
    .def_readwrite("nmini", &I3LogLikelihoodFitParams::nmini_)
    .def("reset", &I3LogLikelihoodFitParams::Reset, "Sets all values to default")
    .def(dataclass_suite<I3LogLikelihoodFitParams>())
    ;

  register_pointer_conversions<I3LogLikelihoodFitParams>();

  class_<I3Vector<I3LogLikelihoodFitParams>, bases<I3FrameObject>,
    boost::shared_ptr<I3Vector<I3LogLikelihoodFitParams> > >("I3VectorI3LogLikelihoodFitParams")
        .def(dataclass_suite<I3Vector<I3LogLikelihoodFitParams> >())
      ;

  I3CONVERTER_NAMESPACE(gulliver);
  I3CONVERTER_EXPORT_DEFAULT(I3LogLikelihoodFitParamsConverter,
                 "Dumps all fit parameter objects from  gulliver reconstructions");
}
