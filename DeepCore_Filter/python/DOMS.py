######################################################################
# Simple python class that stores the DOM lists for triggering,
# cleaning, veto region, etc...
#
# $Id$
# $Date$ $Revision$ $Author$
#######################################################################

##from icecube.DeepCore_Filter import DOMList, PINGUDOMList
from icecube.DeepCore_Filter import DOMList

class DOMS:
    def __init__(self, name):
        self.name = name

        # Create an empty list for the IceCubeStrings
        # to accomdate if a PINGU geometry is selected.
        self.allIceCubeStrings = []


        # For any PINGU geometry the IceCube geometry will be IC86EDC
        if name[:5] == "PINGU":

            self.allIceCubeStrings    = PINGUDOMList.get( name, "PINGUBreakoutStrings") + DOMList.get( "IC86EDC", "IceCubeStrings") #Inclusive IC+DC+PINGU
            self.PINGUStrings         = PINGUDOMList.get( name, "PINGUBreakoutStrings") + DOMList.get( "IC86EDC", "DeepCoreStrings") #Inclusive of DC+PINGU
            self.PINGUBreakoutStrings = PINGUDOMList.get( name, "PINGUBreakoutStrings") #Exclusive PINGU Strings
            self.PINGUFiducialDOMs    = PINGUDOMList.get( name, "PINGUFiducialDOMs") + DOMList.get( "IC86EDC", "DeepCoreFiducialDOMs")
            self.PINGUVetoDOMs        = DOMList.get( "IC86EDC", "DeepCoreVetoDOMs")
            name = "IC86EDC"

        else:
            self.allIceCubeStrings    = DOMList.get( name, "IceCubeStrings") #Inclusive of DeepCore Strings
        
        # Normal assignment of all the IceCube and DeepCore Strings and DOMs
        self.exclusiveIceCubeStrings = DOMList.get( name, "exclusiveIceCubeStrings") #Exclusive of DeepCore Strings
        self.DOMsToRemove            = DOMList.getDOMsToRemove( name)
        self.DeepCoreStrings         = DOMList.get( name, "DeepCoreStrings")
        self.DeepCoreBreakoutStrings = DOMList.get( name, "DeepCoreBreakoutStrings")
        self.DeepCoreFiducialDOMs    = DOMList.get( name, "DeepCoreFiducialDOMs")
        self.DeepCoreVetoDOMs        = DOMList.get( name, "DeepCoreVetoDOMs")
        self.DeepCoreICStrings       = DOMList.get( name, "DeepCoreICStrings")
            
        return

