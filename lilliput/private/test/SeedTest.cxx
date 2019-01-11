/**
    copyright  (C) 2007
    the icecube collaboration
    $Id$

    @version $Revision$
    @date $Date$
    @author boersma
*/

// generic stuff
#include <I3Test.h>
#include "icetray/I3Units.h"
#include "dataclasses/physics/I3Particle.h"

// gulliver stuff
#include "lilliput/seedservice/I3BasicSeedService.h"

TEST_GROUP(SeedTest);

TEST(Reverse)
{
    double tolerance = 0.0001;
    I3ParticlePtr p(new I3Particle);
    p->SetPos(1,2,3);
    for ( int izen = 0; izen <= 180; izen += 30 ){
        for ( int iazi = 0; iazi <= 360; iazi += 60 ){
            p->SetDir( izen * I3Units::degree, iazi * I3Units::degree );
            I3Direction oridir = p->GetDir();
            std::vector<I3ParticlePtr> v( I3BasicSeedService::Reverse(p) );
            ENSURE( v.size() == 1, "failed to get opposite alternative" );
            I3Direction newdir = v[0]->GetDir();
            ENSURE_DISTANCE( newdir.GetX() + oridir.GetX(), 0.0, tolerance,
                             "reversal problem: X not opposite!" );
            ENSURE_DISTANCE( newdir.GetY() + oridir.GetY(), 0.0, tolerance,
                             "reversal problem: Y not opposite!" );
            ENSURE_DISTANCE( newdir.GetZ() + oridir.GetZ(), 0.0, tolerance,
                             "reversal problem: Z not opposite!" );
            if ( izen == 0 || izen == 180 ) break;
        }
    }
}

TEST(Tetrahedron)
{
    // distance (squared) between tetrahedric points on a unit sphere
    // const double checkdist = 2.0*sqrt(6.0)/3.0;
    const double checkdist2 = 8.0/3.0;
    const double tolerance = 0.0001;
    I3ParticlePtr p(new I3Particle);
    p->SetPos(1,2,3); // irrelevant
    double yesterday = -86400*I3Units::second; // irrelevant
    p->SetTime(yesterday); // irrelevant
    for ( int izen = 0; izen <= 180; izen += 30 ){
        for ( int iazi = 0; iazi <= 180; iazi += 30 ){
            p->SetDir( izen * I3Units::degree, iazi * I3Units::degree );
            std::vector<I3ParticlePtr> v( I3BasicSeedService::Tetrahedron(p) );
            ENSURE( v.size() == 3, "failed to get tetrahedric alternatives" );
            v.push_back(p);
            std::vector<I3ParticlePtr>::iterator p1 = v.begin();
            while ( p1 != v.end() ){
                const I3Direction &dir1 = (*p1)->GetDir();
                std::vector<I3ParticlePtr>::iterator p2 = p1;
                while ( ++p2 != v.end() ){
                    const I3Direction &dir2 = (*p2)->GetDir();
                    double dx = dir1.GetX() - dir2.GetX();
                    double dy = dir1.GetY() - dir2.GetY();
                    double dz = dir1.GetZ() - dir2.GetZ();
                    ENSURE_DISTANCE( dx*dx+dy*dy+dz*dz, checkdist2, tolerance,
                             "tetrahedron problem: wrong distance!" );
                }
                ++p1;
            }
            if ( izen == 0 || izen == 180 ) break;
        }
    }
}

TEST(Cube)
{

    // inner products of 15 combinations of cube directions
    // reverse
    // perp1
    // reverse of perp1
    // perp2
    // reverse of perp2
    // original
    const double checklist[15] = {
        0,0,0,0,-1,
       -1,0,0,0,
        0,0,0,
        -1,0,
        0};
    const double tolerance = 0.0001;

    I3ParticlePtr p(new I3Particle);
    p->SetPos(1,2,3); // irrelevant
    double yesterday = -86400*I3Units::second; // irrelevant
    p->SetTime(yesterday); // irrelevant
    for ( int izen = 0; izen <= 180; izen += 30 ){
        for ( int iazi = 0; iazi <= 180; iazi += 30 ){
            p->SetDir( izen * I3Units::degree, iazi * I3Units::degree );
            std::vector<I3ParticlePtr> v( I3BasicSeedService::Cube(p) );
            ENSURE( v.size() == 5, "failed to get cubic alternatives" );

            // check all 15 combinations
            v.push_back(p);
            std::vector<I3ParticlePtr>::iterator p1 = v.begin();
            int ichk=0;
            while ( p1 != v.end() ){
                const I3Direction &dir1 = (*p1)->GetDir();
                std::vector<I3ParticlePtr>::iterator p2 = p1;
                while ( ++p2 != v.end() ){
                    const I3Direction &dir2 = (*p2)->GetDir();
                    double dotprod =   dir1.GetX() * dir2.GetX()
                                     + dir1.GetY() * dir2.GetY()
                                     + dir1.GetZ() * dir2.GetZ();
                    ENSURE_DISTANCE( dotprod, checklist[ichk++], tolerance,
                                     "cube problem: wrong dot product!" );
                }
                ++p1;
            }
            if ( izen == 0 || izen == 180 ) break;
        }
    }
}
