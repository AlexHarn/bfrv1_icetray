#include <I3Test.h>

#include <boost/make_shared.hpp>

#include <icetray/I3Tray.h>

#include "I3TestDummyEventLogLikelihood.h"

//TODO: This set of tests only checks the parameters accepted by the
//      I3EventLogLikelihoodCombiner. The actual function of combining
//      likelihoods should also be tested!
TEST_GROUP(Gulliver_LikelihoodCombination);

//------------------------------------------------------------------------------
//The likelihood combiner should accept both pointers to likelihoods and strings
//which are the names of existing likelihoods in the context. These things
//should be successfully extracted from any iterable data structure.

TEST(CombinerAcceptsStrings){
    GetIcetrayLogger()->SetLogLevelForUnit("I3Tray",I3LOG_WARN);
    I3Tray tray;
    I3EventLogLikelihoodBasePtr l1=
    boost::make_shared<I3TestDummyEventLogLikelihood>(1,2,3,.1,.1,.1);
    I3EventLogLikelihoodBasePtr l2=
    boost::make_shared<I3TestDummyEventLogLikelihood>(4,2,3,.1,.1,.1);
    tray.GetContext().Put("LLH1",l1);
    tray.GetContext().Put("LLH2",l2);

    std::vector<std::string> llhs;
    llhs.push_back("LLH1");
    llhs.push_back("LLH2");
    tray.AddService("I3EventLogLikelihoodCombinerFactory","cllh")
    ("InputLogLikelihoods",llhs);
    tray.AddModule("ManyStreamsSource");
    tray.Execute(10);
}

//no one is likely to use a tuple in real code,
//but it is another perfectly good iterable object
TEST(CombinerAcceptsStrings_Tuple){
    GetIcetrayLogger()->SetLogLevelForUnit("I3Tray",I3LOG_WARN);
    I3Tray tray;
    I3EventLogLikelihoodBasePtr l1=
    boost::make_shared<I3TestDummyEventLogLikelihood>(1,2,3,.1,.1,.1);
    I3EventLogLikelihoodBasePtr l2=
    boost::make_shared<I3TestDummyEventLogLikelihood>(4,2,3,.1,.1,.1);
    tray.GetContext().Put("LLH1",l1);
    tray.GetContext().Put("LLH2",l2);

    boost::python::tuple llhs=boost::python::make_tuple("LLH1","LLH2");
    tray.AddService("I3EventLogLikelihoodCombinerFactory","cllh")
    ("InputLogLikelihoods",llhs);
    tray.AddModule("ManyStreamsSource");
    tray.Execute(10);
}

TEST(CombinerAcceptsPointers){
    GetIcetrayLogger()->SetLogLevelForUnit("I3Tray",I3LOG_WARN);
    I3Tray tray;
    I3EventLogLikelihoodBasePtr l1=
    boost::make_shared<I3TestDummyEventLogLikelihood>(1,2,3,.1,.1,.1);
    I3EventLogLikelihoodBasePtr l2=
    boost::make_shared<I3TestDummyEventLogLikelihood>(4,2,3,.1,.1,.1);

    boost::python::import("icecube.icetray");
    boost::python::import("icecube.gulliver");
    boost::python::list llhs;
    llhs.append(l1);
    llhs.append(l2);
    tray.AddService("I3EventLogLikelihoodCombinerFactory","cllh")
    ("InputLogLikelihoods",llhs);
    tray.AddModule("ManyStreamsSource");
    tray.Execute(10);
}

TEST(CombinerAcceptsPointers_Tuple){
    GetIcetrayLogger()->SetLogLevelForUnit("I3Tray",I3LOG_WARN);
    I3Tray tray;
    I3EventLogLikelihoodBasePtr l1=
    boost::make_shared<I3TestDummyEventLogLikelihood>(1,2,3,.1,.1,.1);
    I3EventLogLikelihoodBasePtr l2=
    boost::make_shared<I3TestDummyEventLogLikelihood>(4,2,3,.1,.1,.1);

    boost::python::import("icecube.icetray");
    boost::python::import("icecube.gulliver");
    boost::python::tuple llhs=boost::python::make_tuple(l1,l2);
    tray.AddService("I3EventLogLikelihoodCombinerFactory","cllh")
    ("InputLogLikelihoods",llhs);
    tray.AddModule("ManyStreamsSource");
    tray.Execute(10);
}

//mixing strings and pointer in the same container should still work
TEST(CombinerAcceptsPointerStringMixture){
    GetIcetrayLogger()->SetLogLevelForUnit("I3Tray",I3LOG_WARN);
    I3Tray tray;
    I3EventLogLikelihoodBasePtr l1=
    boost::make_shared<I3TestDummyEventLogLikelihood>(1,2,3,.1,.1,.1);
    I3EventLogLikelihoodBasePtr l2=
    boost::make_shared<I3TestDummyEventLogLikelihood>(4,2,3,.1,.1,.1);

    boost::python::import("icecube.icetray");
    boost::python::import("icecube.gulliver");
    boost::python::list llhs;
    tray.GetContext().Put("LLH1",l1);
    llhs.append("LLH1");
    llhs.append(l2);
    tray.AddService("I3EventLogLikelihoodCombinerFactory","cllh")
    ("InputLogLikelihoods",llhs);
    tray.AddModule("ManyStreamsSource");
    tray.Execute(10);
}

//------------------------------------------------------------------------------
//Trying to pass invalid things as likelihoods should be caught

//If given a string, a matching key must exist in the context
TEST(StringNotInContext){
    GetIcetrayLogger()->SetLogLevelForUnit("I3Tray",I3LOG_WARN);
    I3Tray tray;
    I3EventLogLikelihoodBasePtr l1=
    boost::make_shared<I3TestDummyEventLogLikelihood>(1,2,3,.1,.1,.1);
    tray.GetContext().Put("LLH1",l1);

    std::vector<std::string> llhs;
    llhs.push_back("LLH1");
    llhs.push_back("foo");
    tray.AddService("I3EventLogLikelihoodCombinerFactory","cllh")
    ("InputLogLikelihoods",llhs);
    tray.AddModule("ManyStreamsSource");
    try{
        tray.Execute(10);
        FAIL("A string referring to a likelihood which is not in the context should be rejected");
    }catch(std::runtime_error& e){/*squash*/}
}

//If given a string, the value stored under that key in the context must be a
//likelihood object
TEST(StringDoesNotReferToLikelihood){
    GetIcetrayLogger()->SetLogLevelForUnit("I3Tray",I3LOG_WARN);
    I3Tray tray;
    I3EventLogLikelihoodBasePtr l1=
    boost::make_shared<I3TestDummyEventLogLikelihood>(1,2,3,.1,.1,.1);
    tray.GetContext().Put("LLH1",l1);
    tray.GetContext().Put("LLH2",boost::make_shared<int>(5));

    std::vector<std::string> llhs;
    llhs.push_back("LLH1");
    llhs.push_back("LLH2");
    tray.AddService("I3EventLogLikelihoodCombinerFactory","cllh")
    ("InputLogLikelihoods",llhs);
    tray.AddModule("ManyStreamsSource");
    try{
        tray.Execute(10);
        FAIL("A string referring to an object which is not a likelihood should be rejected");
    }catch(std::runtime_error& e){/*squash*/}
}

//If given a pointer it must point to a likelihood object
TEST(NonLLHPointer){
    GetIcetrayLogger()->SetLogLevelForUnit("I3Tray",I3LOG_WARN);
    I3Tray tray;
    I3EventLogLikelihoodBasePtr l1=
    boost::make_shared<I3TestDummyEventLogLikelihood>(1,2,3,.1,.1,.1);
    boost::python::long_ l2(15);

    boost::python::import("icecube.icetray");
    boost::python::import("icecube.gulliver");
    boost::python::list llhs;
    llhs.append(l1);
    llhs.append(l2);
    tray.AddService("I3EventLogLikelihoodCombinerFactory","cllh")
    ("InputLogLikelihoods",llhs);
    tray.AddModule("ManyStreamsSource");
    try{
        tray.Execute(10);
        FAIL("Non-likelihood objects should be rejected");
    }catch(std::runtime_error& e){/*squash*/}
}

//------------------------------------------------------------------------------
//The collection of weights must either have zero length or have the same length
//as the collection of likelihoods

TEST(TooFewWeights){
    GetIcetrayLogger()->SetLogLevelForUnit("I3Tray",I3LOG_WARN);
    I3Tray tray;
    I3EventLogLikelihoodBasePtr l1=
    boost::make_shared<I3TestDummyEventLogLikelihood>(1,2,3,.1,.1,.1);
    I3EventLogLikelihoodBasePtr l2=
    boost::make_shared<I3TestDummyEventLogLikelihood>(4,2,3,.1,.1,.1);
    tray.GetContext().Put("LLH1",l1);
    tray.GetContext().Put("LLH2",l2);

    std::vector<std::string> llhs;
    llhs.push_back("LLH1");
    llhs.push_back("LLH2");
    std::vector<double> weights;
    weights.push_back(1);
    tray.AddService("I3EventLogLikelihoodCombinerFactory","cllh")
    ("InputLogLikelihoods",llhs)
    ("RelativeWeights",weights);
    tray.AddModule("ManyStreamsSource");
    try{
        tray.Execute(10);
        FAIL("Specifying fewer weights than likelihoods should be rejected");
    }catch(std::runtime_error& e){/*squash*/}
}

TEST(MatchingNumberWeights){
    GetIcetrayLogger()->SetLogLevelForUnit("I3Tray",I3LOG_WARN);
    I3Tray tray;
    I3EventLogLikelihoodBasePtr l1=
    boost::make_shared<I3TestDummyEventLogLikelihood>(1,2,3,.1,.1,.1);
    I3EventLogLikelihoodBasePtr l2=
    boost::make_shared<I3TestDummyEventLogLikelihood>(4,2,3,.1,.1,.1);
    tray.GetContext().Put("LLH1",l1);
    tray.GetContext().Put("LLH2",l2);

    std::vector<std::string> llhs;
    llhs.push_back("LLH1");
    llhs.push_back("LLH2");
    std::vector<double> weights;
    weights.push_back(1);
    weights.push_back(2);
    tray.AddService("I3EventLogLikelihoodCombinerFactory","cllh")
    ("InputLogLikelihoods",llhs)
    ("RelativeWeights",weights);
    tray.AddModule("ManyStreamsSource");
    tray.Execute(10);
}

TEST(TooManyWeights){
    GetIcetrayLogger()->SetLogLevelForUnit("I3Tray",I3LOG_WARN);
    I3Tray tray;
    I3EventLogLikelihoodBasePtr l1=
    boost::make_shared<I3TestDummyEventLogLikelihood>(1,2,3,.1,.1,.1);
    I3EventLogLikelihoodBasePtr l2=
    boost::make_shared<I3TestDummyEventLogLikelihood>(4,2,3,.1,.1,.1);
    tray.GetContext().Put("LLH1",l1);
    tray.GetContext().Put("LLH2",l2);

    std::vector<std::string> llhs;
    llhs.push_back("LLH1");
    llhs.push_back("LLH2");
    std::vector<double> weights;
    weights.push_back(1);
    weights.push_back(2);
    weights.push_back(3);
    tray.AddService("I3EventLogLikelihoodCombinerFactory","cllh")
    ("InputLogLikelihoods",llhs)
    ("RelativeWeights",weights);
    tray.AddModule("ManyStreamsSource");
    try{
        tray.Execute(10);
        FAIL("Specifying fewer weights than likelihoods should be rejected");
    }catch(std::runtime_error& e){/*squash*/}
}
