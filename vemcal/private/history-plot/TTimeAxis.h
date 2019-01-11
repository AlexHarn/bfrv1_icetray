


#ifndef _VEMCAL_TTIMEAXIS_H_
#define _VEMCAL_TTIMEAXIS_H_


#include <TGaxis.h>

class TTimeAxis : public TGaxis
{
 public:
   TTimeAxis();
   TTimeAxis(Double_t xmin,Double_t ymin,Double_t xmax,Double_t ymax,
	     Double_t wmin,Double_t wmax,Int_t ndiv=510, Option_t *chopt="",
	     Double_t gridlength = 0);
   

   virtual void PaintAxis(Double_t xmin,Double_t ymin,Double_t xmax,Double_t ymax,
			  Double_t &wmin,Double_t &wmax,Int_t &ndiv, Option_t *chopt="",
			  Double_t gridlength = 0, Bool_t drawGridOnly = kFALSE);
   
   // void SetTimeOffset(Double_t toffset, Option_t *option="local");
   
   
   //ClassDef(TGaxis, 0) 
};

#endif


