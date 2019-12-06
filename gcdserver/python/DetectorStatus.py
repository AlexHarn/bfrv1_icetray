
import icecube.gcdserver.Geometry as G

"""
file: Status.py
Contains all of the string keys for detector status documents along with
simple routines and classes to access the data.  This file is
designed to internalize all knowledge of detector status documents.
"""


class ObjectType(object):
    """
    Enumeration of DB object type strings
    """
    DOM_CONFIG_LIST = "DOM Configuration List"
    TRIGGER_CONFIG_LIST = "Trigger Configuration"
    RUN_CONFIG = "Run Configuration"


class DOMConfigList(G.DictionaryBacked):

    DOM_CONFIG_LIST_KEY = "domConfigs" 

    def __init__(self):
        super(DOMConfigList, self).__init__()
        self.getdict()[self.DOM_CONFIG_LIST_KEY] = []
        
    def addDOMConfig(self, value):
        self.getdict()[self.DOM_CONFIG_LIST_KEY].append(value)

    def getDOMConfigs(self):
        return self.getdict()[self.DOM_CONFIG_LIST_KEY]


class TriggerConfigList(G.DictionaryBacked):

    TRIGGER_CONFIG_LIST_KEY = "triggerConfigs" 

    def __init__(self):
        super(TriggerConfigList, self).__init__()
        self.getdict()[self.TRIGGER_CONFIG_LIST_KEY] = []
        
    def addTriggerConfig(self, value):
        self.getdict()[self.TRIGGER_CONFIG_LIST_KEY].append(value)

    def getTriggerConfigs(self):
        return self.getdict()[self.TRIGGER_CONFIG_LIST_KEY]


class RunConfig(G.DictionaryBacked):

    DOM_CONFIG_LIST_NAMES_KEY = "domConfigListNames"
    TRIGGER_CONFIG_LIST_NAME = "triggerConfigListName"
    
    def __init__(self):
        super(RunConfig, self).__init__()
        self.getdict()[self.DOM_CONFIG_LIST_NAMES_KEY] = []
        
    def addDOMConfigListName(self, value):
        self.getdict()[self.DOM_CONFIG_LIST_NAMES_KEY].append(value)

    def getDOMConfigListNames(self):
        return self.getdict()[self.DOM_CONFIG_LIST_NAMES_KEY]

    def setTriggerConfigListName(self, value):
        self.getdict()[self.TRIGGER_CONFIG_LIST_NAME] = value

    def getTriggerConfigListName(self):
        return self.getdict()[self.TRIGGER_CONFIG_LIST_NAME]
