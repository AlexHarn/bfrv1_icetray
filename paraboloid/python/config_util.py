if __name__ == "__main__":
    print("this is an auxiliary script for python module/service configuration")
    sys.exit(0)

from icecube.icetray import logging

# In C++ you get this functionality of this function directly with GetParameter
# Would be nice to have this also in the python module interface.
def get_service_by_name_or_by_object(configurable,parname,service_pybase_class=None,service_cppbase_class=None,optional=True):
    s = configurable.GetParameter(parname)
    if optional and s is None:
        return s
    elif type(s) == str:
        if s in configurable.context.keys():
            logging.log_debug("getting %s from context with name %s" % (parname,s),unit=configurable.name)
            service=configurable.context[s]
            if isinstance(service,service_pybase_class) or isinstance(service,service_cppbase_class):
                return service
            else:
                logging.log_fatal("got %s from context with name %s but it seems to have the wrong type '%s', expecting %s or %s" %
                                  (parname,s,type(service),service_pybase_class, service_cppbase_class),
                                  unit=configurable.name)
        else:
            logging.log_fatal("got %s for '%s' but this name does not exist in the context, available names are %s" %
                              (s,parname,",".join(configurable.context.keys())),
                              unit=configurable.name)
    elif isinstance(s,service_pybase_class) or isinstance(s,service_cppbase_class):
        logging.log_debug("got %s object directly" % parname,unit=configurable.name)
    else:
        logging.log_fatal("got %s but it seems to have an unexpected type: '%s', expected types are: string, %s or %s" %
                          (parname,type(s),service_pybase_class,service_cppbase_class),
                          unit=configurable.name)
    return s
