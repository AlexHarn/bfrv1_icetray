#!/usr/bin/env python

import os
import sys
import xml.etree.ElementTree as ElementTree

import icecube.gcdserver.Geometry as G
import icecube.gcdserver.DetectorStatus as D
from icecube.gcdserver.OptionParser import GCDOptionParser
from icecube.gcdserver.MongoDB import getDB
from icecube.gcdserver.XMLDict import XMLDict
from I3MS import statusDBInserter

"""
Export run configuration XML data into the mongoDB status database.
"""

def importActiveTriggers(inserter, triggerFile, name):

    # Parse the trigger XML file
    tree = ElementTree.parse(triggerFile)
    root = tree.getroot()
    if root.tag != "activeTriggers":
        raise Exception("Unable to find activeTriggers tag in %s" %
                                                            triggerFile)

    o = D.TriggerConfigList()
    for e in root.findall('triggerConfig'):
        o.addTriggerConfig(XMLDict(e))
    result = G.DataObject(name, D.ObjectType.TRIGGER_CONFIG_LIST, o.getdict())
    inserter.insert(result)


def importDOMConfigList(inserter, domConfigListFile, name):

    # Parse the DOM config XML file
    tree = ElementTree.parse(domConfigListFile)
    root = tree.getroot()
    if root.tag != "domConfigList":
        raise Exception("Unable to find domConfigList tag in %s" %
                                                     domConfigListFile)

    o = D.DOMConfigList()
    for e in root.findall('domConfig'):
        o.addDOMConfig(XMLDict(e))

    result = G.DataObject(name, D.ObjectType.DOM_CONFIG_LIST, o.getdict())
    inserter.insert(result)


def importConfig(inserter, inputFile):

    # Parse the run config XML file
    tree = ElementTree.parse(inputFile)
    root = tree.getroot()
    if root.tag != "runConfig":
        raise Exception("Unable to find runConfig tag")

    runConfigFileName = os.path.basename(inputFile)
    if runConfigFileName.endswith(".xml"):
        runConfigFileName = runConfigFileName[:-4]
    o = D.RunConfig()
    dirName = os.path.dirname(inputFile)
    if dirName == '':
        # User is importing directly from the config directory
        dirName = os.getcwd()

    # Get the trigger config
    triggerConfig = root.find('triggerConfig')
    if triggerConfig is None:
        raise Exception("No trigger configuration found")
    triggerConfigName = triggerConfig.text
    o.setTriggerConfigListName(triggerConfigName)
    triggerFile = dirName + '/trigger/' + triggerConfigName + ".xml"
    importActiveTriggers(inserter, triggerFile, triggerConfigName)

    # Get the DOM configs: Need both 'domConfigList' and 'stringHub' elements
    # to support multiple versions of the runConfig file
    domConfigFileNames = [e.text for e in root.findall('domConfigList')]
    domConfigFileNames.extend([e.attrib['domConfig'] for e in
                                          root.findall('stringHub')])
    for name in domConfigFileNames:
        o.addDOMConfigListName(name)
        domConfigListFile = dirName + '/domconfigs/' + name + ".xml"
        importDOMConfigList(inserter, domConfigListFile, name)
 
    inserter.insert(G.DataObject(runConfigFileName,
                                 D.ObjectType.RUN_CONFIG, o.getdict()))


def doInsert(db, configFiles, i3msHost, force=False):

    errCode = 0
    with statusDBInserter(db, i3msHost, force=force) as inserter:
        for configFile in configFiles:
            try:
                importConfig(inserter, configFile)
                inserter.commit()
            except Exception as e:
                errCode = -1
                print("Unable to import file %s: %s" % (configFile, e))
    return errCode


if __name__ == "__main__":
    parser = GCDOptionParser()
    parser.add_option("-f", "--force", dest="force",
                      action="store_true", default=False,
                      help="Force load configuration if I3MS is unavailable")
    (options, args) = parser.parse_args()
    if len(args) < 1:
        print("Usage: ConfigImport.py configurationFile1 configurationFile2 ...")
        parser.print_help()
        sys.exit(-1)
    errCode = doInsert(getDB(options.dbhost, options.dbuser, options.dbpass),
                       args, options.i3mshost, force=options.force)
    sys.exit(errCode)
