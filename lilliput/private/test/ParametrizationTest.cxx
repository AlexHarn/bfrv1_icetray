
/**
    copyright  (C) 2006
    the icecube collaboration
    $Id$

    @version $Revision$
    @date $Date$
    @author boersma
*/

#include <I3Test.h>

#include "icetray/I3Logging.h"
#include <string>
#include <sstream>
#include <vector>

#include "lilliput/parametrization/I3SimpleParametrization.h"
#include "lilliput/minimizer/I3GSLSimplex.h"
#include "gulliver/I3Gulliver.h"
#include "gulliver/I3EventLogLikelihoodBase.h"
#include "dataclasses/I3Position.h"
#include "icetray/I3Units.h"

TEST_GROUP(Parametrization);

const std::vector<double> nobounds(2,0.);
class TestParMaker{
    private:
        std::vector<double> steps_;
        std::vector< vector<double> > absbounds_;
        std::vector< vector<double> > relbounds_;
        int mode_;

    public:
        // constructor
        TestParMaker(){
            Reset();
        }

        // set (back) to defaults
        void Reset(){
            steps_.clear();
            steps_.resize(I3SimpleParametrization::PAR_N,0.);
            absbounds_.clear();
            absbounds_.resize(I3SimpleParametrization::PAR_N,nobounds);
            relbounds_.clear();
            relbounds_.resize(I3SimpleParametrization::PAR_N,nobounds);
            mode_ = I3SimpleParametrization::VERTEX_Default;
        }


        // generic setter
        void Set( int i, double step,
                  double absmin=0., double absmax=0., double relmin=0., double relmax=0. ){
            // maybe insert a paranoid check on the value of i
            log_trace("i=%d step=%f steps_.size()=%zu",i,step,steps_.size());
            steps_[i] = step;
            absbounds_[i][0] = absmin;
            absbounds_[i][1] = absmax;
            relbounds_[i][0] = relmin;
            relbounds_[i][1] = relmax;
        }

        // setter for xyz
        void SetXYZ( double step,
                     double absmin=0., double absmax=0., double relmin=0., double relmax=0. ){
            for (int i=I3SimpleParametrization::PAR_X;
                     i<=I3SimpleParametrization::PAR_Z; ++i ){
                Set(i,step,absmin,absmax,relmin,relmax);
            }
        }

        // setter for L
        void SetL( double step,
                   double absmin=0., double absmax=0., double relmin=0., double relmax=0.,
                   bool isLog = true ){
            int i = isLog ? I3SimpleParametrization::PAR_LogL
                          : I3SimpleParametrization::PAR_LinL;
            int other = isLog ? I3SimpleParametrization::PAR_LinL
                              : I3SimpleParametrization::PAR_LogL;
            Set(i,step,absmin,absmax,relmin,relmax);
            Set(other,0.,0.,0.,0.,0.);
        }

        // setter for E
        void SetE( double step,
                   double absmin=0., double absmax=0., double relmin=0., double relmax=0.,
                   bool isLog = true ){
            int i = isLog ? I3SimpleParametrization::PAR_LogE
                          : I3SimpleParametrization::PAR_LinE;
            int other = isLog ? I3SimpleParametrization::PAR_LinE
                              : I3SimpleParametrization::PAR_LogE;
            Set(i,step,absmin,absmax,relmin,relmax);
            Set(other,0.,0.,0.,0.,0.);
        }

        // setter for direction
        void SetDir( double step,
                     double absmin=0., double absmax=0., double relmin=0., double relmax=0.){
            for (int i=I3SimpleParametrization::PAR_Zen;
                     i<=I3SimpleParametrization::PAR_Azi; ++i ){
                // log_trace("i=%d step=%f",i,step);
                Set(i,step,absmin,absmax,relmin,relmax);
            }
        }

        // setter for time
        void SetTime( double step,
                      double absmin=0., double absmax=0., double relmin=0., double relmax=0.){
            int i = I3SimpleParametrization::PAR_T;
            Set(i,step,absmin,absmax,relmin,relmax);
        }

        // setter for speed
        void SetSpeed( double step,
                      double absmin=0., double absmax=0., double relmin=0., double relmax=0.){
            int i = I3SimpleParametrization::PAR_Speed;
            Set(i,step,absmin,absmax,relmin,relmax);
        }

        // setter for mode (only relevant for contained tracks): stop/start point or default
        void SetMode( int mode ){ mode_ = mode; }

        // produce a service object
        I3SimpleParametrizationPtr MakeNewServicePtr(std::string name){
            std::vector<double> stepscopy = steps_;
            std::vector< std::vector<double> > abscopy = absbounds_;
            std::vector< std::vector<double> > relcopy = relbounds_;
            // need to use copies, because the I3SimpleParametrization constructors
            // loots the vectors with a swap().
            I3SimpleParametrizationPtr newpar (
                new I3SimpleParametrization( name, stepscopy, abscopy, relcopy, mode_));
            return newpar;
        }

};


// convenience class to create a I3SimpleParametrization service 
// (maybe good to provide a similar shorthand constructor in I3SimpleParametrization)

TEST(AnyTrack)
{
    TestParMaker tpm;
    tpm.SetTime(30.*I3Units::ns);
    tpm.SetXYZ(20.*I3Units::m);
    tpm.SetDir(0.2*I3Units::radian);
    tpm.SetL(0.2,0.,0.,0.,0.,true/*log*/);
    tpm.SetE(0.2,0.,0.,0.,0.,true/*log*/);
    double c = I3Constants::c;
    tpm.SetSpeed(0.01*c,0.5*c,1.5*c);
    I3SimpleParametrizationPtr testpar = tpm.MakeNewServicePtr("inftrack");

    double testvalues[I3SimpleParametrization::PAR_N];
    double &t    = testvalues[I3SimpleParametrization::PAR_T]    = 1234567.89*I3Units::ns;
    double &x    = testvalues[I3SimpleParametrization::PAR_X]    = 123456.789*I3Units::m;
    double &y    = testvalues[I3SimpleParametrization::PAR_Y]    = 12345.6789*I3Units::m;
    double &z    = testvalues[I3SimpleParametrization::PAR_Z]    = 1234.56789*I3Units::m;
    double &zen  = testvalues[I3SimpleParametrization::PAR_Zen]  = 123.456789*I3Units::degree;
    double &azi  = testvalues[I3SimpleParametrization::PAR_Azi]  = 12.3456789*I3Units::degree;
    double &E    = testvalues[I3SimpleParametrization::PAR_LinE] = 1.23456789*I3Units::PeV;
    /*double &LogE =*/ testvalues[I3SimpleParametrization::PAR_LogE] = log10(E);
    double &L    = testvalues[I3SimpleParametrization::PAR_LinL] = 0.123456789*I3Units::km; 
    /*double &LogL =*/testvalues[I3SimpleParametrization::PAR_LogL] = log10(L);
    double &speed= testvalues[I3SimpleParametrization::PAR_Speed] = 0.8*I3Constants::c; 
    I3EventHypothesisPtr eptr(new I3EventHypothesis);
    eptr->particle->SetTime(t);
    eptr->particle->SetPos(x,y,z);
    eptr->particle->SetDir(zen,azi);
    eptr->particle->SetEnergy(E);
    eptr->particle->SetLength(L);
    eptr->particle->SetSpeed(speed);

    std::bitset<I3SimpleParametrization::PAR_N> freevar;
    // for ( int i=0; i<I3SimpleParametrization::PAR_N; ++i ) freevar[i]=1;
    freevar.set();
    freevar[I3SimpleParametrization::PAR_LinE] = 0;
    freevar[I3SimpleParametrization::PAR_LinL] = 0;
    ENSURE_EQUAL( size_t(9), freevar.count(), "number of free parameters wrong" );

    const double eps = 1.0e-5;
    for ( int n=0; n<5; ++n ){
        // testpar->SetHypothesisPtr(eptr);
        const std::vector<I3FitParameterInitSpecs> &parinits = testpar->GetParInitSpecs(eptr);
        std::ostringstream msg1;
        msg1 << "inconsistent number of free parameters, "
             << parinits.size() << " vs. " << testpar->GetNPar();
        ENSURE( parinits.size() == testpar->GetNPar(), msg1.str().c_str() );
        std::ostringstream msg2;
        msg2 << "inconsistent number of free parameters, got "
             << parinits.size() << " (should be 9: txyz, zen, azi, LogL, LogE, speed)";
        ENSURE( parinits.size() == 9, msg2.str().c_str() );
        size_t ipar = 0;
        size_t npar = freevar.count();
        std::vector<double> newpars(npar);
        for (size_t jpar=0; ipar<npar && jpar<I3SimpleParametrization::PAR_N; ++jpar){
            if ( ! freevar[jpar] ) continue;
            std::ostringstream msg;
            double getvalue = parinits[ipar].initval_;
            double expectvalue = testvalues[jpar];
            msg << "ERROR: par " << I3SimpleParametrization::GetParName(jpar)
                << "=" << getvalue << ", expected " << expectvalue;
            ENSURE_DISTANCE( getvalue, expectvalue, eps*expectvalue, msg.str().c_str() );
            // next test: pseudorandomly add/subtract 20% to/from each parameter
            double newtestvalue = (1.0 + 0.04*(((7*ipar+n*13)%11)-5.0))*expectvalue;
            newpars[ipar] = testvalues[jpar] = newtestvalue;
            ++ipar;
        }
        I3EventHypothesisConstPtr enew = testpar->GetHypothesisPtr(newpars);
        double xexpect = testvalues[I3SimpleParametrization::PAR_X];
        ENSURE_DISTANCE( enew->particle->GetPos().GetX(), xexpect, eps*xexpect, "x not updated correctly" );
        double texpect = testvalues[I3SimpleParametrization::PAR_T];
        ENSURE_DISTANCE( enew->particle->GetTime(), texpect, eps*texpect, "time not updated correctly" );
        double zenexpect = testvalues[I3SimpleParametrization::PAR_Zen];
        ENSURE_DISTANCE( enew->particle->GetZenith(), zenexpect, eps*zenexpect, "zenith not updated correctly" );
        double logEexpect = testvalues[I3SimpleParametrization::PAR_LogE];
        ENSURE_DISTANCE( log10(enew->particle->GetEnergy()), logEexpect, eps*logEexpect, "energy not updated correctly" );
        double logLexpect = testvalues[I3SimpleParametrization::PAR_LogL];
        ENSURE_DISTANCE( log10(enew->particle->GetLength()), logLexpect, eps*logLexpect, "length not updated correctly" );
        double speedexpect = testvalues[I3SimpleParametrization::PAR_Speed];
        ENSURE_DISTANCE( enew->particle->GetSpeed(), speedexpect, eps*speedexpect, "speed not updated correctly" );
    }

}

// simplistic testing LLH
// Gaussian with distances of start/stop points to "true" start/stop points
// true start is at 0,0,0
// true stop is at d,0,0
class I3ContainedLLHTEST : public I3EventLogLikelihoodBase {
    private:
        const I3Position trueStart_;
        const I3Position trueStop_;
        const double sigma2_;
        const double lognorm_;
        const std::string name_;
    public:
        I3ContainedLLHTEST(double distance=100., double sigma=10., std::string n="testllh"):
            I3EventLogLikelihoodBase(),
            trueStart_(0.,0.,0.), trueStop_(distance,0.,0.),
            sigma2_(sigma*sigma),lognorm_(-log(sqrt(2*M_PI*sigma2_))),name_(n) {}
        ~I3ContainedLLHTEST(){}

        // overriding virtual method from the base class
        void SetGeometry( const I3Geometry& geo){}//dummy
        // overriding virtual method from base class
        void SetEvent( const I3Frame &f ){}// dummy

        // overriding virtual method from base class
        double GetLogLikelihood( const I3EventHypothesis &ehypo ){
            const I3Position & pstart = ehypo.particle->GetStartPos();
            const I3Position & pstop = ehypo.particle->GetStopPos();
        double dstart = (trueStart_ - pstart).Magnitude();
        double dstop = (trueStop_ - pstop).Magnitude();
            /*
            log_trace("start=(%.1f,%.1f,%.1f) stop=(%.1f,%.1f,%.1f)",
                     pstart.GetX(), pstart.GetY(), pstart.GetZ(),
                     pstop.GetX(), pstop.GetY(), pstop.GetZ());
            */
            return -0.5*(dstart*dstart+dstop*dstop)/sigma2_+lognorm_;
        }

        // overriding virtual method from base class
        unsigned int GetMultiplicity() {return 42;}// dummy

        // overriding virtual method from base class
        const std::string GetName() const {return name_;}

        const I3Position& GetStopPos() const { return trueStop_; }
        const I3Position& GetStartPos() const { return trueStart_; }
};

I3_POINTER_TYPEDEFS(I3ContainedLLHTEST);

TEST(KeepStartVertex)
{
    double truex=100.*I3Units::m;
    double seedlength=120.*I3Units::m;
    double tolerance = 0.0001;
    double LogLstepsize = 0.2;
    double DIRstepsize = 0.1*I3Units::radian;
    double llhwidth = 5.0*I3Units::m;
    double good_enough = 0.1*I3Units::m;

    // keep start point fixed, fit length (moving stop point)
    // VERTEX_Start does the same as VERTEX_Default
    TestParMaker tpm;
    tpm.SetMode(I3SimpleParametrization::VERTEX_Start);
    tpm.SetL(LogLstepsize,0.,0.,0.,0.,true/*log*/);
    I3SimpleParametrizationPtr Lparptr = tpm.MakeNewServicePtr("startL");
    I3ContainedLLHTESTPtr llhptr( new I3ContainedLLHTEST(truex,llhwidth));
    I3GSLSimplexPtr miniptr( new I3GSLSimplex( "gsl",tolerance,tolerance,1000,100,true) );

    I3GulliverPtr gptr( new I3Gulliver( Lparptr,llhptr,miniptr,"containedtracktest"));

    I3LogLikelihoodFitPtr fitptr( new I3LogLikelihoodFit );
    I3Particle &p = *(fitptr->hypothesis_->particle);
    p.SetTime(0.);
    p.SetEnergy(0.5*I3Units::TeV);
    p.SetPos(0.,0.,0.);
    p.SetDir(1.,0.,0.);
    p.SetShape(I3Particle::ContainedTrack);
    p.SetType(I3Particle::MuPlus);
    p.SetLength(seedlength);
    gptr->Fit(fitptr);

    ENSURE(p.GetFitStatus()==I3Particle::OK,
            "fit of length with fixed starting point FAILED to converge");
    ENSURE_DISTANCE(
        (llhptr->GetStopPos()-p.GetStopPos()).Magnitude(), 0., good_enough,
            "fitting length with fixed starting point got WRONG RESULT");
    // now a bit more ambitious: fit also the direction
    p.SetPos(0.,0.,0.);
    p.SetDir(cos(0.3),sin(0.3),0.); // way out
    p.SetLength(seedlength);
    tpm.SetDir(DIRstepsize);
    Lparptr = tpm.MakeNewServicePtr("starDIRL");
    gptr->ChangeParametrization( Lparptr );
    gptr->Fit(fitptr);
    ENSURE(p.GetFitStatus()==I3Particle::OK,
            "fit of length and direction with fixed starting point FAILED to converge");
    ENSURE_DISTANCE(
            (llhptr->GetStopPos()-p.GetStopPos()).Magnitude(), 0., good_enough,
            "fitting length and direction with fixed starting point got WRONG RESULT");

    // now the same exercises with stop point instead of start point

    //  keep stop point fixed, fit length (moving start point)
    p.SetLength(seedlength);
    double wrongx = 25.0;
    p.SetPos(wrongx,0.,0.);
    p.SetLength(truex-wrongx); // stop point is now at true value
    tpm.SetDir(0.);
    tpm.SetMode(I3SimpleParametrization::VERTEX_Stop);
    Lparptr = tpm.MakeNewServicePtr("stopL");
    gptr->ChangeParametrization( Lparptr );
    gptr->Fit(fitptr);

    ENSURE(p.GetFitStatus()==I3Particle::OK,
            "fit of length with fixed stopping point FAILED to converge");
    ENSURE_DISTANCE(
            (llhptr->GetStartPos()-p.GetStartPos()).Magnitude(), 0., good_enough,
            "fitting length with fixed stopping point got WRONG RESULT");
    // now a bit more ambitious: fit also the direction
    p.SetDir(cos(0.3)*sin(0.2),sin(0.3)*sin(0.2),cos(0.2)); // way out
    const I3Direction &d = p.GetDir();
    p.SetPos(truex-seedlength*d.GetX(),-seedlength*d.GetY(),-seedlength*d.GetZ());
    p.SetLength(seedlength);
    ENSURE_DISTANCE(
            (llhptr->GetStopPos()-p.GetStopPos()).Magnitude(), 0., good_enough,
            "initializing stop point failed");
    tpm.SetDir(DIRstepsize);
    Lparptr = tpm.MakeNewServicePtr("stopDIRL");
    gptr->ChangeParametrization( Lparptr );
    gptr->Fit(fitptr);
    ENSURE(p.GetFitStatus()==I3Particle::OK,
            "fit of length and direction with fixed stopping point FAILED to converge");
    ENSURE_DISTANCE(
            (llhptr->GetStartPos()-p.GetStartPos()).Magnitude(), 0., good_enough,
            "fitting length and direction with fixed stopping point got WRONG RESULT");

}

TEST(RelativeBounds)
{

    // make a parametrization of log(E) and t, with relative bounds
    TestParMaker tpm;
    double logErelmin = -0.5;
    double logErelmax = +0.4;
    double logEstep=0.1;
    double Trelmin = -200.*I3Units::ns;
    double Trelmax = +100.*I3Units::ns;
    double Tstep= +10.*I3Units::ns;
    double Srelmin = -0.1*I3Constants::c;
    double Srelmax = +0.1*I3Constants::c;
    double Sstep = +0.01*I3Constants::c;
    tpm.SetE(logEstep,0.,0.,logErelmin,logErelmax,true/*log*/);
    tpm.SetTime(Tstep,0.,0.,Trelmin,Trelmax);
    tpm.SetSpeed(Sstep,0.,0.,Srelmin,Srelmax);
    I3SimpleParametrizationPtr TlogESparptr = tpm.MakeNewServicePtr("TlogES");

    // make a hypothesis with some energy
    I3EventHypothesisPtr eptr(new I3EventHypothesis);
    double testE = 1.0*I3Units::PeV;
    double testT = 123456.789*I3Units::ns;
    double testS = 0.9*I3Constants::c;
    eptr->particle->SetEnergy(testE);
    eptr->particle->SetTime(testT);
    eptr->particle->SetSpeed(testS);
    TlogESparptr->SetHypothesisPtr(eptr);

    // get parameter init specs
    const std::vector<I3FitParameterInitSpecs> &parinits = TlogESparptr->GetParInitSpecs(eptr);
    ENSURE(parinits.size() == 3,
           "there should be exactly 3 free fit parameters in this parametrization");
    const I3FitParameterInitSpecs& parinitTime = parinits[0];
    const I3FitParameterInitSpecs& parinitLogE = parinits[1];
    const I3FitParameterInitSpecs& parinitSpeed = parinits[2];

    // check bounds (note that with log10(E), relative bounds are multiplicative)
    const double eps = 1.e-6;
    double testEmin = pow(10.0,logErelmin)*testE;
    double testEmax = pow(10.0,logErelmax)*testE;
    double testTmin = testT + Trelmin;
    double testTmax = testT + Trelmax;
    double testSmin = testS + Srelmin;
    double testSmax = testS + Srelmax;
    ENSURE_DISTANCE( parinitTime.initval_, testT, eps*testT, "wrong initial time" );
    ENSURE_DISTANCE( parinitLogE.initval_, log10(testE), eps*log10(testE), "wrong initial log(E) value" );
    std::ostringstream msgLogE;
    msgLogE << "wrong energy limits: "
            << "min=" << parinitLogE.minval_ << " (expected: " << log10(testEmin) << ") "
            << "max=" << parinitLogE.maxval_ << " (expected: " << log10(testEmax) << ") ";
    ENSURE_DISTANCE( parinitLogE.minval_, log10(testEmin), eps*log10(testEmin), msgLogE.str().c_str() );
    ENSURE_DISTANCE( parinitLogE.maxval_, log10(testEmax), eps*log10(testEmax), msgLogE.str().c_str() );

    std::ostringstream msgTime;
    msgTime << "wrong time limits: "
            << "min=" << parinitTime.minval_ << " (expected: " << testTmin << ") "
            << "max=" << parinitTime.maxval_ << " (expected: " << testTmax << ") ";
    ENSURE_DISTANCE( parinitTime.minval_, testTmin, eps*testTmin, msgTime.str().c_str() );
    ENSURE_DISTANCE( parinitTime.maxval_, testTmax, eps*testTmax, msgTime.str().c_str() );

    std::ostringstream msgSpeed;
    msgSpeed << "wrong speed limits: "
             << "min=" << parinitSpeed.minval_ << " (expected: " << testSmin << ") "
             << "max=" << parinitSpeed.maxval_ << " (expected: " << testSmax << ") ";
    ENSURE_DISTANCE( parinitSpeed.minval_, testSmin, eps*testSmin, msgSpeed.str().c_str() );
    ENSURE_DISTANCE( parinitSpeed.maxval_, testSmax, eps*testSmax, msgSpeed.str().c_str() );

}

TEST(AbsoluteBounds)
{

    // make a parametrization of xyz and log(L)
    // position xyz gets only absolute bounds
    // log(L) gets both absolute and relative bounds
    // speed gets relative bounds
    TestParMaker tpm;
    double logLrelmin = -0.5;
    double logLrelmax = +0.4;
    double logLabsmin = 1.0; /* 10 m */
    double logLabsmax = 4.0; /* 10 km */
    double logLstep = 0.2;
    double XYZmin = -2000.*I3Units::m;
    double XYZmax = +2000.*I3Units::m;
    double XYZstep = +20.*I3Units::m;
    double Sabsmin = 0.5*I3Constants::c;
    double Sabsmax = +1.0*I3Constants::c;
    double Sstep = +0.05*I3Constants::c;
    tpm.SetL(logLstep,logLabsmin,logLabsmax,logLrelmin,logLrelmax,true/*log*/);
    tpm.SetXYZ(XYZstep,XYZmin,XYZmax,0.,0.);
    tpm.SetSpeed(Sstep,Sabsmin,Sabsmax,0.,0.);
    I3SimpleParametrizationPtr XYZlogLSparptr = tpm.MakeNewServicePtr("XYZlogLS");

    // make a hypothesis with some length, direction, time and position
    I3EventHypothesisPtr eptr(new I3EventHypothesis);
    double testL = 100.0*I3Units::m;
    double testS = 0.8*I3Constants::c;
    double testT = 123456.789*I3Units::ns;
    double testX = 123.456789*I3Units::m;
    double testY = 12.3456789*I3Units::m;
    double testZ = 1.23456789*I3Units::m;
    eptr->particle->SetSpeed(testS); // some speed
    eptr->particle->SetLength(testL); // some length
    eptr->particle->SetTime(testT); // some time
    eptr->particle->SetPos(testX,testY,testZ); // some place
    eptr->particle->SetDir(M_SQRT2,0,M_SQRT2); // 45 deg in xz plane
    XYZlogLSparptr->SetHypothesisPtr(eptr);

    // get parameter init specs (5 parameters: xyz, logL and Speed)
    std::vector<I3FitParameterInitSpecs> parinits = XYZlogLSparptr->GetParInitSpecs(eptr);
    ENSURE(parinits.size() == 5,
           "there should be exactly 5 free fit parameters in this parametrization");
    I3FitParameterInitSpecs parinitX = parinits[0];
    I3FitParameterInitSpecs parinitY = parinits[1];
    I3FitParameterInitSpecs parinitZ = parinits[2];
    I3FitParameterInitSpecs parinitLogL = parinits[3];
    I3FitParameterInitSpecs parinitS = parinits[4];

    // check initial values
    const double eps = 1.e-6;
    ENSURE_DISTANCE( parinitX.initval_, testX, eps*testX, "wrong initial X" );
    ENSURE_DISTANCE( parinitY.initval_, testY, eps*testY, "wrong initial Y" );
    ENSURE_DISTANCE( parinitZ.initval_, testZ, eps*testZ, "wrong initial Z" );
    ENSURE_DISTANCE( parinitLogL.initval_, log10(testL), eps*log10(testL), "wrong initial log(L) value" );
    ENSURE_DISTANCE( parinitS.initval_, testS, eps*testS, "wrong initial speed" );

    // check bounds
    double testLogLmin = log10(testL)+logLrelmin;
    double testLogLmax = log10(testL)+logLrelmax;
    std::ostringstream msgLogL;
    msgLogL << "wrong logL limits: "
            << "min=" << parinitLogL.minval_ << " (expected: " << testLogLmin << ") "
            << "max=" << parinitLogL.maxval_ << " (expected: " << testLogLmax << ") ";
    ENSURE_DISTANCE( parinitLogL.minval_, testLogLmin, eps*testLogLmin, msgLogL.str().c_str() );
    ENSURE_DISTANCE( parinitLogL.maxval_, testLogLmax, eps*testLogLmax, msgLogL.str().c_str() );

    std::ostringstream msgX;
    msgX << "wrong X limits: "
         << "min=" << parinitX.minval_ << " (expected: " << XYZmin << ") "
         << "max=" << parinitX.maxval_ << " (expected: " << XYZmax << ") ";
    ENSURE_DISTANCE( parinitX.minval_, XYZmin, eps*fabs(XYZmin), msgX.str().c_str() );
    ENSURE_DISTANCE( parinitX.maxval_, XYZmax, eps*XYZmax, msgX.str().c_str() );

    std::ostringstream msgS;
    msgS << "wrong speed limits: "
         << "min=" << parinitS.minval_ << " (expected: " << Sabsmin << ") "
         << "max=" << parinitS.maxval_ << " (expected: " << Sabsmax << ") ";
    ENSURE_DISTANCE( parinitS.minval_, Sabsmin, eps*Sabsmin, msgS.str().c_str() );
    ENSURE_DISTANCE( parinitS.maxval_, Sabsmax, eps*Sabsmax, msgS.str().c_str() );

    // change L such that relative lower bound is lower than absolute lower bound
    // absolute lower bound then overrides the relative lower bound
    testL = pow(10.,logLabsmin+1.1*logLstep);
    eptr->particle->SetLength(testL);
    XYZlogLSparptr->SetHypothesisPtr(eptr);
    parinits = XYZlogLSparptr->GetParInitSpecs(eptr);
    parinitLogL = parinits[3];
    testLogLmin = logLabsmin;
    testLogLmax = log10(testL)+logLrelmax;
    std::ostringstream msgLogL2;
    msgLogL2 << "wrong logL limits: "
             << "min=" << parinitLogL.minval_ << " (expected: " << testLogLmin << ") "
             << "max=" << parinitLogL.maxval_ << " (expected: " << testLogLmax << ") ";
    ENSURE_DISTANCE( parinitLogL.initval_, log10(testL), eps*log10(testL), "wrong initial log(L) value" );
    ENSURE_DISTANCE( parinitLogL.minval_, testLogLmin, eps*testLogLmin, msgLogL2.str().c_str() );
    ENSURE_DISTANCE( parinitLogL.maxval_, testLogLmax, eps*testLogLmax, msgLogL2.str().c_str() );

    // change l such that relative upper bound is higher than absolute upper bound
    // absolute upper bound then overrides the relative upper bound
    testL = pow(10.,logLabsmax-1.1*logLstep);
    eptr->particle->SetLength(testL);
    XYZlogLSparptr->SetHypothesisPtr(eptr);
    parinits = XYZlogLSparptr->GetParInitSpecs(eptr);
    parinitLogL = parinits[3];
    testLogLmax = logLabsmax;
    testLogLmin = log10(testL)+logLrelmin;
    std::ostringstream msgLogL3;
    msgLogL3 << "wrong logL limits: "
             << "min=" << parinitLogL.minval_ << " (expected: " << testLogLmin << ") "
             << "max=" << parinitLogL.maxval_ << " (expected: " << testLogLmax << ") ";
    ENSURE_DISTANCE( parinitLogL.initval_, log10(testL), eps*log10(testL), "wrong initial log(L) value" );
    ENSURE_DISTANCE( parinitLogL.minval_, testLogLmin, eps*testLogLmin, msgLogL3.str().c_str() );
    ENSURE_DISTANCE( parinitLogL.maxval_, testLogLmax, eps*testLogLmax, msgLogL3.str().c_str() );

}

