PortiaEventOMKeyConverter
=========================

a module to convert I3PortiaEvent of the largest charge into  OMKey

Usage
-----
  I3PortiaEventOMKeyConverter (C++ I3Module)

  Parameters:
    IcePickServiceKey
      Description : Key for an IcePick in the context that this module should check before processing physics frames.
      Default     : ''

    If
      Description : A python function... if this returns something that evaluates to True, Module runs, else it doesn't
      Default     : None

    InputPortiaEventName
      Description : Input name of I3PortiaEvent
      Default     : 'EHEPortiaEventSummary'

    OutputOMKeyListName
      Description : Output name of OMKey list
      Default     : 'LargestOMKey'
