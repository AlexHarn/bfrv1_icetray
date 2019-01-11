from icecube.ddddr import I3MuonEnergyParams, I3MuonEnergyCascadeParams
from icecube import tableio

class I3MuonEnergyCascadeParamsConverter(tableio.I3Converter):
    booked = I3MuonEnergyCascadeParams
    def CreateDescription(self, meparams):
        desc = tableio.I3TableRowDescription()
        desc.add_field("nDOMsCascade",tableio.types.Float64, "", 
                       "Number of DOMs available for the leading cascade energy reconstruction.")
        desc.add_field("cascade_energy",tableio.types.Float64,"",
                       "Reconstructed leading cascade energy.")
        desc.add_field("cascade_energy_sigma",tableio.types.Float64, "", 
                       "Statistical error on the reconstructed leading cascade energy.")
        desc.add_field("cascade_position_x",tableio.types.Float64,"",
                       "X coordinate of the position of the leading cascade.")
        desc.add_field("cascade_position_y",tableio.types.Float64,"",
                       "Y coordinate of the position of the leading cascade.")
        desc.add_field("cascade_position_z",tableio.types.Float64,"",
                       "Z coordinate of the position of the leading cascade.")
        desc.add_field("cascade_slant_depth",tableio.types.Float64, "", 
                       "Slant depth of the leading cascade.")

        return desc

    def FillRows(self, meparams, rows):
        rows["nDOMsCascade"] = meparams.nDOMsCascade
        rows["cascade_energy"] = meparams.cascade_energy
        rows["cascade_energy_sigma"] = meparams.cascade_energy_sigma
        rows["cascade_position_x"] = meparams.cascade_position.x
        rows["cascade_position_y"] = meparams.cascade_position.y
        rows["cascade_position_z"] = meparams.cascade_position.z
        rows["cascade_slant_depth"] = meparams.cascade_slant_depth
        return 1


class I3MuonEnergyParamsConverter(tableio.I3Converter):
    booked = I3MuonEnergyParams
    def CreateDescription(self, meparams):
        desc = tableio.I3TableRowDescription()
        desc.add_field("N",tableio.types.Float64, "", 
                       "Normalization of Exp. or TomF function")
        desc.add_field("N_err",tableio.types.Float64,"",
                       "Uncertainty of normalization N")
        desc.add_field("b",tableio.types.Float64, "", 
                       "Slope of Exp. or TomF function")
        desc.add_field("b_err",tableio.types.Float64,"",
                       "Uncertainty of slope b")
        desc.add_field("gamma",tableio.types.Float64, "", 
                       "Exponent of TomF function")
        desc.add_field("gamma_err",tableio.types.Float64,"",
                       "Uncertainty of exponent gamma")
        desc.add_field("nDOMs", tableio.types.Float64,"",
                       "Number of DOMs used for energy loss reconstr.")
        desc.add_field("rllh", tableio.types.Float64,"",
                       "Rllh of the fit")
        desc.add_field("chi2", tableio.types.Float64,"",
                       "Chi square of the fit")
        desc.add_field("chi2ndof", tableio.types.Float64,"",
                       "Chi square per deg of freedom")
        desc.add_field("peak_energy", tableio.types.Float64, "GeV",
                       "Peak energy of the eloss dist.")
        desc.add_field("peak_sigma", tableio.types.Float64, "GeV",
                       "Uncertainty of the peak_energy")
        desc.add_field("mean", tableio.types.Float64, "GeV",
                       "Mean energy loss")
        desc.add_field("median", tableio.types.Float64, "GeV",
                       "Median energy loss")
        desc.add_field("bin_width", tableio.types.Float64, "",
                       "Bin width of the eloss dist.")

        return desc

    def FillRows(self, meparams, rows):
        rows["N"] = meparams.N
        rows["N_err"] = meparams.N_err
        rows["b"] = meparams.b
        rows["b_err"] = meparams.b_err
        rows["gamma"] = meparams.gamma
        rows["gamma_err"] = meparams.gamma_err
        rows["nDOMs"] = meparams.nDOMs
        rows["rllh"] = meparams.rllh
        rows["chi2"] = meparams.chi2
        rows["chi2ndof"] = meparams.chi2ndof
        rows["peak_energy"] = meparams.peak_energy
        rows["peak_sigma"] = meparams.peak_sigma
        rows["mean"] = meparams.mean
        rows["median"] = meparams.median
        rows["bin_width"] = meparams.bin_width
        return 1

tableio.I3ConverterRegistry.register(I3MuonEnergyParamsConverter)
tableio.I3ConverterRegistry.register(I3MuonEnergyCascadeParamsConverter)
