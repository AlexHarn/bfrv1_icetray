
#include <icetray/I3FrameObject.h>
#include <icetray/load_project.h>

#include <boost/preprocessor.hpp>

//
//  Add the class you are registering to this list of parenthesized typenames.
//  Don't forget to watch that the newlines are backslashed.
//  To register class Nick, add (Nick) to the list below, add
//  Nick.cxx to the list of i3_add_library out in CMakeLists.txt,
//  and create a file Nick.cxx that contains a function 
//    void register_Nick();
//  that does the boost.python registration for class Nick.
//
#define REGISTER_THESE_THINGS						\
  (HealPixCoordinate)(TDST)

#define I3_REGISTRATION_FN_DECL(r, data, t) void BOOST_PP_CAT(register_,t)();
#define I3_REGISTER(r, data, t) BOOST_PP_CAT(register_,t)();
BOOST_PP_SEQ_FOR_EACH(I3_REGISTRATION_FN_DECL, ~, REGISTER_THESE_THINGS)

I3_PYTHON_MODULE(dst)
{
  load_project("icetray", false);
  load_project("dst", false);

  BOOST_PP_SEQ_FOR_EACH(I3_REGISTER, ~, REGISTER_THESE_THINGS);
}
