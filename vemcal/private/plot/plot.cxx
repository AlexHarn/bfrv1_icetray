#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <sstream>
#include <iomanip> 


#include <TFile.h>
#include <TCanvas.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TF1.h>
#include <TProfile.h>
#include <TLegend.h>
#include <TText.h>
#include <TPaveText.h>
#include <TPaveStats.h>

#include <TStyle.h>
#include <TROOT.h>
#include <TKey.h>
#include <TSystem.h>
#include <TClass.h>

//using namespace std;



void plot_muon_spec(TH1F* muonSpec, const std::string& outFormat="png", int size=800)
{
  TCanvas* canv = new TCanvas("tmpcanv","tmpcanv",size,size);
  canv->SetLeftMargin(0.12);
  canv->SetRightMargin(0.03);
  

  // Determine x range
  double totEntries = muonSpec->Integral();
  double frac = 1.0;
  int bin = muonSpec->GetNbinsX();
  while(frac>0.98 && bin>0) 
  {
    frac -= muonSpec->GetBinContent(bin)/totEntries;
    bin--;
  }
  
  double xmax = muonSpec->GetBinLowEdge(static_cast<int>(1.1*bin) + 1);
  muonSpec->GetXaxis()->SetRangeUser(0,xmax);
  

  // Remove smoothed histogram
  std::stringstream smoothName;
  smoothName << muonSpec->GetName() << "_smoothed";
  TObject* obj = muonSpec->GetListOfFunctions()->FindObject(smoothName.str().c_str());
  if(obj) muonSpec->GetListOfFunctions()->Remove(obj);
  
  TText* vem_label = dynamic_cast<TText*>(muonSpec->GetListOfFunctions()->FindObject("vem_label"));
  if(vem_label) vem_label->SetTextFont(gStyle->GetTextFont());
  
  // Draw Spectrum
  muonSpec->UseCurrentStyle();
  muonSpec->SetFillColor(17);
  
  muonSpec->GetYaxis()->SetTitleOffset(1.85);
  muonSpec->Draw();
  



  // Create Legend
  TLegend* leg = new TLegend(0.6,0.55,0.9,0.75);
  leg->SetFillStyle(0);
  leg->SetBorderSize(0);
  leg->SetTextSize(0.035);
  
  
  TF1* func_tot = muonSpec->GetFunction("total");
  if(func_tot) leg->AddEntry(func_tot, "Total", "l");
  
  TF1* func_sig = muonSpec->GetFunction("sig");
  if(func_sig) leg->AddEntry(func_sig, "Muons", "l");
  
  TF1* func_bkg = muonSpec->GetFunction("bkg");
  if(func_bkg) leg->AddEntry(func_bkg, "Background", "l");
  
  if(func_tot || func_sig || func_sig) leg->Draw();
  
  int str, om;
  if(sscanf(muonSpec->GetName(),"muonSpec_%d_%d",&str,&om)==2)
  {
    std::ostringstream filename;
    filename.fill('0');
    filename << std::setw(2) << str;
    filename << ((om<63)?"A_":"B_");
    filename << muonSpec->GetName() << "." << outFormat;
    
    canv->SaveAs(filename.str().c_str());
  }
  
  delete canv;
}


void plot_hglg_diff(TH2F* hglgHist, const std::string& outFormat="png", int size=800)
{
  TCanvas* canv = new TCanvas("tmpcanv","tmpcanv",size,size);
  canv->SetLeftMargin(0.12);
  canv->SetRightMargin(0.03);
  canv->SetLogx(1);
  
  hglgHist->UseCurrentStyle();
  
  double xmin = hglgHist->GetXaxis()->GetXmin();
  double xmax = hglgHist->GetXaxis()->GetXmax();
  
  std::stringstream profName;
  profName << hglgHist->GetName() << "_prfx";
  TProfile* prof = static_cast<TProfile*>(hglgHist->GetListOfFunctions()->FindObject(profName.str().c_str()));
  if(prof)
  {
      int i = 1;
      while(prof->GetBinError(i)<=0 && i<prof->GetNbinsX()) i++;
      xmin = prof->GetBinLowEdge(i);
      
      prof->SetLineColor(2);
  }
  
  double ymin = -1.0;
  double ymax = 1.0;
  
  TF1* bestFit  = hglgHist->GetFunction("bestFit");
  TPaveText* pave = NULL;
  TLine* linex = NULL;
  TLine* liney = NULL;
  if(bestFit)
  {
     double deltaQ  = bestFit->GetParameter(0);
     double lgCross = bestFit->GetParameter(1);
     double lgCorr  = 1.0 - deltaQ;
     double hgCross = lgCorr*lgCross;
     
     ymin = deltaQ - 1.0;
     ymax = deltaQ + 1.0;
     
     linex = new TLine(xmin, deltaQ ,xmax, deltaQ);
     linex->SetLineStyle(7);
     linex->SetLineWidth(1);
     linex->SetLineColor(4);
     
     liney = new TLine(lgCross, ymin, lgCross, ymax);
     liney->SetLineStyle(7);
     liney->SetLineWidth(1);
     liney->SetLineColor(4);
     
     
     pave = new TPaveText(0.7, 0.22, 0.96, 0.32,"NDC");
     pave->SetFillColor(0);
     pave->SetTextSize(0.03);
     pave->SetBorderSize(1);
     pave->SetShadowColor(0);
     pave->SetTextAlign(12);
     
     char label[100];
     sprintf(label,"LG_{corr}   = %.3f", lgCorr);
     pave->AddText(label);
     sprintf(label,"HG_{cr-ov} = %.1f pe", hgCross);
     pave->AddText(label);
  }
  
  
  hglgHist->GetXaxis()->SetRangeUser(xmin, xmax);
  hglgHist->GetYaxis()->SetRangeUser(ymin, ymax);
  
  // Draw Spectrum
  //hglgHist->GetYaxis()->SetTitleOffset(1.3);
  hglgHist->Draw();
  
  if(linex)
  {
    linex->Draw();
    
    TText text;
    text.SetTextSize(0.032);
    text.SetTextColor(linex->GetLineColor());
    
    char label[20];
    sprintf(label,"%.2f", linex->GetY1());
    text.DrawText(1.15*xmin, linex->GetY1() + 0.01*(ymax-ymin), label);
    
  }
  if(liney)
  {
    liney->Draw();
    
    TText text;
    text.SetTextSize(0.032);
    text.SetTextColor(liney->GetLineColor());
    
    char label[20];
    sprintf(label,"%.1f", liney->GetX1());
    text.DrawText(1.05*liney->GetX1(), ymin + 0.03*(ymax-ymin), label);
  }
  

  if(pave)  pave->Draw();
  
  
  
  int str, om;
  if(sscanf(hglgHist->GetName(),"hglgDiff_%d_%d_%*d",&str,&om)==2)
  {
    std::ostringstream filename;
    filename.fill('0');
    filename << std::setw(2) << str;
    filename << ((om<63)?"A_":"B_");
    filename << hglgHist->GetName() << "." << outFormat;
    
    canv->SaveAs(filename.str().c_str());
  }
  
  delete canv;
}


void plot_summary_vem(TH1F* sumVEM, const std::string& outFormat="png", int size=800)
{
  TCanvas* canv = new TCanvas("tmpcanv","tmpcanv",size,size);
  canv->SetLeftMargin(0.12);
  canv->SetRightMargin(0.03);
  
  sumVEM->UseCurrentStyle();
  sumVEM->SetStats(false);
  //sumVEM->SetFillColor(sumVEM->GetLineColor());
  //sumVEM->SetFillStyle(3005);
  sumVEM->SetFillColor(17);
  

  TH1F* sumLG = dynamic_cast<TH1F*>(sumVEM->GetListOfFunctions()->FindObject("summary_vem_lg"));
  if(sumLG)
  {
    sumLG->SetFillColor(2);
    sumLG->SetLineColor(2);
    sumLG->SetFillStyle(3004);
  }
  
  TPaveText* pave = dynamic_cast<TPaveText*>(sumVEM->GetListOfFunctions()->FindObject("min_max_values"));
  if(pave)
  {
    sumVEM->GetListOfFunctions()->Remove(pave);
    pave->UseCurrentStyle();
    pave->SetTextSize(0.03);
  }

  
  // Create Legend
  TLegend* leg = new TLegend(0.7,0.73,0.9,0.88);
  leg->SetFillStyle(0);
  leg->SetBorderSize(0);
  leg->SetTextSize(0.035);
  leg->AddEntry(sumVEM, "HG DOMs", "f");
  
  
  double ymax = sumVEM->GetMaximum();
  
  TH1F* sum_lg = static_cast<TH1F*>(sumVEM->GetListOfFunctions()->FindObject("summary_vem_lg"));
  if(sum_lg)
  {
    ymax = std::max(ymax, sum_lg->GetMaximum());
    
    sum_lg->SetFillColor(sum_lg->GetLineColor());
    sum_lg->SetFillStyle(3004);
    
    leg->AddEntry(sum_lg, "LG DOMs", "f");
  }
  
  sumVEM->GetYaxis()->SetRangeUser(0, 1.1*ymax);
  sumVEM->Draw();
  leg->Draw();
  
  if(pave)
  {
     pave->ConvertNDCtoPad();
     pave->SetX1NDC(0.59);
     pave->SetX2NDC(0.95);
     pave->SetY1NDC(0.48);
     pave->SetY2NDC(0.68);
     pave->ConvertNDCtoPad();
     pave->Draw();
  }
     
  std::ostringstream filename;
  filename << "summary_vem" << "." << outFormat;
  
  canv->SaveAs(filename.str().c_str());
  
  delete canv;
}


void plot_summary_crossover(TH1F* sumCO, const std::string& outFormat="png", int size=800)
{
  TCanvas* canv = new TCanvas("tmpcanv","tmpcanv",size,size);
  canv->SetLeftMargin(0.12);
  canv->SetRightMargin(0.03);
  
  gStyle->SetOptStat("emr");
  
  sumCO->UseCurrentStyle();
  //sumCO->SetStats(true);
  sumCO->SetFillColor(17);
  
  TPaveText* pave = dynamic_cast<TPaveText*>(sumCO->GetListOfFunctions()->FindObject("min_max_values"));
  if(pave)
  {
      sumCO->GetListOfFunctions()->Remove(pave);
      pave->UseCurrentStyle();
      pave->SetTextSize(0.03);
  }
  
  sumCO->Draw();
  
  if(pave)
  {
      pave->ConvertNDCtoPad();
      pave->SetX1NDC(0.15);
      pave->SetX2NDC(0.47);
      pave->SetY1NDC(0.8);
      pave->SetY2NDC(0.89);
      pave->ConvertNDCtoPad();
      pave->Draw();
  }
  
  std::ostringstream filename;
  filename << "summary_crossover" << "." << outFormat;
  
  canv->SaveAs(filename.str().c_str());
  
  delete canv;
  gStyle->SetOptStat("e");
}


void plot_summary_rchi2(TH1F* sumRChi2, const std::string& outFormat="png", int size=800)
{
  TCanvas* canv = new TCanvas("tmpcanv","tmpcanv",size,size);
  canv->SetLeftMargin(0.12);
  canv->SetRightMargin(0.03);
  
  
  sumRChi2->UseCurrentStyle();
  sumRChi2->SetStats(false);
  sumRChi2->SetFillColor(sumRChi2->GetLineColor());
  sumRChi2->SetFillColor(17);
  

  TH1F* sumHGLG = dynamic_cast<TH1F*>(sumRChi2->GetListOfFunctions()->FindObject("summary_rchi2_hglg"));
  if(sumHGLG)
  {
    sumHGLG->SetFillColor(2);
    sumHGLG->SetLineColor(2);
    sumHGLG->SetFillStyle(3004);
  }
  
  TPaveText* pave = dynamic_cast<TPaveText*>(sumRChi2->GetListOfFunctions()->FindObject("min_max_values"));
  if(pave)
  {
      sumRChi2->GetListOfFunctions()->Remove(pave);
      pave->UseCurrentStyle();
      pave->SetTextSize(0.03);
  }
  

  // Create Legend
  TLegend* leg = new TLegend(0.7,0.73,0.9,0.88);
  leg->SetFillStyle(0);
  leg->SetBorderSize(0);
  leg->SetTextSize(0.035);
  leg->AddEntry(sumRChi2, "Muon fits", "f");
  
  
  double ymax = sumRChi2->GetMaximum();
  
  TH1F* sum_hglg = static_cast<TH1F*>(sumRChi2->GetListOfFunctions()->FindObject("summary_rchi2_hglg"));
  if(sum_hglg)
  {
    ymax = std::max(ymax, sum_hglg->GetMaximum());
    
    sum_hglg->SetFillColor(sum_hglg->GetLineColor());
    sum_hglg->SetFillStyle(3004);
    
    leg->AddEntry(sum_hglg, "HG-LG fits", "f");
  }
  
  sumRChi2->GetYaxis()->SetRangeUser(0, 1.1*ymax);
  sumRChi2->Draw();
  leg->Draw();
  
  
  if(pave)
  {
      pave->ConvertNDCtoPad();
      pave->SetX1NDC(0.6);
      pave->SetX2NDC(0.95);
      pave->SetY1NDC(0.48);
      pave->SetY2NDC(0.68);
      pave->ConvertNDCtoPad();
      pave->Draw();
  }
  
  
  std::ostringstream filename;
  filename << "summary_rchi2" << "." << outFormat;
  
  canv->SaveAs(filename.str().c_str());
  
  delete canv;
}


void plot(const std::string& filename, const std::string& outFormat="png", const std::string& outDir=".")
{
    gROOT->SetBatch(true);
    
    /*
    // Choose a nice style
    gROOT->SetStyle("Plain");
    gStyle->SetFillColor(0);
    gStyle->SetPalette(1,0);
    gStyle->SetOptStat("e");
    gStyle->SetStatX(0.96);
    gStyle->SetStatY(0.89);
    
    // Set Font (ARIAL normal)
    gStyle->SetTextFont(42);
    gStyle->SetTitleFont(42);
    gStyle->SetTitleFont(42,"xyz");
    gStyle->SetLabelFont(42,"xyz");
    gStyle->SetStatFont(42);
    */

    
    
    // Choose a nice style
    gROOT->SetStyle("Plain");
    gStyle->SetFillColor(0);
    gStyle->SetPalette(1,0);
    gStyle->SetOptStat("e");
    gStyle->SetStatX(0.96);
    gStyle->SetStatY(0.89);
     
    // Set Font (ARIAL normal)
    gStyle->SetTextFont(42);
    gStyle->SetTextSize(0.1);
    gStyle->SetTitleFont(42);
    gStyle->SetTitleFont(42,"xyz");
    gStyle->SetTitleOffset(1.25,"x");
    gStyle->SetTitleOffset(1.6,"y");
    gStyle->SetTitleSize(0.033,"xyz");
    gStyle->SetTitleFontSize(0.035);
    gStyle->SetLabelFont(42,"xyz");
    gStyle->SetLabelSize(0.033,"xyz");
    gStyle->SetLabelOffset(0.005,"x");
    gStyle->SetLabelOffset(0.007,"y");
    gStyle->SetStatFont(42);
    gStyle->SetTitleAlign(21);
    gStyle->SetTitleBorderSize(0);
    gStyle->SetTitleX(0.5);
    gStyle->SetTitleY(0.93);
    



    TFile* rootFile = new TFile(filename.c_str(),"READ");
    
    // Set working directory to outDir
    // Create outDir, if it doesn't exist
    std::string oldDir = gSystem->WorkingDirectory();
    gSystem->mkdir(outDir.c_str(), true);
    gSystem->ChangeDirectory(outDir.c_str());
    
    TList* keyList = rootFile->GetListOfKeys();
    for(int i=0; i<keyList->GetEntries(); i++)
    {
      TObject* obj = static_cast<TKey*>(keyList->At(i))->ReadObj();
      //std::cout << obj->GetName() << std::endl;
      
      std::string objName = obj->GetName();
      if(objName.compare(0,8,"muonSpec")==0)
      {
	TH1F* muonSpec = static_cast<TH1F*>(obj);
	plot_muon_spec(muonSpec, outFormat);
	
      }
      else if(objName.compare(0,8,"hglgDiff")==0)
      {
	TH2F* hglgDiff = static_cast<TH2F*>(obj);
	plot_hglg_diff(hglgDiff, outFormat);
      }
      else if(objName.compare(0,11,"summary_vem")==0)
      {
	TH1F* summary = static_cast<TH1F*>(obj);
	plot_summary_vem(summary, outFormat);
      }
      else if(objName.compare(0,17,"summary_crossover")==0)
      {
	TH1F* summary = static_cast<TH1F*>(obj);
	plot_summary_crossover(summary, outFormat);
      }
      else if(objName.compare(0,13,"summary_rchi2")==0)
      {
	TH1F* summary = static_cast<TH1F*>(obj);
	plot_summary_rchi2(summary, outFormat);
      }
      else
      {
	std::cout << "Don't know what to do with object \"" << obj->GetName() << "\"" << std::endl;
      }
      
      //if(i==5) break;
    }
    
    // Change back to the previous directory
    gSystem->ChangeDirectory(oldDir.c_str());
}


int main(int argc, char *argv[])
{
    std::vector<std::string> file_names;
  
    // Define options and default values
    std::map<std::string, std::string> options;
    options["-format"]="png";
    options["-dir"]=".";
    
    std::string prevOpt="";
    std::map<std::string, std::string>::iterator opt_iter;
    for(int i=1; i<argc; i++)
    {
      std::string arg(argv[i]);
      
      // Check options with values
      bool isOpt=false;
      if(prevOpt.empty())
      {
	for(opt_iter=options.begin(); opt_iter!=options.end(); ++opt_iter)
	{
	  if(arg.compare(0,opt_iter->first.length(), opt_iter->first)==0)
	  {
	    arg.erase(0,opt_iter->first.size());
	    arg.erase(0,arg.find_first_not_of("= "));
	    if(arg.empty())
	    {
	      prevOpt = opt_iter->first;
	    }
	    else
	    {
	      opt_iter->second = arg;
	    }
	    isOpt=true;
	    break;
	  }
	}
      }
      else
      {
	options[prevOpt]=arg;
	prevOpt="";
	isOpt=true;
      }
      
      if(!isOpt && arg.compare(arg.size()-5,arg.size(),".root")==0) file_names.push_back(argv[i]);
    }
    
    /*
    cout << endl;
    for(opt_iter=options.begin(); opt_iter!=options.end(); ++opt_iter)
    {
      cout << opt_iter->first << " " << opt_iter->second << endl;

    }
    
    cout << endl;
    for(int i=0; i<file_names.size(); i++)
    {
	cout << file_names.at(i) << endl;
	
    }
    */
    
    plot(file_names.at(0), options["-format"], options["-dir"]);
    
    return 0;
}
