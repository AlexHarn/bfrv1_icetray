#include <cmath>
#include <limits>

#include <gsl/gsl_errno.h>
#include <gsl/gsl_spline.h>

#include "ipdf/I3/I3MediumPropertiesFile.h"
#include "ipdf/I3/IceTableInterpolator.h"

const double IceTableInterpolator::RECO_WAVELENGTH = 380.;

IceTableInterpolator::IceTableInterpolator(const std::string& filepath){
    log_trace("going to open ice optical properties file %s",filepath.c_str());
    I3MediumPropertiesFile propfile(filepath);
    log_trace("going to read ice optical properties from %s",filepath.c_str());
    const std::vector<I3MediumPropertiesFile::Layer>& layers = propfile.Layers();
    size_t n = layers.size();
    absorptivity_.resize(n);
    inv_eff_scatt_length_.resize(n);
    z_.resize(n);
    log_trace("found %zu layers",n);
    azmin_ = NAN;
    azmax_ = NAN;
    szmin_ = NAN;
    szmax_ = NAN;
    zmin_ = +std::numeric_limits<double>::max();
    zmax_ = -std::numeric_limits<double>::max();
    // double* z = new double[n];
    // double* absorptivity = new double[n];
    // double* inv_scatt_length = new double[n];
    for (size_t i=0; i<n; ++i){
        log_trace("starting layer i=%zu",i);
        const I3MediumPropertiesFile::Layer& layer = layers[i];
        double zlower = layer.LowerEdge();
        double zupper = layer.UpperEdge();
        double zi = 0.5*( zlower + zupper );
        z_[i]=zi;
	int nwl = layer.AbsorptionCoefficents().Get().size();

        if (nwl<=1){
            log_fatal("Bad ice file: nwl=%d wave length bins for layer i=%zu???", nwl,i);
        }
	double minwl = layer.AbsorptionCoefficents().LowestWavelength();
	double maxwl = layer.AbsorptionCoefficents().HighestWavelength();
        log_debug("%2zu. depth %.1f-%.1f wavelength range: %d bins from %.1f till %.1f nm",
                i,zlower,zupper,nwl,minwl,maxwl);
	if ( std::isnan(minwl) || std::isnan(maxwl) || (RECO_WAVELENGTH < minwl) || (RECO_WAVELENGTH >= maxwl)){
            // this may have to be relaxed if it turns out that there exist ice files with 
            // optical properties for only exactly one wavelength (being the reco wavelength).
            log_fatal("Bad ice file: reco wavelength=%f nm not inside table range [%f-%f)",
                    RECO_WAVELENGTH,minwl,maxwl);
        }

        const double* a_array = &(layer.AbsorptionCoefficents().Get())[0]; // in C++11 we can use vector::data()
        const double* s_array = &(layer.ScatteringCoefficents().Get())[0]; // in C++11 we can use vector::data()
        double *wl = new double[nwl];
        for (int j=0; j<nwl; j++){
            wl[j] = minwl+j*(maxwl-minwl)/(nwl-1.);
            double a=a_array[j];
            double s=s_array[j];
            if (std::isnan(a) || std::isnan(s)){
                log_fatal("something is bad with %s: at z=%f, wl=%f, scat=%f and abs=%f",filepath.c_str(),zi,wl[j],s,a);
            }
        }
        gsl_interp_accel *wl_acc = gsl_interp_accel_alloc ();

        // get absorptivity at reco wavelength
        gsl_spline* wl_a_c_spline = gsl_spline_alloc (gsl_interp_cspline, nwl);
        gsl_spline_init (wl_a_c_spline, wl, a_array, nwl);
        absorptivity_[i] = gsl_spline_eval (wl_a_c_spline, RECO_WAVELENGTH, wl_acc);

        // get inv scatt length at reco wavelength
        gsl_spline* wl_s_c_spline = gsl_spline_alloc (gsl_interp_cspline, nwl);
        gsl_spline_init (wl_s_c_spline, wl, s_array, nwl);
        inv_eff_scatt_length_[i] = gsl_spline_eval (wl_s_c_spline, RECO_WAVELENGTH, wl_acc);

        log_debug("zrange=[%f,%f] absorption coeff = %f scatt coeff = %f",
                zlower,zupper,absorptivity_[i],inv_eff_scatt_length_[i]);

        if (std::isnan(absorptivity_[i]) || std::isnan(inv_eff_scatt_length_[i])){
            log_error("%s: zrange=[%f,%f] absorption coeff = %f scatt coeff = %f",
                    filepath.c_str(),zlower,zupper,absorptivity_[i],inv_eff_scatt_length_[i]);
            for (int j=0;j<nwl;j++){
                log_error("wl=%f a=%f s=%f",wl[j],a_array[j],s_array[j]);
            }
            log_fatal("BAD");
        }

        zmin_ = std::min(zi,zmin_);
        zmax_ = std::max(zi,zmax_);

        delete [] wl;
        gsl_spline_free (wl_a_c_spline);
        gsl_spline_free (wl_s_c_spline);
        gsl_interp_accel_free (wl_acc);

        log_trace("done with layer i=%zu",i);
    }

    // define absorptivity spline
    acc_ = gsl_interp_accel_alloc ();
    a_c_spline_ = gsl_spline_alloc (gsl_interp_cspline, n);
    gsl_spline_init (a_c_spline_, &z_[0], &absorptivity_[0], n);

    // define inverse scattering length spline
    s_c_spline_ = gsl_spline_alloc (gsl_interp_cspline, n);
    gsl_spline_init (s_c_spline_, &z_[0], &inv_eff_scatt_length_[0], n);

    azmin_ = gsl_spline_eval (a_c_spline_, zmin_, acc_);
    szmin_ = gsl_spline_eval (s_c_spline_, zmin_, acc_);
    azmax_ = gsl_spline_eval (a_c_spline_, zmax_, acc_);
    szmax_ = gsl_spline_eval (s_c_spline_, zmax_, acc_);

    log_debug("%s: zmin=%f absorption coeff = %f scatt coeff = %f", filepath.c_str(),zmin_,azmin_,szmin_);
    log_debug("%s: zmax=%f absorption coeff = %f scatt coeff = %f", filepath.c_str(),zmax_,azmax_,szmax_);

    if ( std::isnan(azmin_) ||
         std::isnan(azmax_) ||
         std::isnan(szmin_) ||
         std::isnan(szmax_) ){
        log_fatal("OOPS: at zmin=%f a=%f s=%f, at zmax=%f a=%f s=%f", zmin_, azmin_, szmin_, zmax_, azmax_, szmax_);
    }

    // delete [] z;
    // delete [] absorptivity;
    // delete [] inv_scatt_length;
    log_trace("successfully created an interpolation table.");
}

IceTableInterpolator::~IceTableInterpolator(){
    /* clean up GSL spline stuff */
    gsl_interp_accel_free (acc_);
    gsl_spline_free (a_c_spline_);
    gsl_spline_free (s_c_spline_);
}


double IceTableInterpolator::AbsorptionLength(double d){
    if (d<zmin_){
        return 1./azmin_;
    }
    if (d>zmax_){
        return 1./azmax_;
    }
    // hm. maybe we should check for zeros
    return 1./gsl_spline_eval(a_c_spline_,d,acc_);
}

double IceTableInterpolator::Absorptivity(double d){
    if (d<zmin_){
        return azmin_;
    }
    if (d>zmax_){
        return azmax_;
    }
    return gsl_spline_eval(a_c_spline_,d,acc_);
}

double IceTableInterpolator::EffScattLength(double d){
    if (d<zmin_){
        return 1./szmin_;
    }
    if (d>zmax_){
        return 1./szmax_;
    }
    return 1./gsl_spline_eval(s_c_spline_,d,acc_);
}

double IceTableInterpolator::InvEffScattLength(double d){
    if (d<zmin_){
        return szmin_;
    }
    if (d>zmax_){
        return szmax_;
    }
    return gsl_spline_eval(s_c_spline_,d,acc_);
}

double IceTableInterpolator::AveragedAbsorptionLength(double d1, double d2){
    return 1./AveragedAbsorptivity(d1,d2);
    /*
    if (d1>d2) std::swap(d1,d2);
    if (d2-d1<0.001){ // hmm.....
        return AveragedAbsorptionLength(0.5*(d1+d2));
    }
    if (d1>zmax_){
        return AveragedAbsorptionLength(zmax_);
    }
    if (d2<zmin_){
        return AveragedAbsorptionLength(zmin_);
    }
    if (d2>zmax_){
        return (d2-d1)/((d2-zmax_)*azmax_+  (zmax_-d1)*AveragedAbsorptivity(d1,zmax_));
    }
    if (d1<zmin_){
        return (d2-d1)/((zmin_-d1)*azmin_+  (d2-zmin_)*AveragedAbsorptivity(zmin_,d2));
    }
    return (d2-d1)/gsl_spline_eval_integ (a_c_spline_, d1, d2, acc_);
    */
}

double IceTableInterpolator::AveragedAbsorptivity(double d1, double d2){
    if (d1>d2) std::swap(d1,d2);
    if (d2-d1<0.001){ // hmm.....
        return Absorptivity(0.5*(d1+d2));
    }
    if (d1>zmax_){
        return Absorptivity(zmax_);
    }
    if (d2<zmin_){
        return Absorptivity(zmin_);
    }
    if (d2>zmax_){
        return ((d2-zmax_)*azmax_+  (zmax_-d1)*AveragedAbsorptivity(d1,zmax_))/(d2-d1);
    }
    if (d1<zmin_){
        return ((zmin_-d1)*azmin_+  (d2-zmin_)*AveragedAbsorptivity(zmin_,d2))/(d2-d1);
    }
    return gsl_spline_eval_integ (a_c_spline_, d1, d2, acc_)/(d2-d1);
}

double IceTableInterpolator::AveragedEffScattLength(double d1, double d2){
    return 1./AveragedInvEffScattLength(d1,d2);
}

double IceTableInterpolator::AveragedInvEffScattLength(double d1, double d2){
    if (d1>d2) std::swap(d1,d2);
    if (d2-d1<0.001){ // hmm.....
        return InvEffScattLength(0.5*(d1+d2));
    }
    if (d1>zmax_){
        return InvEffScattLength(zmax_);
    }
    if (d2<zmin_){
        return InvEffScattLength(zmin_);
    }
    if (d2>zmax_){
        return ((d2-zmax_)*szmax_+  (zmax_-d1)*AveragedInvEffScattLength(d1,zmax_))/(d2-d1);
    }
    if (d1<zmin_){
        return ((zmin_-d1)*szmin_+  (d2-zmin_)*AveragedInvEffScattLength(zmin_,d2))/(d2-d1);
    }
    return gsl_spline_eval_integ (s_c_spline_, d1, d2, acc_)/(d2-d1);
}
