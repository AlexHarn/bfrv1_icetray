/**
    copyright  (C) 2004
    the icecube collaboration
    $Id$

    @version $Revision: 1.2 $
    @date $Date$
    @author S Robbins <robbins@physik.uni-wuppertal.de>

    @brief Test assertion macro AssertX and IPdfException, which 
    uses this macro.
*/

#include <I3Test.h>
#include "ipdf/Utilities/IPdfException.h"
#include <string>
using std::string;
using std::cout;
using std::endl;
using IPDF::IPdfException;

TEST_GROUP(IPdfExceptionTest)

  ///
  /// Test IPdfException base class
TEST(base)
  {
    try {
      AssertX(false,IPdfException);
    } catch (const IPdfException& exp) {
      // SUCCESS
      return;
    } catch (const std::exception& exp) {
      FAIL("Caught std::exception but failed to catch IPdfException.");
    } catch (...) {
      FAIL("FATAL - unidentified exception.");
    }
    FAIL("Expected exception not thrown."); /// @todo Fix AssertX on Mac
  }
  
  ///
  /// Test a IPdfException derived class
TEST(derived)
  {
    try {
      AssertX(false,IPDF::NotImplemented);
    } catch (const IPdfException& exp) {
      // SUCCESS
      return;
    } catch (const std::exception& exp) {
      FAIL("Caught std::exception but failed to catch IPdfException.");
    } catch (...) {
      FAIL("FATAL - unidentified exception.");
    }
    FAIL("Expected exception not thrown."); /// @todo Fix AssertX on Mac
  }
  
  ///
  /// Test AssertX for true case
TEST(true)
  {
    try {
      AssertX(true,IPdfException);
    } catch (...) {
      FAIL("AssertX(true) threw :-(");
    }
  }
