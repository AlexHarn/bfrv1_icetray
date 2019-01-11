
"""
file: CalibrationDefaults.py
Contains magic numbers for calibration objects.  These numbers were
never measured/verified for any particular object, but they are
assumed to be reasonable default values in the case more accurate
numbers are not contained in the database.
"""

from icecube import dataclasses

# Droop parameters for default old-toroid DOM
oldToroidTauParam = dataclasses.TauParam()
oldToroidTauParam.p0 = 400
oldToroidTauParam.p1 = 5000
oldToroidTauParam.p2 = 16
oldToroidTauParam.p3 = 400
oldToroidTauParam.p4 = 5000
oldToroidTauParam.p5 = 16
oldToroidTauParam.tau_frac = -3.3

# Droop parameters for default new-toroid DOM
newToroidTauParam = dataclasses.TauParam()
newToroidTauParam.p0 = 10960
newToroidTauParam.p1 = 56665
newToroidTauParam.p2 = 6.5
newToroidTauParam.p3 = 500
newToroidTauParam.p4 = 0
newToroidTauParam.p5 = 1
newToroidTauParam.tau_frac = -0.5

# Quantum efficiency for HQE R7081-100 PMT relative to R7081
R7081_100_Efficiency = 1.35