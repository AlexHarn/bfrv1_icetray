# 
# copyright  (C) 2010
# The Icecube Collaboration
# 
# $Id: __init__.py 74503 2011-04-26 20:56:53Z kislat $
# 
# @version $Revision: 74503 $
# @date $LastChangedDate: 2011-04-26 16:56:53 -0400 (Tue, 26 Apr 2011) $
# @author Jakob van Santen <vansanten@wisc.edu> $LastChangedBy: kislat $
# 
from icecube import icetray, recclasses
icetray.load('fill-ratio', False)

from .FillRatio import FillRatioModule

try:
    import icecube.tableio
    import converters
except ImportError:
    pass
