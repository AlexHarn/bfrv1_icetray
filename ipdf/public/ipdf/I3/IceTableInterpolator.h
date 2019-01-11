#ifndef __ICETABLEINTERPOLATOR_H_INCLUED__
#define __ICETABLEINTERPOLATOR_H_INCLUED__

#include <gsl/gsl_errno.h>
#include <gsl/gsl_spline.h>
#include <string>
#include <vector>

#include "icetray/I3Logging.h"

/*
 // these forward declarations do not really work
typedef gsl_interp_accel* gsl_interp_accel_ptr;
typedef gsl_spline* gsl_spline_ptr;
// neither do these
struct gsl_interp_accel;
struct gsl_spline;
*/

class IceTableInterpolator {
    public:
        static const double RECO_WAVELENGTH;
        IceTableInterpolator(const std::string& filepath);
        ~IceTableInterpolator();

        double AbsorptionLength(double d);
        double Absorptivity(double d1);
        double EffScattLength(double d);
        double InvEffScattLength(double d);

        double AveragedAbsorptionLength(double d1,double d2);
        double AveragedAbsorptivity(double d1,double d2);
        double AveragedEffScattLength(double d1,double d2);
        double AveragedInvEffScattLength(double d1,double d2);

        const std::vector<double>& GetZLower() const { return zlower_; }
        const std::vector<double>& GetZUpper() const { return zupper_; }
        const std::vector<double>& GetZ() const { return z_; }
        const std::vector<double>& GetAbsorptivities() const { return absorptivity_; }
        const std::vector<double>& GetInvEffScattLengths() const { return inv_eff_scatt_length_; }

    private:
        /// inhibit default constructor
        // IceTableInterpolator();
        /// GSL accelerator for interpolation
        gsl_interp_accel *acc_;
        /// GSL spline for absorptivity interpolation
        gsl_spline *a_c_spline_;
        /// GSL spline for inverse scattering length interpolation
        gsl_spline *s_c_spline_;

        std::vector<double> zlower_;
        std::vector<double> zupper_;
        std::vector<double> z_;
        std::vector<double> absorptivity_;
        std::vector<double> inv_eff_scatt_length_;

        // overflow stuff
        double zmin_;
        double azmin_;
        double szmin_;
        double zmax_;
        double azmax_;
        double szmax_;
    SET_LOGGER("LayeredIce");
};

#endif /* __ICETABLEINTERPOLATOR_H_INCLUED__ */
