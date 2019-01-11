#ifndef LLHRECO_H_INCLUDED
#define LLHRECO_H_INCLUDED
#ifndef __CINT__

namespace I3FRreco{
  void reconstruct(multe&, bool);
  void pp_start(int);
  void pp_stop();
  double pdens(const mydom&, double);
  void reco_e(multe&, preco&, double);
  void pp_setpars(double, double, bool);
  void inimctrack(multe&, preco&);
}

#endif

#endif  // LLHRECO_H_INCLUDED
