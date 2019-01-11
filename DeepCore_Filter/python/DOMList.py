######################################################################
#
# The following python module contains the lists of DOMs for
# triggering, cleaning etc... that are set by the run year to be 
# analyzed.
#
# $Id$
# $Date$ $Revision$ $Author$
#
######################################################################

from I3Tray import *


######################################################################
######################################################################
#
# Functions
#
######################################################################
######################################################################



####################
# DOM selection for either the 6 DC strings or
# the surrounding layer of IceCube DOMs
####################

def getDeepCoreFiducialDOMs( ICConfig):

    DOMList = []

    # For certain ICConfigs there are no DeepCore DOMs
    if not ICConfig in DeepCoreStringsDict:
        return DOMList

    # The fiducial volume of DeepCore is the bottom 50 DOMs
    # for the 6 DC strings and the bottom 23 for the 7 
    # IceCube strings. Strings 79 and 80 have been moved
    # to be infill strings within DeepCore and have the
    # DeepCore 10/50 DOM breakouts. 
    
    for s in DeepCoreStringsDict[ ICConfig]:
        # In the DeepCore fiducial volume in the IC59 geometry
        # then used DOMs 41--60, otherwise use 39--60.
        if 0 < s and s <= 78:
            if ICConfig == "IC59":
              for DOMNumber in range( 41, 61):
                  DOMList.append( OMKey( s, DOMNumber))
            else:
              for DOMNumber in range( 39, 61):
                  DOMList.append( OMKey( s, DOMNumber))
        elif 79 <= s and s <= 86:
            for DOMNumber in range( 11, 61):
                DOMList.append( OMKey( s, DOMNumber))


    return tuple( DOMList)




####################
# DOM selection for the veto region of DeepCore
####################
def getDeepCoreVetoDOMs( ICConfig):
    
    DCDOMs = getDeepCoreFiducialDOMs( ICConfig)
    DCVeto = []

    # Loop through all the DOMs in the selected
    # run configuration and select the DOMs that
    # are inverse from the DeepCore fiducial DOMs

    for string in IceCubeStringsDict[ ICConfig]:
        for DOMNumber in range( 1, 61):
            testOMKey = OMKey( string, DOMNumber) 
            if not testOMKey in DCDOMs:
                DCVeto.append( testOMKey)

    return tuple( DCVeto)



####################
# Get a selection of IceCube strings that
# do not contain the DeepCore strings with the
# 10/50 DOM breakout pattern. Some of the 
# DAQ triggers, such as the 'string trigger',
# do not use the special DeepCore strings.
#################### 

def getExclusiveIceCubeStrings( ICConfig):

    exclIceCubeStrings = []

    # If there are no special DeepCore strings in the
    # run configuration then return an empty tuple

    if not ICConfig in DeepCoreBreakoutStringsDict:
        return tuple( exclIceCubeStrings)


    # Iterate over the IceCube strings and select the 
    # the inverse all the strings exlusive of the
    # special DeepCore strings

    for string in IceCubeStringsDict[ ICConfig]:
        if not string in DeepCoreBreakoutStringsDict[ ICConfig]:
            exclIceCubeStrings.append( string)
            
    return tuple( exclIceCubeStrings)




####################
# Generic Get() function that returns the 
# list of OMKeys, DOMs, strings etc... that is 
# requested
####################

def get( runConfig, listKey):

    if not listKey in masterDict:
        print("FAIL: No DOM/String dictionary of name " + listKey + " found in the master DOMList dictionary")
        return []
    else:
        desiredDict = masterDict[ str(listKey)]
        return tuple( desiredDict[ str(runConfig)] )


    

####################
# Selection of DOMs to be cleaned 
# for the particular IceCube geometry configuration
####################

def getDOMsToRemove( ICConfig):

    if ICConfig in removeDOMsDict:
        return tuple( removeDOMsDict[ ICConfig])
    else:
        return


######################################################################
######################################################################
#
# Tuples of all the various DOMs or strings that could be selected.
#
######################################################################
######################################################################


####################
# Dictionary of the DOMs on the IceCube string 
# that are part of 'DeepCore'. 
####################

DeepCoreStringsDict = { "IC59":    ( 26, 27, 36, 37, 45, 46, 83),
                        "IC79":    ( 26, 27, 35, 36, 37, 45, 46, 81, 82, 83, 84, 85, 86),
                        "IC86":    ( 26, 27, 35, 36, 37, 45, 46, 79, 80, 81, 82, 83, 84, 85, 86),
                        "ICTEST":  ( 26, 27, 35, 36, 37, 45, 46, 81, 82, 83, 84, 85, 86),
                        "IC86EDC": ( 25, 26, 27, 34, 35, 36, 37, 44, 45, 46, 47, 54, 79, 80, 81, 82, 83, 84, 85, 86),
                        "IC86TwoLayVeto": ( 16, 17, 18, 19, 24, 25, 26, 27, 28, 33, 34, 35, 36, 37, 38, 43, 44, 45, 46, 47, 48, 53, 54, 55, 56, 57, 62, 63, 79, 80, 81, 82, 83, 84, 85, 86),
                        }

# Deepcore strings with the 10/50 breakout pattern
DeepCoreBreakoutStringsDict = { "IC59":    (83,),
                                "IC79":    (81, 82, 83, 84, 85, 86,),
                                "IC86":    (79, 80, 81, 82, 83, 84, 85, 86,),
                                "ICTEST":  (81, 82, 83, 84, 85, 86,),
                                "IC86EDC": (79, 80, 81, 82, 83, 84, 85, 86,),
                                "IC86TwoLayVeto": (79, 80, 81, 82, 83, 84, 85, 86,),
                                }



####################
# IceCube strings
####################

IC1_Strings     = (21,)
IC9_Strings     = IC1_Strings + ( 29, 30, 38, 39, 40, 49, 50, 59)
IC22_Strings    = IC9_Strings + ( 46, 47, 48, 56, 57, 58, 65, 66, 67, 72, 73, 74, 78)
IC40_Strings    = IC22_Strings + ( 44, 45, 52, 53, 54, 55, 60, 61, 62, 63, 64, 68, 69, 70, 71, 75, 76, 77)
IC59_Strings    = IC40_Strings + ( 2, 3, 4, 5, 6, 10, 11, 12, 13, 17, 18, 19, 20, 26, 27, 28, 36, 37, 83)
IC79_Strings    = IC59_Strings + ( 8, 9, 15, 16, 23, 24, 25, 32, 33, 34, 35, 41, 42, 43, 51, 81, 82, 84, 85, 86)
IC86_Strings    = IC79_Strings + ( 1, 7, 14, 22, 31, 79, 80)

IceCubeStringsDict = { "IC1":     IC1_Strings,
                       "IC9":     IC9_Strings,
                       "IC22":    IC22_Strings,
                       "IC40":    IC40_Strings,
                       "IC59":    IC59_Strings,
                       "IC79":    IC79_Strings,
                       "IC86":    IC86_Strings,
                       "ICTEST":  IC79_Strings,
                       "IC86EDC": IC86_Strings,
                       "IC86TwoLayVeto": IC86_Strings,
                       }

exclusiveIceCubeStringsDict = { "IC1":            getExclusiveIceCubeStrings( "IC1"),
                                "IC9":            getExclusiveIceCubeStrings( "IC9"),
                                "IC22":           getExclusiveIceCubeStrings( "IC22"),
                                "IC40":           getExclusiveIceCubeStrings( "IC40"),
                                "IC59":           getExclusiveIceCubeStrings( "IC59"),
                                "IC79":           getExclusiveIceCubeStrings( "IC79"),
                                "IC86":           getExclusiveIceCubeStrings( "IC86"),
                                "ICTEST":         getExclusiveIceCubeStrings( "ICTEST"),
                                "IC86EDC":        getExclusiveIceCubeStrings( "IC86EDC"),
                                "IC86TwoLayVeto": getExclusiveIceCubeStrings( "IC86TwoLayVeto"),
                                }

####################
# Dead DOMs that are not expected
# to ever be recovered
####################

uselessDOMs = ( 
    OMKey(8,59),#  "Euonymus_Alatus" 
    OMKey(8,60),#  "inculcate" 
    OMKey(9,16),#  "The_Penguin" 
    OMKey(18,46),#  "Triquetrum" 
    OMKey(24,56),#  "Tetilla" 
    OMKey(26,46),#  "Melian" 
    OMKey(29,59),#  "Auroraphobia" 
    OMKey(29,60),#  "Nix" 
    OMKey(30,23),#  "Peugeot_505" 
    OMKey(30,60),#  "Rowan" 
    OMKey(32,57),#  "Dromornis" 
    OMKey(32,58),#  "Sde_Boker" 
    OMKey(39,22),#  "Liljeholmen" 
    OMKey(34,11),#  "Chipsat" 
    OMKey(34,12),#  "Diaphyse" 
    OMKey(34,13),#  "Gianni_Schicchi" 
    OMKey(34,14),#  "Bohnebeitel" 
    OMKey(34,15),#  "Frihult" 
    OMKey(34,17),#  "Gymnastics" 
    OMKey(34,22),#  "Gickel" 
    OMKey(42,47),#  "Bancha" 
    OMKey(42,48),#  "Misu" 
    OMKey(50,36),#  "Ocelot" 
    OMKey(54,47),#  "Garbanzo_bean" 
    OMKey(69,44),#  "Niue" 
    OMKey(69,47),#  "New_Britain" 
    OMKey(69,48),#  "Rawaki" 
    OMKey(71,39),#  "Beaver" 
    OMKey(74,9),#  "Immunology" 
    OMKey(86,27),#  "Gaffelfibbla" 
    OMKey(86,28),#  "Dill" 
    ) 

####################
# Dead DOMs that have a low
# likelihood to ever be recovered
####################

almostUselessDOMs = ( 
    OMKey(6,11),#  "Discworld" 
    OMKey(38,59),#  "Blackberry" 
    OMKey(39,61),#  "Hydrogen" 
    OMKey(68,42),#  "Krabba" 
    ) 

####################
# DOMs that are plugged into wczar that
# have a reasonable chance of being
# reintroduced to data taking
####################
    
nonStringHubDOMs_2010 = ( 
    OMKey(5,5),#  "Thavil" 
    OMKey(5,6),#  "Drill_Chuck" 
    OMKey(9,15),#  "Daryan" 
    OMKey(11,1),#  "Scaphoideum" 
    OMKey(11,2),#  "Nissan" 
    OMKey(18,45),#  "Eslamabad" 
    OMKey(19,59),#  "Dromophobia" 
    OMKey(19,60),#  "Coxae" 
    OMKey(27,7),#  "Vicuna" 
    OMKey(27,8),#  "Hu" 
    OMKey(28,57),#  "Cacofonix" 
    OMKey(28,59),#  "Zarand" 
    OMKey(39,22),#  "Liljeholmen" 
    OMKey(40,51),#  "Juneberry" 
    OMKey(40,52),#  "Alfa_Romeo_Spider" 
    OMKey(44,25),#  "Mirkwood" 
    OMKey(44,26),#  "Pink" 
    OMKey(44,47),#  "Homeopatix" 
    OMKey(44,48),#  "Mad_Hatter" 
    OMKey(47,55),#  "Kemps_Card" 
    OMKey(47,56),#  "Ensta" 
    OMKey(51,27),#  "Socialsekreterare" 
    OMKey(51,28),#  "Danbo" 
    OMKey(53,17),#  "Bryozoan" 
    OMKey(53,18),#  "Fa" 
    OMKey(54,51),#  "Dendrophilia" 
    OMKey(54,52),#  "Raketost" 
    OMKey(59,45),#   "Essex_6" 
    OMKey(59,46),#   "Pichelsberg"  
    OMKey(60,55),#  "Schango" 
    OMKey(60,56),#  "Marscapone" 
    OMKey(62,51),#  "Ottawa_Tribe" 
    OMKey(62,52),#  "Sauron" 
    OMKey(66,33),#  "New_York" 
    OMKey(66,34),#  "Dou_Mu" 
    OMKey(66,45),#  "Alpaca" 
    OMKey(66,46),#  "Tallahassee" 
    OMKey(69,23),#  "Black" 
    OMKey(69,24),#  "Magenta" 
    ) 

testRemovedDOMs = ( OMKey(38,59),# Blackberry
                    OMKey(39,61),# Hydrogen
                    OMKey(39,21),# Aspudden
                    OMKey(39,22),# Liljeholmen
                    OMKey(40,51),# Juneberry
                    OMKey(40,52),# Alfa_Romeo_Spider
                    OMKey(44,25),# Mirkwood
                    OMKey(44,26),# Pink
                    OMKey(44,47),# Homeopatix
                    OMKey(44,48),# Mad_Hatter
                    OMKey(44,55),# Gaston
                    OMKey(44,56),# Ermolai
                    OMKey(46,49),# Gerbil
                    OMKey(46,50),# Ringkobben
                    OMKey(47,55),# Kemps_Card
                    OMKey(47,56),# Ensta
                    OMKey(53,17),# Bryozoan
                    OMKey(53,18),# Fa
                    OMKey(66,33),# New_York 
                    OMKey(66,34),# Dou_Mu
                    OMKey(66,45),# Alpaca
                    OMKey(66,46),# Tallahassee
                    OMKey(69,23),# Black
                    OMKey(69,24),# Magenta
                    OMKey(72,37),# Hundgrundet
                    OMKey(72,38),# Buttercup
                    OMKey(77,47),# Hamster
                    OMKey(77,48),# Meldii
                    OMKey(36,44),# Everglades (HQE)
                    OMKey(36,46),# Kuro_Usagi (HQE)
                    OMKey(36,48),# The_Badlands (HQE)           
                    OMKey(36,49),# Congaree (HQE)
                    OMKey(36,50),# Grand_Canyon (HQE) 
                    OMKey(36,51),# Tsuki_Usagi (HQE)
                    OMKey(36,52),# Wind_Cave (HQE)
                    OMKey(36,53),# Crater_Lake (HQE)
                    OMKey(36,54),# Mojave_Desert (HQE)
                    OMKey(36,55),# Carlsbad_Caverns (HQE)
                    OMKey(36,56),# Sequoia (HQE)
                    OMKey(36,57),# Isle_Royale (HQE)
                    OMKey(36,58),# Cuyahoga_Valley (HQE)
                    OMKey(36,59),# Joshua_Tree (HQE)
                    OMKey(87,1),	
                    OMKey(87,2),
                    OMKey(87,3),
                    OMKey(87,4),
                    OMKey(87,6),
                    OMKey(87,8),
                    OMKey(87,21),   
                    )

testRemovedDOMs_1 = ( OMKey(38,59),# Blackberry 
                      OMKey(6,11),  # Discworld - Meteor DOM
                      OMKey(68,42)  # Krabba
                      )


removeDOMsDict = { "IC1":     uselessDOMs + almostUselessDOMs + nonStringHubDOMs_2010,
                   "IC9":     uselessDOMs + almostUselessDOMs + nonStringHubDOMs_2010,
                   "IC22":    uselessDOMs + almostUselessDOMs + nonStringHubDOMs_2010,
                   "IC40":    uselessDOMs + almostUselessDOMs + nonStringHubDOMs_2010,
                   "IC59":    uselessDOMs + almostUselessDOMs + nonStringHubDOMs_2010,
                   "IC79":    uselessDOMs + almostUselessDOMs + nonStringHubDOMs_2010,
                   "IC86":    uselessDOMs + almostUselessDOMs + nonStringHubDOMs_2010,
                   "ICTEST":  testRemovedDOMs_1,
                   "IC86EDC":  uselessDOMs + almostUselessDOMs + nonStringHubDOMs_2010,
                   "IC86TwoLayVeto":  uselessDOMs + almostUselessDOMs + nonStringHubDOMs_2010,
                   }


DeepCoreFiducialDOMsDict = { "IC59":    getDeepCoreFiducialDOMs( "IC59"),
                             "IC79":    getDeepCoreFiducialDOMs( "IC79"),
                             "IC86":    getDeepCoreFiducialDOMs( "IC86"),
                             "ICTEST":  getDeepCoreFiducialDOMs( "ICTEST"),
                             "IC86EDC": getDeepCoreFiducialDOMs( "IC86EDC"),
                             "IC86TwoLayVeto": getDeepCoreFiducialDOMs( "IC86TwoLayVeto"),
                             }


DeepCoreVetoDOMsDict = { "IC59":    getDeepCoreVetoDOMs( "IC59"),
                         "IC79":    getDeepCoreVetoDOMs( "IC79"),
                         "IC86":    getDeepCoreVetoDOMs( "IC86"),
                         "ICTEST":  getDeepCoreVetoDOMs( "ICTEST"),
                         "IC86EDC":    getDeepCoreVetoDOMs( "IC86EDC"),
                         "IC86TwoLayVeto":    getDeepCoreVetoDOMs( "IC86TwoLayVeto"),
                         }

DeepCoreICStringsDict = { "IC59":    (26, 27, 36, 37, 45, 46),
                          "IC79":    (26, 27, 35, 36, 37, 45, 46),
                          "IC86":    (26, 27, 35, 36, 37, 45, 46),
                          "ICTEST":  (26, 27, 35, 36, 37, 45, 46),
                          "IC86EDC": (25, 26, 27, 34, 35, 36, 37, 44, 45, 46, 47, 54),
                          "IC86TwoLayVeto": (16, 17, 18, 19, 24, 25, 26, 27, 28, 33, 34, 35, 36, 37, 38, 43, 44, 45, 46, 47, 48, 53, 54, 55, 56, 57, 62, 63),
                          }

######################################################################
######################################################################
#
# Master dictionary containing every dictionary in
# the DOMList python module. This allows for a generic
# get() function that only requires as input the name of dictionary.
# 
#
######################################################################
######################################################################

masterDict = { "IceCubeStrings":          IceCubeStringsDict,
               "exclusiveIceCubeStrings": exclusiveIceCubeStringsDict,
               "DeepCoreStrings":         DeepCoreStringsDict,
               "DeepCoreBreakoutStrings": DeepCoreBreakoutStringsDict,
               "removeDOMs":              removeDOMsDict,
               "DeepCoreFiducialDOMs":    DeepCoreFiducialDOMsDict,
               "DeepCoreVetoDOMs":        DeepCoreVetoDOMsDict,
               "DeepCoreICStrings":       DeepCoreICStringsDict,
               }
               
