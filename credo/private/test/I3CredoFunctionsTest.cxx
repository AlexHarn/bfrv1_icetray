/*
 *  $Id$
 *  @version $Revision$
 *  @date $Date$
 *  @author emiddell
*/

#include <I3Test.h>
#include "credo/I3CredoFunctions.h"
#include "dataclasses/physics/I3RecoPulse.h"

TEST_GROUP(I3CredoFunctions);
    
TEST(GetPhotonicsCorrectionFactor) {
    // test that for several input combinations the correction factor is in [0,1]
    double corrfactor = 0;

    // create a list of amplitude predictions, including challenging values
    std::vector<double> amplitudes;
    std::vector<double>::const_iterator i_amp;
    for (double i = -4; i < 4; i +=.5)
        amplitudes.push_back(pow(10,i));

    amplitudes.push_back(0.);
    amplitudes.push_back(-1.);
    amplitudes.push_back(NAN);
    
    // create a list of distances, including challenging values
    std::vector<double> distances;
    std::vector<double>::const_iterator i_dist;
    distances.push_back(NAN);
    for (double dist=-100; dist < 500; dist += 100) 
        distances.push_back(dist);

    for (i_dist = distances.begin(); i_dist != distances.end(); ++i_dist) {
        const double dist = *i_dist;
        for (i_amp = amplitudes.begin(); i_amp != amplitudes.end(); ++i_amp) {
            const double amplitude = *i_amp;

            // IceCube ATWD+FADC
            corrfactor= I3Credo::GetPhotonicsCorrectionFactor(I3CredoDOMCache::ICECUBEDOM,
                                                          amplitude,
                                                          dist,
                                                          false); // ATWD only
            ENSURE( (0 <= corrfactor) && (corrfactor <= 1) , "correction factor should be in [0,1]");
            
            // IceCube ATWD only
            corrfactor= I3Credo::GetPhotonicsCorrectionFactor(I3CredoDOMCache::ICECUBEDOM,
                                                          amplitude,
                                                          dist,
                                                          true);
            ENSURE( (0 <= corrfactor) && (corrfactor <= 1) , "correction factor should be in [0,1]");            
        }     
    }
}

TEST(DistSquared) {
    I3Position pos1(0,0,0);
    double norm = 1. / sqrt(3);
    for (int i = -10; i< 10; i++) {
        I3Position pos2( norm*i, - norm*i, norm*i);
        ENSURE_DISTANCE(I3Credo::DistSquared(pos1, pos2), i*i, 1e-12);
    }
}

TEST(FillOneGap_NoMinOffset) {
    I3RecoPulseSeries series;
    I3RecoPulse pls; 
    pls.SetTime(0); pls.SetWidth(1); pls.SetCharge(1); series.push_back(pls); // gap 0
    pls.SetTime(1); pls.SetWidth(1); pls.SetCharge(1); series.push_back(pls); 
    
    // get an iterator to the last element
    I3RecoPulseSeries::const_iterator lastpulse = --(series.end());
    ENSURE( *lastpulse == series.back() );
    
    // fill this pulseseries with two empty pulses up to t=3 with width.5 pulses
    double tnext = 3;
    I3Credo::FillOneGap(series, lastpulse, tnext, 0, .5);  
    ENSURE( series.size() == 4, "2 pulses added");

    pls = series.at(2);
    ENSURE_DISTANCE( pls.GetTime(), 2, 1e-12); 
    ENSURE_DISTANCE( pls.GetCharge(),0, 1e-12); 
    ENSURE_DISTANCE( pls.GetWidth(), .5, 1e-12);
    pls = series.at(3);
    ENSURE_DISTANCE( pls.GetTime(), 2.5, 1e-12); 
    ENSURE_DISTANCE( pls.GetCharge(),0, 1e-12); 
    ENSURE_DISTANCE( pls.GetWidth(), .5, 1e-12);

    pls = series.back();
    ENSURE_DISTANCE( pls.GetTime() + pls.GetWidth(), tnext, 1e-12);
}

TEST(FillOneGap_MinOffset) {
    I3RecoPulseSeries series;
    I3RecoPulse pls; 
    pls.SetTime(0); pls.SetWidth(2); pls.SetCharge(1); series.push_back(pls); 
    
    // get an iterator to the last element
    I3RecoPulseSeries::const_iterator lastpulse = --(series.end());
    
    // don't fill this gap since  2.5 - 2 <  1
    I3Credo::FillOneGap(series, lastpulse, 2.5, 1, .5);  
    ENSURE( series.size() == 1, "no pulses added");
    // should also not be filled
    I3Credo::FillOneGap(series, lastpulse, 2.5, .5, .5);  
    ENSURE( series.size() == 1, "no pulses added");
    // but this one
    I3Credo::FillOneGap(series, lastpulse, 2.5, .4, .5);  
    ENSURE( series.size() == 2, "one pulses added");
    pls = series.back();
    ENSURE_DISTANCE( pls.GetTime(), 2, 1e-12); 
    ENSURE_DISTANCE( pls.GetCharge(),0, 1e-12); 
    ENSURE_DISTANCE( pls.GetWidth(), .5, 1e-12);
}

TEST(GetPulseMapWithGaps) {
    I3RecoPulseSeries series;
    I3RecoPulseSeriesMapPtr map = I3RecoPulseSeriesMapPtr(new I3RecoPulseSeriesMap());
    I3RecoPulse pls; 
    pls.SetTime(0); pls.SetWidth(5); pls.SetCharge(1); series.push_back(pls);  
    // gap 5
    pls.SetTime(10); pls.SetWidth(5); pls.SetCharge(2); series.push_back(pls); 
    // gap 200 .. should be filled with two 100ns empty pulses
    pls.SetTime(215); pls.SetWidth(5); pls.SetCharge(3); series.push_back(pls);
    OMKey omkey(12,34);
    (*map)[omkey] = series;
    
    I3RecoPulseSeriesMapPtr newmap = I3Credo::GetPulseMapWithGaps(map,false);
    ENSURE( newmap->size() == 1);
    I3RecoPulseSeriesMap::const_iterator iter;
    iter = newmap->find(omkey);
    const I3RecoPulseSeries& newseries = iter->second;
    ENSURE( newseries.size() == 5);
    pls = newseries.at(0);
    ENSURE_DISTANCE(pls.GetTime(),   0, 1e-12);
    ENSURE_DISTANCE(pls.GetWidth(),  5, 1e-12);
    ENSURE_DISTANCE(pls.GetCharge(), 1, 1e-12);
    pls = newseries.at(1);
    ENSURE_DISTANCE(pls.GetTime(),  10, 1e-12);
    ENSURE_DISTANCE(pls.GetWidth(),  5, 1e-12);
    ENSURE_DISTANCE(pls.GetCharge(), 2, 1e-12);
    pls = newseries.at(2);
    ENSURE_DISTANCE(pls.GetTime(),   15, 1e-12);
    ENSURE_DISTANCE(pls.GetWidth(), 100, 1e-12);
    ENSURE_DISTANCE(pls.GetCharge(),  0, 1e-12);
    pls = newseries.at(3);
    ENSURE_DISTANCE(pls.GetTime(),  115, 1e-12);
    ENSURE_DISTANCE(pls.GetWidth(), 100, 1e-12);
    ENSURE_DISTANCE(pls.GetCharge(),  0, 1e-12);
    pls = newseries.at(4);
    ENSURE_DISTANCE(pls.GetTime(),  215, 1e-12);
    ENSURE_DISTANCE(pls.GetWidth(),   5, 1e-12);
    ENSURE_DISTANCE(pls.GetCharge(),  3, 1e-12);
    
    
    
} 




