.. _STTools_framework_st_configuration_scheme:

The ST configuration scheme
===========================

The ST configuration is based on individual DOM links, i.e. DOM pairs. Thus,
for a particular DOM pair the space and time requirements
can be configured. A particular ST setting can be applied to a list of DOM
links, e.g. IceCube-IceCube DOMs, IceCube-DeepCore DOMs, DeepCore-DeepCore DOMs.
STTools does not know about IC, DC, or PINGU DOMs. It is the responsibility of
the user to setup the correct ST configuration (i.e. list of DOM links for a
certain ST setting). However, the individual ST algorithms provide easy-to-use
configuration services.

In order to avoid the need for creating millions of DOM-to-DOM link combinations
by the user, the ST configuration scheme of STTools, uses the special classes
`OMKeySet` and `OMKeyLinkSet`. An OMKeySet defines a set of OMKeys, by providing
a list of string and DOM number ranges as strings:

.. sourcecode:: python

    # Defining DOMs 1 to 60 on strings 1 to 10 and 12.
    from icecube.STTools import OMKeySet
    omkeyset = OMKeySet(["1-10", "12"], ["1-60"])

Two OMKeySet objects define an OMKeyLinkSet specifying a set of DOM links:

.. sourcecode:: python

    from icecube.STTools import OMKeyLinkSet
    omkeylinkset = OMKeyLinkSet(omkeyset, omkeyset)

This creates a set of DOM links for DOMs 1 to 60 on strings 1 to 10 and 12, i.e.
all DOM-DOM link combinations for these DOMs.

The STConfiguration class (and its derived classes in the different ST
algorithms), defines the actual ST settings for a given list of
OMKeyLinkSet objects, i.e. a given list of DOM-DOM links. This means a specific
ST configuration is applied to the given list of DOM-DOM links.

Finally, the STConfigurationService class (and its derived class in the
particular ST algorithm implementation) takes a list of STConfiguration objects
to complete the ST configuration for the entire detector.

It is worth noting, that the STConfiguration class is derived from the
I3FrameObject class. Thus, ST configurations can be put into I3Frame objects to
store the ST configuration to disk.
