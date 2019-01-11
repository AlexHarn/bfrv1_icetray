#ifndef INTERPOL_H_INCLUDED
#define INTERPOL_H_INCLUDED
#ifndef __CINT__

struct intfunction{
  const char *rname;
  virtual double ifunction(double, double) = 0;
};

class interpol:intfunction{

 private:

  double *x, *y;
  double **f;
  double *iX, *iY, *iF;
  double *c;
  double *d;
  // int row, starti;
  int starti;
  int romberg;
  bool rational;
  int nx, ny;
  double xmin, xmax, xstep;
  double ymin, ymax, ystep;
  double (*func)(double, double);
  double ifunction(double x, double y){ return (*func)(x, y); }

 public:
  interpol(int, double, double, int, double, double, double (*)(double, double));
  interpol(int, double, double, int, double, double, intfunction *);
  virtual ~interpol();
  double interpolate(double, double);

 private:
  void init(int, double, double, int, double, double, intfunction *);
  double interpolate(double, int);
  double interpolate(double);

};

#endif

#endif  // INTERPOL_H_INCLUDED
