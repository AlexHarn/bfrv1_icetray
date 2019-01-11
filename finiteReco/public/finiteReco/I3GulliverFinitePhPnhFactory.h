/**
 * @brief declaration of the I3GulliverFinitePhPnhFactory class
 *
 * @file I3GulliverFinitePhPnhFactory.h
 * @version $Revision$
 * @date $Date$
 * @author Sebastian Euler <sebastian.euler@physikrwth-aachen.de>
 *
 * This class is an I3ServiceFactory. It places a pointer to I3GulliverFinitePhPnh in the frame, which is an I3PDFService used within the gulliver project. (see <https://wiki.icecube.wisc.edu/index.php/FiniteReco.CalculationOfProbabilities>)
 */

#ifndef I3GULLIVERFINITEPHPNHFACTORY_H_INCLUDED
#define I3GULLIVERFINITEPHPNHFACTORY_H_INCLUDED

#include <string>

#include "finiteReco/I3GulliverFinitePhPnh.h"
#include "icetray/IcetrayFwd.h"
#include "icetray/I3ServiceFactory.h"
#include "gulliver/I3MinimizerBase.h"


class I3GulliverFinitePhPnhFactory : public I3ServiceFactory {
public:

    // Constructors and destructor

    I3GulliverFinitePhPnhFactory(const I3Context& context);

    ~I3GulliverFinitePhPnhFactory();

    // public member functions

    /**
     * Install this service into the specified object.
     *
     * @param services the I3Services into which the service should be installed.
     * @return true if the service is successfully installed.
     */
    virtual bool InstallService(I3Context& services);

    /**
     * Configure service prior to installing it. 
     */
    virtual void Configure();

   private:

    // private constructors, destructor and assignment

    // stop defaults
    I3GulliverFinitePhPnhFactory( const I3GulliverFinitePhPnhFactory& rhs);
    I3GulliverFinitePhPnhFactory operator= (const I3GulliverFinitePhPnhFactory& rhs);

    // options
    std::string useSignalsFrom_;
    std::string flagStringLLH_;
    std::string namePhotorec_;
    std::string probName_;
    std::string inputProbFile_;
    double noiseRate_;
    double defaultEventDuration_;
    double absorptionLength_;
    double rCylinder_;
    double probMultiDet_;
    double finiteDefaultLength_;
    std::vector<int> selectedStrings_;
    bool onlyInfiniteTables_;
    bool useOnlyFirstHit_;

    //instantiation data
    I3GulliverFinitePhPnhPtr finitePhPnh_;
    std::string name_;
    SET_LOGGER("I3GulliverFinitePhPnhFactory");
};

#endif /* I3GULLIVERFINITEPHPNHFACTORY_H_INCLUDED */
