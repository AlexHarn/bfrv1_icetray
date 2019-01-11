//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TMinuit                                                              //
//                                                                      //
// The MINUIT minimisation package (base class)                         //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#ifndef ROOT_TMinuit
#define ROOT_TMinuit

#include <stdint.h>
#include <string>

namespace lilliput { class TMinuit {

private:
   TMinuit(const TMinuit &m);
   TMinuit& operator=(const TMinuit &m); // Not implemented

// should become private....
public:
        enum{kMAXWARN=100};

        int32_t        fNpfix;            //Number of fixed parameters
        int32_t        fEmpty;            //Initialization flag (1 = Minuit initialized)
        int32_t        fMaxpar;           //Maximum number of parameters
        int32_t        fMaxint;           //Maximum number of internal parameters
        int32_t        fNpar;             //Number of free parameters (total number of pars = fNpar + fNfix)
        int32_t        fMaxext;           //Maximum number of external parameters
        int32_t        fMaxIterations;    //Maximum number of iterations
        int32_t        fMaxpar5;          // fMaxpar*(fMaxpar+1)/2
        int32_t        fMaxcpt;
        int32_t        fMaxpar2;          // fMaxpar*fMaxpar
        int32_t        fMaxpar1;          // fMaxpar*(fMaxpar+1)

        double     fAmin;             //Minimum value found for FCN
        double     fUp;               //FCN+-UP defines errors (for chisquare fits UP=1)
        double     fEDM;              //Estimated vertical distance to the minimum
        double     fFval3;            //
        double     fEpsi;             //
        double     fApsi;             //
        double     fDcovar;           //Relative change in covariance matrix
        double     fEpsmac;           //machine precision for floating points:
        double     fEpsma2;           //sqrt(fEpsmac)
        double     fVlimlo;           //
        double     fVlimhi;           //
        double     fUndefi;           //Undefined number = -54321
        double     fBigedm;           //Big EDM = 123456
        double     fUpdflt;           //
        double     fXmidcr;           //
        double     fYmidcr;           //
        double     fXdircr;           //
        double     fYdircr;           //

        double     *fU;               //[fMaxpar2] External (visible to user in FCN) value of parameters
        double     *fAlim;            //[fMaxpar2] Lower limits for parameters. If zero no limits
        double     *fBlim;            //[fMaxpar2] Upper limits for parameters
        double     *fErp;             //[fMaxpar] Positive Minos errors if calculated
        double     *fErn;             //[fMaxpar] Negative Minos errors if calculated
        double     *fWerr;            //[fMaxpar] External parameters error (standard deviation, defined by UP)
        double     *fGlobcc;          //[fMaxpar] Global Correlation Coefficients
        double     *fX;               //[fMaxpar] Internal parameters values
        double     *fXt;              //[fMaxpar] Internal parameters values X saved as Xt
        double     *fDirin;           //[fMaxpar] (Internal) step sizes for current step
        double     *fXs;              //[fMaxpar] Internal parameters values saved for fixed params
        double     *fXts;             //[fMaxpar] Internal parameters values X saved as Xt for fixed params
        double     *fDirins;          //[fMaxpar] (Internal) step sizes for current step for fixed params
        double     *fGrd;             //[fMaxpar] First derivatives
        double     *fG2;              //[fMaxpar]
        double     *fGstep;           //[fMaxpar] Step sizes
        double     *fGin;             //[fMaxpar2]
        double     *fDgrd;            //[fMaxpar] Uncertainties
        double     *fGrds;            //[fMaxpar]
        double     *fG2s;             //[fMaxpar]
        double     *fGsteps;          //[fMaxpar]
        double     *fVhmat;           //[fMaxpar5] (Internal) error matrix stored as Half MATrix, since it is symmetric
        double     *fVthmat;          //[fMaxpar5] VHMAT is sometimes saved in VTHMAT, especially in MNMNOT
        double     *fP;               //[fMaxpar1]
        double     *fPstar;           //[fMaxpar2]
        double     *fPstst;           //[fMaxpar]
        double     *fPbar;            //[fMaxpar]
        double     *fPrho;            //[fMaxpar] Minimum point of parabola
        double     *fWord7;           //[fMaxpar]
        double     *fXpt;             //[fMaxcpt] X array of points for contours
        double     *fYpt;             //[fMaxcpt] Y array of points for contours

        double     *fCONTgcc;         //[fMaxpar] array used in mncont
        double     *fCONTw;           //[fMaxpar] array used in mncont
        double     *fFIXPyy;          //[fMaxpar] array used in mnfixp
        double     *fGRADgf;          //[fMaxpar] array used in mngrad
        double     *fHESSyy;          //[fMaxpar] array used in mnhess
        double     *fIMPRdsav;        //[fMaxpar] array used in mnimpr
        double     *fIMPRy;           //[fMaxpar] array used in mnimpr
        double     *fMATUvline;       //[fMaxpar] array used in mnmatu
        double     *fMIGRflnu;        //[fMaxpar] array used in mnmigr
        double     *fMIGRstep;        //[fMaxpar] array used in mnmigr
        double     *fMIGRgs;          //[fMaxpar] array used in mnmigr
        double     *fMIGRvg;          //[fMaxpar] array used in mnmigr
        double     *fMIGRxxs;         //[fMaxpar] array used in mnmigr
        double     *fMNOTxdev;        //[fMaxpar] array used in mnmnot
        double     *fMNOTw;           //[fMaxpar] array used in mnmnot
        double     *fMNOTgcc;         //[fMaxpar] array used in mnmnot
        double     *fPSDFs;           //[fMaxpar] array used in mnpsdf
        double     *fSEEKxmid;        //[fMaxpar] array used in mnseek
        double     *fSEEKxbest;       //[fMaxpar] array used in mnseek
        double     *fSIMPy;           //[fMaxpar] array used in mnsimp
        double     *fVERTq;           //[fMaxpar] array used in mnvert
        double     *fVERTs;           //[fMaxpar] array used in mnvert
        double     *fVERTpp;          //[fMaxpar] array used in mnvert
        double     *fCOMDplist;       //[fMaxpar] array used in mncomd
        double     *fPARSplist;       //[fMaxpar] array used in mnpars

        int32_t        *fNvarl;           //[fMaxpar2] parameters flag (-1=undefined, 0=constant..)
        int32_t        *fNiofex;          //[fMaxpar2] Internal parameters number, or zero if not currently variable
        int32_t        *fNexofi;          //[fMaxpar] External parameters number for currently variable parameters
        int32_t        *fIpfix;           //[fMaxpar] List of fixed parameters
        int32_t        fNu;               //
        int32_t        fIsysrd;           //standardInput unit
        int32_t        fIsyswr;           //standard output unit
        int32_t        fIsyssa;           //
        int32_t        fNpagwd;           //Page width
        int32_t        fNpagln;           //Number of lines per page
        int32_t        fNewpag;           //
        int32_t        fIstkrd[10];       //
        int32_t        fNstkrd;           //
        int32_t        fIstkwr[10];       //
        int32_t        fNstkwr;           //
        int32_t        fISW[7];           //Array of switches
        int32_t        fIdbg[11];         //Array of internal debug switches
        int32_t        fNblock;           //Number of Minuit data blocks
        int32_t        fIcomnd;           //Number of commands
        int32_t        fNfcn;             //Number of calls to FCN
        int32_t        fNfcnmx;           //Maximum number of calls to FCN
        int32_t        fNfcnlc;           //
        int32_t        fNfcnfr;           //
        int32_t        fItaur;            //
        int32_t        fIstrat;           //
        int32_t        fNwrmes[2];        //
        int32_t        fNfcwar[20];       //
        int32_t        fIcirc[2];         //
        int32_t        fStatus;           //Status flag for the last called Minuit function
        int32_t        fKe1cr;            //
        int32_t        fKe2cr;            //
        bool       fLwarn;            //true if warning messges are to be put out (default=true)
        bool       fLrepor;           //true if exceptional conditions are put out (default=false)
        bool       fLimset;           //true if a parameter is up against limits (for MINOS)
        bool       fLnolim;           //true if there are no limits on any parameters (not yet used)
        bool       fLnewmn;           //true if the previous process has unexpectedly improved FCN
        bool       fLphead;           //true if a heading should be put out for the next parameter definition
        char         *fChpt;            //!Character to be plotted at the X,Y contour positions
        std::string      *fCpnam;           //[fMaxpar2] Array of parameters names
        std::string      fCfrom;            //
        std::string      fCstatu;           //
        std::string      fCtitl;            //
        std::string      fCword;            //
        std::string      fCundef;           //
        std::string      fCvrsn;            //
        std::string      fCovmes[4];        //
        std::string      fOrigin[kMAXWARN]; //
        std::string      fWarmes[kMAXWARN]; //
        void         (*fFCN)(int32_t &npar, double *gin, double &f, double *u, int32_t flag); //!

// methods performed on TMinuit class
public:
   TMinuit();
   TMinuit(int32_t maxpar);
   virtual       ~TMinuit();
   virtual void   BuildArrays(int32_t maxpar=15);
   virtual int32_t  Command(const char *command);
   virtual int32_t  DefineParameter( int32_t parNo, const char *name, double initVal, double initErr, double lowerLimit, double upperLimit );
   virtual void   DeleteArrays();
   virtual int32_t  Eval(int32_t npar, double *grad, double &fval, double *par, int32_t flag);
   virtual int32_t  FixParameter( int32_t parNo );
   int32_t          GetMaxIterations() const {return fMaxIterations;}
   virtual int32_t  GetNumFixedPars() const;
   virtual int32_t  GetNumFreePars() const;
   virtual int32_t  GetNumPars() const;
   virtual int32_t  GetParameter( int32_t parNo, double &currentValue, double &currentError ) const;
   int32_t          GetStatus() const {return fStatus;}
   virtual int32_t  Migrad();
   virtual void   mnamin();
   virtual void   mnbins(double a1, double a2, int32_t naa, double &bl, double &bh, int32_t &nb, double &bwid);
   virtual void   mncalf(double *pvec, double &ycalf);
   virtual void   mncler();
   virtual void   mncomd(const char *crdbin, int32_t &icondn);
   virtual void   mncont(int32_t ke1, int32_t ke2, int32_t nptu, double *xptu, double *yptu, int32_t &ierrf);
   virtual void   mncrck(std::string crdbuf, int32_t maxcwd, std::string &comand, int32_t &lnc
                    ,  int32_t mxp, double *plist, int32_t &llist, int32_t &ierr, int32_t isyswr);
   virtual void   mncros(double &aopt, int32_t &iercr);
   virtual void   mncuve();
   virtual void   mnderi();
   virtual void   mndxdi(double pint, int32_t ipar, double &dxdi);
   virtual void   mneig(double *a, int32_t ndima, int32_t n, int32_t mits, double *work, double precis, int32_t &ifault);
   virtual void   mnemat(double *emat, int32_t ndim);
   virtual void   mnerrs(int32_t number, double &eplus, double &eminus, double &eparab, double &gcc);
   virtual void   mneval(double anext, double &fnext, int32_t &ierev);
   virtual void   mnexcm(const char *comand, double *plist, int32_t llist, int32_t &ierflg) ;
   virtual void   mnexin(double *pint);
   virtual void   mnfixp(int32_t iint, int32_t &ierr);
   virtual void   mnfree(int32_t k);
   virtual void   mngrad();
   virtual void   mnhelp(std::string comd);
   virtual void   mnhelp(const char *command="");
   virtual void   mnhess();
   virtual void   mnhes1();
   virtual void   mnimpr();
   virtual void   mninex(double *pint);
   virtual void   mninit(int32_t i1, int32_t i2, int32_t i3);
   virtual void   mnlims();
   virtual void   mnline(double *start, double fstart, double *step, double slope, double toler);
   virtual void   mnmatu(int32_t kode);
   virtual void   mnmigr();
   virtual void   mnmnos();
   virtual void   mnmnot(int32_t ilax, int32_t ilax2, double &val2pl, double &val2mi);
   virtual void   mnparm(int32_t k, std::string cnamj, double uk, double wk, double a, double b, int32_t &ierflg);
   virtual void   mnpars(std::string &crdbuf, int32_t &icondn);
   virtual void   mnpfit(double *parx2p, double *pary2p, int32_t npar2p, double *coef2p, double &sdev2p);
   virtual void   mnpint(double &pexti, int32_t i, double &pinti);
   virtual void   mnpout(int32_t iuext, std::string &chnam, double &val, double &err, double &xlolim, double &xuplim, int32_t &iuint) const;
   virtual void   mnprin(int32_t inkode, double fval);
   virtual void   mnpsdf();
   virtual void   mnrazz(double ynew, double *pnew, double *y, int32_t &jh, int32_t &jl);
   virtual void   mnrn15(double &val, int32_t &inseed);
   virtual void   mnrset(int32_t iopt);
   virtual void   mnsave();
   virtual void   mnscan();
   virtual void   mnseek();
   virtual void   mnset();
   virtual void   mnsimp();
   virtual void   mnstat(double &fmin, double &fedm, double &errdef, int32_t &npari, int32_t &nparx, int32_t &istat);
   virtual void   mntiny(double epsp1, double &epsbak);
   bool         mnunpt(std::string &cfname);
   virtual void   mnvert(double *a, int32_t l, int32_t m, int32_t n, int32_t &ifail);
   virtual void   mnwarn(const char *copt, const char *corg, const char *cmes);
   virtual void   mnwerr();
   virtual int32_t  Release( int32_t parNo );
   virtual int32_t  SetErrorDef( double up );
   virtual void   SetFCN(void (*fcn)(int32_t &, double *, double &f, double *, int32_t));
   virtual void   SetMaxIterations(int32_t maxiter=500) {fMaxIterations = maxiter;}
   virtual int32_t  SetPrintLevel( int32_t printLevel=0 );
}; };

#endif

