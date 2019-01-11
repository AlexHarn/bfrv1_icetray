#include "I3Test.h"

#include "finiteReco/StringLLH.h"

TEST_GROUP(finiteRecoStringLLHTest);

TEST(ProbsSringLLH)
{
  StringLLH stringProb;
  ENSURE(stringProb.GetStringNr()==-100,"String number is not initialised correctly");
  stringProb.AddOM(0.5,true,false);
  ENSURE(stringProb.GetProb() == log(0.5),"The probability is not given correctly to the private members");
  int strNum = 1;
  stringProb.NewString(strNum);
  ENSURE(stringProb.GetStringNr()== 1,"String number is not initialised correctly");
  stringProb.AddOM(0.5,false,false);
  ENSURE(stringProb.GetProb() == 0,"Miscalculation in the recursion");
  ++strNum;
  stringProb.NewString(strNum);
  ENSURE(stringProb.GetProb()== log(1),"You are calculating rubbish");
  stringProb.AddOM(0.1,false,false);
  ENSURE(stringProb.GetProb()== log(1),"You are calculating rubbish");
  stringProb.AddOM(0.2,false,false);
  ENSURE(stringProb.GetProb()==log(0.98),"You are calculating rubbish");
  stringProb.AddOM(0.3,true,false);
  ENSURE(stringProb.GetProb()==log(0.24),"You are calculating rubbish");
  stringProb.AddOM(0.4,true,false);
  ENSURE(stringProb.GetProb()==log(0.096),"You are calculating rubbish");
  ++strNum;
  stringProb.NewString(strNum);
  stringProb.AddOM(0.1,false,false);
  stringProb.AddOM(0.2,false,false);
  stringProb.AddOM(0.3,false,false);
  stringProb.AddOM(0.4,false,false);
  ENSURE(stringProb.GetProb()==log(0.83),"You are calculating rubbish");
  ++strNum;
  stringProb.NewString(strNum);
  stringProb.AddOM(0.1,true,false);
  stringProb.AddOM(0.2,true,false);
  stringProb.AddOM(0.3,true,false);
  stringProb.AddOM(0.4,true,false); 
  ENSURE_DISTANCE(stringProb.GetProb(),log(0.0024), 1e-10, "You are calculating rubbish")
  ENSURE(strNum==4,"You skipped a test");
}
