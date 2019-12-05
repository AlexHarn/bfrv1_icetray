#include <I3Test.h>

#include <gulliver/I3FitParameterInitSpecs.h>

TEST_GROUP(I3FitParameterInitSpecs);

TEST(initialized_constructor) {
  I3FitParameterInitSpecs specs("TheName");

  ENSURE(specs.name_ == "TheName");
  ENSURE(std::isnan(specs.initval_));
  ENSURE(std::isnan(specs.stepsize_));
  ENSURE(std::isnan(specs.minval_));
  ENSURE(std::isnan(specs.maxval_));
}

TEST(comparison_operator) {
  I3FitParameterInitSpecs specs("TheName");
  specs.initval_ = 42.;
  specs.stepsize_ = 55.;
  specs.minval_ = -666.;
  specs.maxval_ = 666.;

  I3FitParameterInitSpecs specs2("TheName");
  specs2.initval_ = 42.;
  specs2.stepsize_ = 55.;
  specs2.minval_ = -666.;
  specs2.maxval_ = 666.;

  ENSURE(specs == specs2);

  specs.name_="Different";
  ENSURE(!(specs == specs2), "If the names are different, they are not equal");

  //The comparison only checks the name
  specs.name_ = specs2.name_;
  specs2.initval_ = 33;
  ENSURE(specs == specs2, "Only the name should matter");
}