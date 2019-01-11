/**
    copyright  (C) 2004
    the icecube collaboration
    $Id$

    @version $Revision: 1.1 $
    @date $Date$
    @author C Wiebusch <wiebusch@physik.uni-wuppertal.de>
        and S Robbins  <robbins@physik.uni-wuppertal.de>

    @brief Implementation of example HitOm representation

    @todo Where to get the relative efficiency of the OM?
*/

#include "ipdf/I3/I3OmReceiver.h"
#include <iostream>


#include "ipdf/Utilities/IPdfException.h"
#include "dataclasses/I3Position.h"
#include "icetray/I3Units.h"

using namespace IPDF;

I3OmReceiver::I3OmReceiver(const I3OMGeo& i3om)
// : ori_(0.), sigma_(0.), sens_(std::numeric_limits<double>::max()) //sens_(i3om.GetRelativeQE())
: ori_(0.), sigma_(0.), sens_(1.0) //sens_(i3om.GetRelativeQE())
{
  const I3Position& posn( i3om.position );
  x_ = posn.GetX()/I3Units::meter;
  y_ = posn.GetY()/I3Units::meter;
  z_ = posn.GetZ()/I3Units::meter;

  this->setOrientation(i3om.orientation);
}

void I3OmReceiver::setOrientation(const I3Orientation& i3ori) {
  ori_ = i3ori.GetZ();
}


std::ostream& IPDF::operator<<(std::ostream& os, 
    const I3OmReceiver& omr) {
  return (os<<"I3OmReceiver: ("
      <<"(x,y,z) ("<<omr.x_<<", "<<omr.y_<<", "<<omr.z_<<")"
      <<",\torientation: "<<omr.ori_
      <<", sensitivity: "<<omr.sens_
      <<", jitter: "<<omr.sigma_
      <<")");
}
