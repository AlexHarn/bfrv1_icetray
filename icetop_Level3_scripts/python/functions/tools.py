from icecube import icetray, dataclasses

def pulse_is_saturated(gcd, om_key, reco_pulse):
    """
    Check if a pulse is saturated.

    The threshold is automatically set depending on the gain type and the DOM calibration.
    Low gain saturation is nominally set to 90000 PEs while low gain saturation is set to
    the hglgCrossOver value in the calibration (it defaults to 3000 PEs if this fails).

    Parameters:
     - gcd: an object with an dataclasses.I3Calibration data member called calibration.
     - om_key: and object of type icetray.OMKey
     - reco_pulse: object of type dataclasses.I3RecoPulse

    Returns: Bool
    """
    from icecube.dataclasses import I3DOMStatus

    # nominal thresholds (magic numbers)
    SAT_LG = 90000.
    SAT_HG = 3000.

    gain = gcd.detector_status.dom_status[om_key].dom_gain_type
    vemCalib = gcd.calibration.vem_cal[om_key]
    pe_per_vem = vemCalib.pe_per_vem/vemCalib.corr_factor

    if gain == I3DOMStatus.High:
        threshold = vemCalib.hglg_cross_over/pe_per_vem
        if threshold <= 0:
            threshold = SAT_HG/pe_per_vem
    elif gain == I3DOMStatus.Low:
        threshold = SAT_LG/pe_per_vem
    else:
        raise Exception('Unknown gain type: %s'%gain)

    return reco_pulse.GetCharge() > threshold


def to_shower_cs(fit):
    """
    Rotate to shower CS takes a fit (assumes fit.dir is set) and returns a rotation matrix.
    Requires numpy.
    """
    import numpy
    from math import cos, sin
    # counter-clockwise (pi + phi) rotation
    d_phi = numpy.matrix([ [ -cos(fit.dir.phi), -sin(fit.dir.phi), 0],
                           [  sin(fit.dir.phi), -cos(fit.dir.phi), 0],
                           [  0,                 0,                1] ])
    # clock-wise (pi - theta) rotation
    d_theta = numpy.matrix([ [  -cos(fit.dir.theta), 0, -sin(fit.dir.theta)],
                             [  0,                  1,  0,                ],
                             [  sin(fit.dir.theta), 0,  -cos(fit.dir.theta)] ])
    return d_theta*d_phi
