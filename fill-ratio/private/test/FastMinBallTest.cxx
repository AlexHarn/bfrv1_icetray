
#include <I3Test.h>

#include "fill-ratio/FastMinBall.h"
#include "icetray/open.h"
#include "icetray/I3Frame.h"
#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>

namespace fs = boost::filesystem;

TEST_GROUP(FastMinBall);

static fs::path
GetDataDir()
{	
	ENSURE(getenv("I3_TESTDATA") != NULL,
	    "I3_TESTDATA must be defined in the parent shell.");

	const std::string I3_TESTDATA(getenv("I3_TESTDATA"));
	fs::path data_dir(I3_TESTDATA);
	
	ENSURE(fs::exists(data_dir), "Directory test data directory exists");
	
	return (data_dir);
}


I3GeometryConstPtr GetGeometry()
{
	fs::path gcdpath(GetDataDir() / "GCD/GeoCalibDetectorStatus_IC86.55697_corrected_V2.i3.gz");
	boost::iostreams::filtering_istream ifs;
	I3::dataio::open(ifs, gcdpath.string());
	I3Frame frame;
	frame.load(ifs);
	frame.load(ifs);
	
	return frame.Get<I3GeometryConstPtr>();
}
	
TEST(Basic)
{
	I3GeometryConstPtr geometry = GetGeometry();
	ENSURE((bool)geometry);
	
	FastMinBall baller(geometry->omgeo);
	I3Position p1(128.65094935953039, 119.71737535807279, 422.16222687353024);
	
	BOOST_FOREACH(const I3OMGeoMap::value_type &pair, baller.GetMinBallGeometry(p1, 111.48575692251376)) {
		
		const I3Position &p2 = pair.second.position;
		double d = hypot(hypot(p1.GetX()-p2.GetX(), p1.GetY()-p2.GetY()), p1.GetZ()-p2.GetZ());
		ENSURE(d <= 200.);
	}
}
