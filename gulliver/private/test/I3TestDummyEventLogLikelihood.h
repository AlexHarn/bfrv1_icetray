#ifndef GULLIVER_I3TESTDUMMYEVENTLOGLIKELIHOOD_H_INCLUDED
#define GULLIVER_I3TESTDUMMYEVENTLOGLIKELIHOOD_H_INCLUDED
#include <cassert>
#include <cmath>
#include <iostream>

#include <gulliver/I3EventHypothesis.h>
#include <gulliver/I3EventLogLikelihoodBase.h>

//
// the likelihood for our dummy track (kind of 2D Gauss)
//
class I3TestDummyEventLogLikelihood : public I3EventLogLikelihoodBase {

    private:

        // truth variables
        const double trueL_;
        const double trueAzi_;
        const double trueX_;

        // scales for penalizing deviations from the truth
        const double scaleL_;
        const double scaleAzi_;
        const double scaleX_;

        // caching variables
        double L_;
        double Azi_;
        double X_;
        double dL_;
        double dAzi_;
        double dX_;

        // for log messages
        const std::string name_;

    public:

        /// constructor
        I3TestDummyEventLogLikelihood(double trueL, double trueAzi, double trueX, double scaleL, double scaleAzi, double scaleX ):
            I3EventLogLikelihoodBase(),
            trueL_(trueL), trueAzi_(trueAzi), trueX_(trueX),
            scaleL_(scaleL), scaleAzi_(scaleAzi), scaleX_(scaleX),
            L_(0.), Azi_(0.), X_(0.), dL_(0.), dAzi_(0.), dX_(0.),
            name_("DummyLLH"){}

        /// destructor
        ~I3TestDummyEventLogLikelihood(){}

        void SetGeometry( const I3Geometry& geo)  {};

        /// provide full event info (dummy: just ignore the event... :o)
        void SetEvent( const I3Frame &f ){}

        /// get the answer
        double GetLogLikelihood( const I3EventHypothesis &eh){

            I3Particle &p = *(eh.particle);

            L_ = p.GetLength();
            Azi_ = p.GetAzimuth();
            X_ = p.GetPos().GetX();

            dL_ = (L_ - trueL_) / scaleL_;
            dAzi_ = (fmod(Azi_ - trueAzi_+M_PI,2*M_PI) - M_PI) / scaleAzi_;
            dX_ = (X_ - trueX_) / scaleX_;

            // return exp(- dL_*dL_ - dAzi_*dAzi_ - dX_*dX_);
            return (- dL_*dL_ - dAzi_*dAzi_ - dX_*dX_);
        }

        /// get the answer plus the gradient
        double GetLogLikelihoodWithGradient( const I3EventHypothesis &p, I3EventHypothesis &gradient, double weight=1 ){
            // XXX: note that this module does not respect weight!
            double LogL = this->GetLogLikelihood(p);

            if ( gradient.particle->GetPos().GetX() > 0 ){
                // gradient.particle->SetPos( -2*X_*LogL, 0., 0. );
                gradient.particle->SetPos( -2*dX_/scaleX_, 0., 0. );
            }
            if ( gradient.particle->GetDir().GetAzimuth() > 0 ){
                // gradient.particle->SetDir( 0., -2*Azi_*LogL );
                gradient.particle->SetDir( 0., -2*dAzi_/scaleAzi_ );
            }
            if ( gradient.particle->GetLength() ){
                // gradient.particle->SetLength( -2*L_*LogL );
                gradient.particle->SetLength( -2*dL_/scaleL_ );
            }

            return LogL;
        }

        /// yep, we gots gradientses
        bool HasGradient(){
            return true;
        }

        /**
         * Get multiplicity
         * Not well defined for this dummy case, usually you'd take something
         * like the number of (selected) hits.
         */
        unsigned int GetMultiplicity() { return 42;}

        const std::string GetName() const { return name_; }
};
typedef boost::shared_ptr<I3TestDummyEventLogLikelihood> I3TestDummyEventLogLikelihoodPtr;
#endif /* GULLIVER_I3TESTDUMMYEVENTLOGLIKELIHOOD_H_INCLUDED */
