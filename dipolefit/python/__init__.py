# 
# copyright  (C) 2010
# The Icecube Collaboration
# 
# $Id: __init__.py 74499 2011-04-26 20:33:59Z kislat $
# 
# @version $Revision: 74499 $
# @date $LastChangedDate: 2011-04-26 16:33:59 -0400 (Tue, 26 Apr 2011) $
# @author Jakob van Santen <vansanten@wisc.edu> $LastChangedBy: kislat $
# 

# pull in dependencies
import icecube.recclasses
from icecube.icetray import load
load('dipolefit', False)

try:
    import icecube.tableio
    import converters
except ImportError:
    pass