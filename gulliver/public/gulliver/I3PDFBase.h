#ifndef I3PDFBASE_H
#define I3PDFBASE_H

#include "dataclasses/I3Position.h"
#include "dataclasses/I3Direction.h"
#include "dataclasses/physics/I3Particle.h"

/**
 * @class I3PDFBase
 * This class is an interface to the PDFs, providing the
 * wf module with a single function to call regardless of the PDF.
 */
class I3PDFBase {

public:

    I3PDFBase(){} 

  //--------------------------------------------------------------

  /*
   * destructor. 
   */
    virtual ~I3PDFBase(){}
  //--------------------------------------------------------------


   /**
    * GetProbability is the virtual function overloaded
    * in the derived classes of I3PDFBase. Interface to the LLH
    * reconstruction is always the same,
    * implementation of function depends on the PDF.
    * Currently, Photorec and patched pandel are supported.
    * Input arguments:
    * I3Position  ompos ... position of current OM
    * I3Position  hypo_pos ... position of hypothesis particle
    * I3Direction hypo_dir ... direction of hypothesis particle
    * double      hypo_time_residual  ...
    *             time residual between direct hit time of
    *             hypothsis particle and actual hit_time
    * double      hypo_energy ... energy of hypothesis particle
    * @return expected_amplitude and time_probability of the hypothesis
    */

  virtual void GetProbability(const I3Position &ompos,
                  const I3Position &hypo_pos,
                  const I3Direction &hypo_dir,
                  double hypo_time_residual,
                  double hypo_energy,
                  double &expected_amplitude,
                  double &time_probability) = 0;
  virtual void GetProbability(const I3Position &ompos,
                  const I3Particle &track,
                  double hypo_time_residual,
                  double &expected_amplitude,
                  double &time_probability){
    this->GetProbability(ompos,
             track.GetPos(),
             track.GetDir(),
             hypo_time_residual,
             track.GetEnergy(),
             expected_amplitude,
             time_probability);
  };

  //--------------------------------------------------------------


};

I3_POINTER_TYPEDEFS(I3PDFBase);

#endif
