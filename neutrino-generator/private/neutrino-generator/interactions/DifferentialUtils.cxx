/**
 *   copyright  (C) 2005
 *   the IceCube Collaboration
 *   $Id:  $
 *
 *   @version $Revision: $
 *   @date $Date: $
 *   @author Kotoyo Hoshina <hoshina@icecube.wisc.edu>
 *
 *   @brief utility class for intarction classes using 
 *    differential cross sections
 * 
 */

#include "neutrino-generator/interactions/DifferentialUtils.h"
#include "neutrino-generator/utils/Constants.h"
#include <vector>

namespace nugen {
namespace DifferentialUtils {

using namespace Constants;

//____________________________________________________________________
void InitializeI3CrossSectionTables(const std::string &differential, 
                       const std::string& total, I3CrossSection &xsec)
{
  //log_debug("InitializeI3CrossSectionTable is called");
  xsec.load(differential, total);
}

//___________________________________________________________________
std::vector<double> SelectXY(double log_e, I3Particle::ParticleType ptype, 
                              I3CrossSection &xsec,
                              I3RandomServicePtr random) 
{
  I3CrossSection::finalStateRecord final 
              = xsec.sampleFinalState(std::pow(10, log_e), ptype, random);

  std::vector<double> result;
  result.push_back(final.x);
  result.push_back(final.y);
  //log_trace("final state x and y = (%f, %f)", final.x, final.y);
  return result;
}

//____________________________________________________________________
double GetXsecCGS(const double energy, I3CrossSection &xsec) 
{
    // check range first.
    // GetXsecCGS returns 0 if energy is out of boundary.
    const splinetable& spt = xsec.getTotalCrossSection();
    double log10e = log10(energy);
    if (log10e < spt.extents[0][0] || log10e > spt.extents[0][1]) {
       return 0;
    }

    // this is old!!
    // The unit of returned cross section is eV^-2
    // 1eV^-1 = hbar*c / 1eV [m]
    //double crosssec = xsec.evaluateTotalCrossSection(energy);
    // convert unit eV^-2 to cm^2
    //double crosssec_in_cm2 = Constants::EVm2toCM2(crosssec);
    //this is old too
    // The unit of returned cross section [cm^2].
    //double crosssec_in_cm2 = xsec.evaluateTotalCrossSection(energy);

    // this is new, on Dec. 5 2017
    // The unit of returned cross section from I3CrossSection is [m^2].
    // The NuGen internal unit is cm^2.
    double crosssec_in_cm2 = 1e+4 * xsec.evaluateTotalCrossSection(energy);

   //log_debug("cross section for energy log10 %g is %g [cm2])", log10(energy), crosssec_in_cm2);
    return crosssec_in_cm2;

}

} // DifferentialUtils
} // nugen


