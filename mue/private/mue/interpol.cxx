/**
 * This code is a c++ version of the simplified java class Interpolate of mmc.
 */

#include <cmath>
#include <cstdlib>
#include "reader.h"
#include "interpol.h"

#define FILESAVE
#ifdef FILESAVE
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <cstdio>
using namespace std;
using namespace I3FRreco;
#endif

#ifndef __GLIBC__
#include <sys/stat.h>
#endif

interpol::interpol(int nx, double xmin, double xmax, int ny, double ymin, double ymax, double (*func)(double, double)){
  this->rname="";
  this->func=func;
  init(nx, xmin, xmax, ny, ymin, ymax, this);
}

interpol::interpol(int nx, double xmin, double xmax, int ny, double ymin, double ymax, intfunction *func){
  init(nx, xmin, xmax, ny, ymin, ymax, func);
}

interpol::~interpol(){
  delete x;
  delete y;
  delete c;
  delete d;
  delete iF;
  for(int j=0; j<ny; j++) delete f[j];
  delete f;
}

void interpol::init(int nx, double xmin, double xmax, int ny, double ymin, double ymax, intfunction *func){
  romberg=5;
  rational=false;
  this->nx=nx;
  this->ny=ny;
  this->xmin=xmin;
  this->xmax=xmax;
  this->ymin=ymin;
  this->ymax=ymax;
  xstep=(xmax-xmin)/nx;
  ystep=(ymax-ymin)/ny;
  x = new double[nx];
  y = new double[ny];
  c = new double[romberg];
  d = new double[romberg];
  iF = new double[ny];
  int i, j;
  double xaux, yaux;
  for(i=0, xaux=xmin+xstep/2; i<nx; i++,xaux+=xstep) x[i]=xaux;
  for(j=0, yaux=ymin+ystep/2; j<ny; j++,yaux+=ystep) y[j]=yaux;
  f = new double *[ny];
#ifdef FILESAVE
  int fd=0;
  int fread=0;
  char fname[100]="\0";
  sprintf(fname, "interpol_%s.data", func->rname);
  if(getinterpo()>1) if(fread==0){
    if((fd = open(fname, O_RDONLY)) < 0){
      if(!((fd = open(fname, O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR)) <0)){
	cerr<<"will be saved in file "<<fname<<" ... ";
	fread=2;
      }
      else cerr<<"cannot save in file "<<fname<<" ... ";
    }
    else{
      cerr<<"will be read in from file "<<fname<<" ... ";
      fread=1;
    }
  }
#endif
  for(j=0; j<ny; j++){
    f[j] = new double[nx];
    for(i=0; i<nx; i++){
      double aux;
#ifdef FILESAVE
      if(fread==1){ if(read(fd, &aux, sizeof(double)) != sizeof(double)){ cerr<<"error reading from file "<<fname<<" ... "; exit(-1); } } else
#endif
	aux=log(func->ifunction(x[i], y[j]));
#ifdef FILESAVE
      if(fread==2) write(fd, &aux, sizeof(double));
#endif
      f[j][i]=aux;
    }
  }
#ifdef FILESAVE
  if(fread>0) close(fd);
#endif
}

double interpol::interpolate(double x, int start){
  int num, i, k;
  bool dd;
  double error=0, result=0;
  double aux, aux2, dx1, dx2;

  num=starti-start;
  if(x==iX[starti]) return iY[starti];
  for(i=0;i<romberg;i++) c[i]=d[i]=iY[start+i];

  if(num==0) dd=true;
  else if(num==romberg-1) dd=false;
  else{
    k=start+num;
    aux=iX[k-1];
    aux2=iX[k+1];
    if((x-aux>aux2-x) && (aux2>aux)) dd=true;
    else dd=false;
  }

  result=iY[start+num];
  for(k=1;k<romberg;k++){
    for(i=0;i<romberg-k;i++){
      if(rational){
	aux=c[i+1]-d[i];
	dx2=iX[start+i+k]-x;
	dx1=d[i]*(iX[start+i]-x)/dx2;
	aux2=dx1-c[i+1];
	if(aux2!=0){
	  aux=aux/aux2;
	  d[i]=c[i+1]*aux;
	  c[i]=dx1*aux;
	}
	else{
	  c[i]=0;
	  d[i]=0;
	}
      }
      else{
	dx1=iX[start+i]-x;
	dx2=iX[start+i+k]-x;
	aux=c[i+1]-d[i];
	aux2=dx1-dx2;
	if(aux2!=0){
	  aux=aux/aux2;
	  c[i]=dx1*aux;
	  d[i]=dx2*aux;
	}
	else{
	  c[i]=0;
	  d[i]=0;
	}
      }
    }
    if(num==0) dd=true;
    if(num==romberg-k) dd=false;
    if(dd) error=c[num];
    else{ num--; error=d[num]; }
    dd=!dd;
    result+=error;
  }

  return result;
}

double interpol::interpolate(double x){
  int start;
  double result, aux;
  aux=(x-xmin)/xstep;
  starti=(int)aux;
  if(starti<0) starti=0;
  if(starti>=nx) starti=nx-1;
  start=(int)(aux-0.5*(romberg-1));
  if(start<0) start=0;
  if(start+romberg>ny || start>nx) start=nx-romberg;
  result=interpolate(x, start);
  return result;
}

double interpol::interpolate(double xi, double yi){
  int i, start, starti;
  double aux, result;
  aux=(yi-ymin)/ystep;
  starti=(int)aux;
  if(starti<0) starti=0;
  if(starti>=ny) starti=ny-1;
  start=(int)(aux-0.5*(romberg-1));
  if(start<0) start=0;
  if(start+romberg>ny || start>ny) start=ny-romberg;
  iX=x;
  for(i=start; i<start+romberg; i++){ iY=f[i]; iF[i]=interpolate(xi); }
  iX=y;
  iY=iF;
  this->starti=starti;
  result=interpolate(yi, start);
  return result;
}
