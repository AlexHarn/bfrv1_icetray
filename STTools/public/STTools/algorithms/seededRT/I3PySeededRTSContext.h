/**
 * Copyright (C) 2013
 * Martin Wolf <martin.wolf@icecube.wisc.edu>
 * and the IceCube Collaboration <http://www.icecube.wisc.edu>
 *
 * @version $Id$
 * @file STTools/public/STTools/algorithms/seededRT/I3PySeededRTSContext.h
 * @date $Date$
 * @brief This file contains the definition of the
 *        I3PySeededRTSContext_interface class.
 *        It is a boost::python visitor class derived from the
 *        I3PySContext_interface template, that helps to
 *        expose the I3SeededRTSContext C++ class to Python.
 *
 *        ----------------------------------------------------------------------
 *        This file is free software; you can redistribute it and/or modify
 *        it under the terms of the GNU General Public License as published by
 *        the Free Software Foundation; either version 3 of the License, or
 *        (at your option) any later version.
 *
 *        This program is distributed in the hope that it will be useful,
 *        but WITHOUT ANY WARRANTY; without even the implied warranty of
 *        MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *        GNU General Public License for more details.
 *
 *        You should have received a copy of the GNU General Public License
 *        along with this program.  If not, see <http://www.gnu.org/licenses/>
 */
#ifndef STTOOLS_ALGORITHMS_SEEDEDRT_I3PYSEEDEDRTSCONTEXT_H_INCLUDED
#define STTOOLS_ALGORITHMS_SEEDEDRT_I3PYSEEDEDRTSCONTEXT_H_INCLUDED

#include "STTools/I3PySContext.h"

#include "STTools/algorithms/seededRT/I3SeededRTSData.h"
#include "STTools/algorithms/seededRT/I3SeededRTSContext.h"

namespace sttools {
namespace seededRT {

class I3PySeededRTSContext_interface
  : public I3PySContext_interface<
                I3SeededRTSData,
                I3SeededRTSContext,
                I3PySeededRTSContext_interface >
{
  private:
    typedef I3PySContext_interface<
                I3SeededRTSData,
                I3SeededRTSContext,
                I3PySeededRTSContext_interface >
            I3PySContext_interface_t;
  public:
    template <class classT>
    void
    visit(classT &cls) const
    {
        // Invoke the visit method of the base class.
        I3PySContext_interface_t::visit(cls);

        cls.add_property("max_dustlayer_correction_length",
            &I3SeededRTSContext::GetMaxDustlayerCorrectionLength,
            &I3SeededRTSContext::SetMaxDustlayerCorrectionLength,
            "The maximal calculated dust layer correction length for the spatial   \n"
            "data map.");
    }
};

}// namespace seededRT
}// namespace sttools

#endif//STTOOLS_ALGORITHMS_SEEDEDRT_I3PYSEEDEDRTSCONTEXT_H_INCLUDED
