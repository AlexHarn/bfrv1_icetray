#ifndef I3CYLINEDERMAP_H_INCLUDED 
#define I3CYLINDERMAP_H_INCLUDED

#include <map>
#include <icetray/OMKey.h>
#include <clsim/shadow/I3ExtraGeometryItemCylinder.h>


typedef std::map<OMKey, I3ExtraGeometryItemCylinder > I3CylinderMap;

I3_POINTER_TYPEDEFS(I3CylinderMap);

#endif
