/**
 *
 * Definition of I3SplineRecoLikelihoodFactory
 *
 * $Id$
 *
 * Copyright (C) 2012
 * The IceCube Collaboration <http://www.icecube.wisc.edu>
 *
 * @file I3SplineRecoLikelihoodFactory.h
 * @version $Revision$
 * @date $Date$
 * @author Kai Schatto <KaiSchatto@gmx.de>
 * @brief This file contains the definition of the I3SplineRecoLikelihoodFactory module,
 *        which is a modification of Jake Feintzeig's splineReco module to perform basic spline reconstructions.
 *
 *        See https://wiki.icecube.wisc.edu/index.php/Improved_likelihood_reconstruction for more information.
 *
 *        This class, I3SplineRecoLikelihoodFactory, is the one with the interface to IceTray. Use this class as 
 *        service in your IceTray scripts, with the name of an I3PhotoSplineServiceFactory in the parameter `PhotonicsService'.
 *        Give the name of this service to your fitter, e.g. SimpleFitter, in parameter `LogLikelihood'.
 *        Look at the example scripts in the resources directory.
 *
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

#ifndef I3SPLINERECOLIKELIHOODFACTORY_H
#define I3SPLINERECOLIKELIHOODFACTORY_H

#include "icetray/I3ServiceFactory.h"
#include "spline-reco/I3SplineRecoLikelihood.h"

class I3SplineRecoLikelihoodFactory:public I3ServiceFactory
{
    public:
        I3SplineRecoLikelihoodFactory (const I3Context &);
        bool InstallService (I3Context &);
        void Configure ();
    private:
        I3SplineRecoLikelihoodPtr llh_;
};

#endif //I3SPLINERECOLIKELIHOODFACTORY
