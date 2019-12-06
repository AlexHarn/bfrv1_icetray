from I3Tray import *
from icecube import icetray, dataclasses, dataio
from icecube.icetray import I3Frame
from icecube.icetray import traysegment
import os
import os.path
from os.path import expandvars


@traysegment
def PPCTraySegment(tray,
                   name,
                   UnshadowedFraction=1.,
                   DOMOversizeFactor=1,
                   gpulib="opencl",
                   volumecyl=True,
                   IceModelLocation=expandvars("$I3_SRC/ice-models/resources/models"),
                   keep_empty_events=False,
                   HoleIceParameterization=expandvars("$I3_SRC/ice-models/resources/models/angsens/as.h2-50cm"),
                   IceModel="Spice3.2.1",
                   InputMCTree="I3MCTree",
                   MCPESeriesName="I3MCPESeriesMap",
                   UseGPUs=True,
                   GPU=-1,
                   tempdir=None):
    """
    PPC Photon Propagation Code TraySegment (supports CUDA/OpenCL)
    """

    ppcIceModel = None
    if IceModelLocation is None:
        IceModelLocation = expandvars("$I3_SRC/ice-models/resources/models")

    if IceModel == "SpiceMie":
        ppcIceModel = expandvars(IceModelLocation + "/spice_mie")
    elif IceModel == "SpiceLea":
        ppcIceModel = expandvars(IceModelLocation + "/spice_lea")
    elif IceModel == "Spice3":
        ppcIceModel = expandvars(IceModelLocation + "/spice_3")
    elif IceModel == "Spice3.1":
        ppcIceModel = expandvars(IceModelLocation + "/spice_3.1")
    elif IceModel == "Spice3.2":
        ppcIceModel = expandvars(IceModelLocation + "/spice_3.2")
    elif IceModel == "Spice3.2.1":
        ppcIceModel = expandvars(IceModelLocation + "/spice_3.2.1")
    elif os.path.exists(expandvars(IceModelLocation + "/" + IceModel)):
        ppcIceModel = expandvars(IceModelLocation + "/" + IceModel)
    else:
        raise RuntimeError("Unknown ice model: %s", IceModel)

    os.putenv("ICEMODELDIR", ppcIceModel)
    os.putenv("PPCHOLEICE", HoleIceParameterization)

    if UseGPUs:
        os.putenv("OGPU", "1")
    else:
        os.putenv("OCPU", "1")
    if GPU >= 0 and UseGPUs:
        os.putenv("CUDA_VISIBLE_DEVICES", str(GPU))
        os.putenv("COMPUTE", ":0." + str(GPU))
        os.putenv("GPU_DEVICE_ORDINAL", str(GPU))

    tray.AddModule("i3ppc", "ppc",
                   If=lambda f: f[InputMCTree].size() or keep_empty_events,
                   gpu=GPU,
                   efficiency_scaling_factor=UnshadowedFraction,
                   cyl=volumecyl,
                   keep=keep_empty_events,
                   MCTree=InputMCTree)

    # PPC does not have an option for setting the name of the PE map.
    # If the default name of PE map changes, this needs to be updated.

    def add_empty_pes(f, MCPESeriesName="MCPESeriesMap"):
        if MCPESeriesName not in f:
            f[MCPESeriesName] = I3MCPESeriesMap()
    tray.Add(add_empty_pes, MCPESeriesName="MCPESeriesMap", streams=[icetray.I3Frame.DAQ])
    if MCPESeriesName != "MCPESeriesMap":
        tray.AddModule("Rename", keys=["MCPESeriesMap", MCPESeriesName])

