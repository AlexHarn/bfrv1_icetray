#!/usr/bin/env python

# This attempts to extend the class
# and returns True if this fails, as it should.
# False means the class is extendable,
# which is not what we want
def ExtendClassFails(c):
    try:
        c.Foo = "bar"
        return False
    except:
        # this is what's expected
        return True

# loops through all the 'Boost.Python.class'es in a module
# the skip_test_list is provided to allow the user to skip certain
# classes.  Most likely for classes with non-trivial constructors,
# but also classes we might want keep dynamic.
# The string is the class name, i.e. "I3Particle" or "vector_I3MCHit"
def test_all_classes(mod, skip_test_list = list() ):
    import inspect
    from icecube.icetray.I3Test import *

    print type(mod)
    print dir(mod)
    # returns True if all of the class extensions fail
    return_value = True
    # loop over all the members of the
    msg_fmt = "Python dynamism is alive and well for (%s) "
    #print inspect.getmembers(sys)
    for k,v in inspect.getmembers(mod):
        # skip classes in this list        
        if k in skip_test_list : continue
        # check the type is a Boost.Python.class,
        # which is going to be anything we provided pybindings for
        if str(type(v)).find("Boost.Python.class") > 0 :
            try:
                # create and instance of the class
                # this will fail for classes with non-trivial constructors
                o = v()
                # now test it
                if not ExtendClassFails( o ) :
                    print msg_fmt % k
                    return_value = False
            except :
                print "You're on your own with this one : ", k
                return_value = False
                continue
    return return_value

                  
