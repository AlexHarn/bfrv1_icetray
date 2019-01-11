#include <sstream>
#include <I3Test.h>
#include "minimizer/I3MinimizerUtils.h"
#include "icetray/I3Logging.h"

using I3MinimizerUtils::ParameterWithInclusiveBounds;

TEST_GROUP(BoundsTest);

TEST(TestMinMax)
{
    double minval=-3., maxval=6.,initval=4;
    ParameterWithInclusiveBounds pwib(minval,maxval,initval);
    // random values
    for (double x=-10000; x<+10000; x+=1.){
        pwib.SetFreeValue(x);
        double b=pwib.GetBoundedValue();
        std::ostringstream oss1;
        oss1 << "(xfree=" << x << ") exceeded max: b-max="
             << b << "-" << maxval << "="
             << (b-maxval)
             << std::endl;
        ENSURE(maxval>=b,oss1.str().c_str());
        std::ostringstream oss2;
        oss2 << "exceeded min: b-min=" << b << "-" << minval << "=" << (b-minval) << std::endl;
        ENSURE(minval<=b,oss2.str().c_str());
    }
    // hitting the extremes
    for (int i=-20; i<=+20; i++ ){
        pwib.SetFreeValue((i+0.5)*M_PI);
        double b=pwib.GetBoundedValue();
        std::ostringstream oss1;
        oss1 << "(i=" << i << ") exceeded max: b-max="
             << b << "-" << maxval << "="
             << (b-maxval)
             << std::endl;
        ENSURE(maxval>=b,oss1.str().c_str());
        std::ostringstream oss2;
        oss2 << "(i=" << i << ") exceeded min: b-min="
             << b << "-" << minval << "="
             << (b-minval)
             << std::endl;
        ENSURE(minval<=b,oss2.str().c_str());
    }
}

TEST(TestStepsize)
{
    double minval=-300., maxval=600.,initval=444,boundstep=155;
    ParameterWithInclusiveBounds pwib(minval,maxval,initval);
    double xfree = pwib.GetFreeValue();
    ENSURE_DISTANCE(xfree,asin(294./450.),0.01,
            "bounded->free conversion gone wrong");
    ENSURE((xfree>0) && (xfree<0.5*M_PI),"free parameter not in expected range??");
    double xstep = pwib.GetFreeStep(boundstep);
    ENSURE_DISTANCE(xstep,asin(294./450.)-asin(139./450.),0.01,
            "free stepsize computation gone wrong");
    pwib.SetFreeValue(xfree-xstep);
    double newboundedvalue = pwib.GetBoundedValue();
    ENSURE_DISTANCE(newboundedvalue,initval-boundstep,0.1,
            "free stepsize value (computed from bounded values) is wrong");
    initval=-100;
    pwib.SetBoundedValue(initval);
    xfree = pwib.GetFreeValue();
    ENSURE_DISTANCE(xfree,asin(-250./450.),0.01,
            "bounded->free conversion gone wrong");
    ENSURE((xfree<0) && (xfree>-0.5*M_PI),
            "free parameter not in expected range??");
    xstep = pwib.GetFreeStep(boundstep);
    ENSURE_DISTANCE(xstep,asin(-95./450.)-asin(-250./450.),0.01,
            "stepsize value computation gone wrong");
    pwib.SetFreeValue(xfree+xstep);
    newboundedvalue = pwib.GetBoundedValue();
    ENSURE_DISTANCE(newboundedvalue,initval+boundstep,0.1,
            "free stepsize value (computed from bounded values) is wrong");
}
