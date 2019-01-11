from icecube.icetray import I3Frame, I3Module
from icecube.dataclasses import I3EventHeader, I3Constants
from math import cos
from ROOT import TH1D,TCanvas, TFile, TF1, TPaveText, gROOT
class PlotStoch(I3Module):
    def __init__(self, ctx):
        I3Module.__init__(self, ctx)
        self.AddParameter("InputElosses",
                          "List of I3Vector<I3Particle>'s which contain true And/or reco eLosses",
                          [''])
        self.AddParameter("InputFitResult",
                          "Input I3Eloss object which contains fitparams and stochastics variables",
                          '')
        self.AddParameter("InputFitResultRed",
                          "Input I3Eloss object which contains fitparams of fit without stochastics",
                          '')
        self.AddParameter("OutputFileName",
                          "Name of outputfile, should have .ps extension.",
                          None)
        self.AddParameter("RootOutputFileName",
                          "Name of root output file, in case you want to store the histograms in a ROOT file.",
                          None)
        self.AddParameter("NEvents",
                          "Max number of events to plot",
                          100)

        self.AddOutBox("OutBox")
    
    def Configure(self):
        self.inputElosses = self.GetParameter("InputElosses")
        self.inputFit = self.GetParameter("InputFitResult")
        self.inputFitRed = self.GetParameter("InputFitResultRed")
        self.outputFile = self.GetParameter("OutputFileName")
        self.rootoutputFile=self.GetParameter("RootOutPutFileName")
        self.nEvents = self.GetParameter("NEvents")

        if len(self.inputElosses) == 0:
            raise RuntimeError("Please specify at least one input I3Vector<I3Particle> to be plotted")
        elif len(self.inputElosses) > 3:
            raise RuntimeError("Too many inputs specified, plot will not be very nice, so remove a couple of inputs")

        self.eventCount = 0
        # If a .ps file is specified, start the output file. If a ROOT file is specified, open it. 
        if self.outputFile:
            self.can=TCanvas("can")
            self.can.Print(self.outputFile + '[')
        if self.rootoutputFile:
            self.rootfile = TFile(self.rootoutputFile,'RECREATE')

    def Physics(self, frame):
        '''
        Structure : 1) Get vectors of Particle
                    2) For each vector : loop over them and store X and dE/dX in 1D hist
                    3) Get fitparameters and plot fit(s) (need muonBundleEnergyLoss for that)
                    4) Add EventID, Run, and other stoch calculations on plot
                    5) Save nEvents to ps file (possible event selection : do in python function before this module)
        '''

        counter = 0
        ## PRELIM

        self.eventCount += 1
        if self.eventCount > self.nEvents :
            self.PushFrame(frame)
            return
        for eLossVect_name in self.inputElosses:
            if eLossVect_name in frame:
                counter += 1
                eLossVect = frame[eLossVect_name]
                bins = len(eLossVect)
                binmin = (I3Constants.zIceTop - eLossVect[0].pos.z)/cos(eLossVect[0].dir.zenith)
                binmax = (I3Constants.zIceTop - eLossVect[-1].pos.z)/cos(eLossVect[-1].dir.zenith)
                if 'I3EventHeader' in frame:
                    evHead = frame['I3EventHeader']
                    run = evHead.run_id
                    event = evHead.event_id
                else :
                    print('VERY VERY WEIRD, no Eventheader found')
                title = 'Run %i event %i' % (run,event)
                hist = TH1D('eloss',title,bins,binmin,binmax)
                for eLoss in eLossVect:
                    slant = (I3Constants.zIceTop - eLoss.pos.z)/cos(eLoss.dir.zenith)
                    hist.Fill(slant,eLoss.energy)
                hist.GetXaxis().SetTitle('slant depth (m)')
                hist.GetYaxis().SetTitle('dE/dX (GeV/m)')
                hist.SetLineWidth(3)
                if self.outputFile:
                    self.can.SetLogy(1)
                if hist.GetEntries() == 0 :
                    continue
                if counter == 1:
                    hist.SetLineColor(2)
                    hist.SetStats(0)
                    hist.DrawCopy('')
                else :
                    if counter == 2:
                        hist.SetLineColor(3)
                    elif counter == 3:
                        hist.SetLineColor(4)
                    hist.DrawCopy('same')
                hist.Draw()
                '''
                3) Get fitparameters and plot fit(s)
                4) Add EventID, Run, and other stoch calculations on plot
                5) Save nEvents to ps file (possible event selection : do in python function before this module)
                '''
                if self.inputFit in frame and frame[self.inputFit].status==0:
                    bundleFitPar = frame[self.inputFit]
                    bundleFit = self._bundleEloss(bundleFitPar.primEnergyEstimate,bundleFitPar.primMassEstimate,binmin,binmax)
                    bundleFit.SetLineWidth(2)
                    bundleFit.DrawCopy('same')
                    hist.Fit(bundleFit,"Q")

                    # Prepare pretty plot
                    box = TPaveText(0.7,0.8,0.95,0.98,"trNDC")
                    box.SetFillColor(10)
                    box.SetLineWidth(2)
                    text = "# HE stochastics : %i" % (bundleFitPar.nHEstoch)
                    box.AddText(text)
                    text = "< Relative Stoch Energy > : %.2f" % (bundleFitPar.avRelStochEnergy)
                    box.AddText(text)
                    text = "< Stoch Energy > : %.2f GeV" % (bundleFitPar.avStochEnergy)
                    box.AddText(text)
                    text = "< Depth > : %.2f m" % (bundleFitPar.avStochDepth)
                    box.AddText(text)
                    text = "#chi^{2}/Ndof : %.6f" % (bundleFitPar.chi2)
                    box.AddText(text)
                    text = "fit status : %i" % (bundleFitPar.status)
                    box.AddText(text)
                    box.Draw('same')
        
                if self.inputFitRed in frame:
                    bundleFitPar = frame[self.inputFitRed]
                    bundleFit = self._bundleEloss(bundleFitPar.primEnergyEstimate,bundleFitPar.primMassEstimate,binmin,binmax)
                    bundleFit.SetLineWidth(2)
                    bundleFit.SetLineStyle(7)
                    bundleFit.DrawCopy('same')
                    hist.Fit(bundleFit,"Q")
                
                if self.rootoutputFile:
                    self.rootfile.cd()
                    hist.Write("eloss_%s"%event)
                hist.Delete()
                if self.outputFile:
                    self.can.Print(self.outputFile)
                    
        self.PushFrame(frame)
        return
    
    def Finish(self):
        if self.outputFile:
            self.can.Print(self.outputFile + ']')
        if self.rootoutputFile:
            self.rootfile.Close()
            
    def _bundleEloss(self,param_E0,param_A,min,max):
        fit = TF1('bundleFit','exp(-0.0003285228*x)*14.5*[1]/cos(0.1645649)*1.757*pow([0]/[1],1.757-1.)* ( -pow([0]/[1],-1.757)*(0.2388136/1.757 - 0.0003285228/(1-1.757)*[0]/[1]) + pow((0.2388136/0.0003285228 * (exp(0.0003285228*x) -1.)),-1.757)* (0.2388136/1.757 - 0.0003285228/(1-1.757)*(0.2388136/0.0003285228 * (exp(0.0003285228*x) -1.))))',min,max)
        fit.FixParameter(0,param_E0)
        fit.FixParameter(1,param_A)
        return fit
        
