#!/usr/bin/env python

from contexts import test_db_context, file_context
from TestData import (SNOW_DEPTH_XML, GEOMETRY_1_61,
                      GEOMETRY_1_62, GEOMETRY_12_65, GEOMETRY_TANK_1A)
import icecube.gcdserver.SnowDepthImport as SnowDepthImport
from icecube.gcdserver.I3MS import geoDBInserter
from icecube.gcdserver.MongoDB import fillBlobDB
from icecube.gcdserver.I3GeometryBuilder import buildI3Geometry
import icecube.gcdserver.Geometry as G
import icecube.gcdserver.Calibration as C
import icecube.gcdserver.GeometryDefaults as GeometryDefaults

from icecube.icetray import OMKey, I3Units
from icecube import dataclasses


def compareOMGeoData(key, val):
    data = GEOMETRY_1_61
    if key == OMKey(1, 62):
        data = GEOMETRY_1_62
    elif key == OMKey(12, 65):
        data = GEOMETRY_12_65
    o = G.GeometryObject.wrapdict(data)
    assert val.position.x == float(o.data[G.Keys.GEOMETRY_X] * I3Units.m)
    assert val.position.y == float(o.data[G.Keys.GEOMETRY_Y] * I3Units.m)
    assert val.position.z == float(o.data[G.Keys.GEOMETRY_Z] * I3Units.m)
    assert val.orientation.x == 0
    assert val.orientation.y == 0
    if key == OMKey(12, 65):
        # Scintillator
        assert val.omtype == dataclasses.I3OMGeo.Scintillator
        assert val.orientation.z == 1
        assert val.area == float(o.data[G.Keys.SENSOR_AREA] * I3Units.m2)
    else:
        assert val.omtype == dataclasses.I3OMGeo.IceTop
        assert val.orientation.z == -1
        assert val.area == GeometryDefaults.R7081PMTArea


def test_geometry():
    with test_db_context() as db:
        # Import snow depth for tank 1A
        with file_context(SNOW_DEPTH_XML) as inputFile:
            SnowDepthImport.doInsert(db, 1, None, [inputFile])
        # Import geometry data for string 1, DOMs 61 and 62 and a
        # scintillator
        with geoDBInserter(db) as geoInserter:
            geoInserter.insert(G.GeometryObject.wrapdict(GEOMETRY_1_61))
            geoInserter.insert(G.GeometryObject.wrapdict(GEOMETRY_1_62))
            geoInserter.insert(G.GeometryObject.wrapdict(GEOMETRY_12_65))
            geoInserter.insert(G.GeometryObject.wrapdict(GEOMETRY_TANK_1A))
            geoInserter.commit()
        # Get the BlobDB instance
        geoDB = fillBlobDB(db, run=1)
        # Build I3Geometry
        geo = buildI3Geometry(geoDB)
        # Ensure the geometry frame was built correctly
        assert len(geo.omgeo) == 3
        for (key, val) in geo.omgeo:
            assert key in [OMKey(1, 61), OMKey(1, 62), OMKey(12, 65)]
            compareOMGeoData(key, val)
        # Now check the tank object
        assert len(geo.stationgeo) == 1
        for (key, stationgeo) in geo.stationgeo:
            assert key == 1
            assert len(stationgeo) == 1
            tankgeo = stationgeo[0]
            assert tankgeo.snowheight == 0.077
            assert tankgeo.tankheight == GeometryDefaults.IceTopTankHeight
            assert tankgeo.tankradius == GeometryDefaults.IceTopTankRadius
            assert tankgeo.fillheight == GeometryDefaults.IceTopTankFillHeight
            assert tankgeo.tanktype == dataclasses.I3TankGeo.Zirconium_Lined
            assert len(tankgeo.omkey_list) == 2
            assert OMKey(1, 61) in tankgeo.omkey_list
            assert OMKey(1, 62) in tankgeo.omkey_list

            def av(o1, o2, key):
                return 0.5 * (float(o1["data"][key]) + float(o2["data"][key]))

            assert tankgeo.position.x == av(GEOMETRY_1_61, GEOMETRY_1_62, "x")
            assert tankgeo.position.y == av(GEOMETRY_1_61, GEOMETRY_1_62, "y")
            assert tankgeo.position.z == av(GEOMETRY_1_61, GEOMETRY_1_62, "z")


if __name__ == "__main__":
    test_geometry()    