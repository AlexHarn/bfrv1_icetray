#!/usr/bin/env python
#
# output debug info of Eout / Ein at grashow resonance.
# To plot the debug info, type
# python draw_GR_y.py
#

from I3Tray import *
from icecube import icetray, dataclasses, phys_services, sim_services, dataio, earthmodel_service, neutrino_generator

#
# for logging
#
icetray.I3Logger.global_logger = icetray.I3PrintfLogger()
icetray.set_log_level(icetray.I3LogLevel.LOG_WARN)
#icetray.set_log_level_for_unit("EarthModelService",icetray.I3LogLevel.LOG_TRACE)
#icetray.set_log_level_for_unit("I3NuG",icetray.I3LogLevel.LOG_TRACE)
icetray.set_log_level_for_unit("I3NuG",icetray.I3LogLevel.LOG_INFO)


# generate random service
random = phys_services.I3GSLRandomService(42)


# generate earthmodel service
earthmodel = ["PREM_mmc"]
materialmodel = ["Standard"]
earth = earthmodel_service.EarthModelService("EarthModelService","",
                              earthmodel, materialmodel, 
                              "IceSheet",
                              20.*I3Units.degree, 1948*I3Units.m)

steer = neutrino_generator.Steering(earth)

# generate interaction service
interactionGR = neutrino_generator.InteractionGR(random, steer)

# generate 10000events per energy bin
interactionGR.debug_print("GRdebug.txt", 10000)


elogmin = 2
elogmax = 8
nebins = 100.
de = (elogmax - elogmin) /nebins

import numpy as np
elogs = np.linspace(elogmin, elogmax, nebins)


for elog in elogs :
    xsec = interactionGR.xsec_cgs(10**elog)
    print("elog %f, xsec %g" % (elog, xsec))
    





