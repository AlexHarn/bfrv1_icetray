
#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <sstream>
#include <iomanip> 
#include <fstream> 

#include <icetray/OMKey.h>
#include <dataclasses/I3Time.h>
#include <I3Db/I3OmDb/I3OmDb.h>

#include <TROOT.h>
#include <TStyle.h>
#include <TFile.h>
#include <TChain.h>
#include <TList.h>
#include <TCanvas.h>
#include <TGraph.h>
#include <TH2F.h>
#include <TText.h>
#include <TLine.h>

#include <../private/history-plot/TTimeAxis.h>

using namespace std;

void GetDOMCalDatesFromDB(const std::string host, const std::vector<std::string>& hist_files, std::vector<I3Time>& domCalDates)
{
    // Connect to database
    int dbRevisionID = 0;
    
    // Create I3OmDb Object
    //I3OmDb* omDb = I3OmDb::InitInstance(0);
    I3OmDb* omDb = I3OmDb::InitInstance();
    
    std::string user  = "www";
    std::string pword = "";
    std::string dbase = "I3OmDb";
    
    // Connect to database
    log_info("Trying to connect to IceCube database ..."); 
    I3OmDb::Result_t rc = omDb->Connect(host.c_str(), dbRevisionID, user.c_str(), pword.c_str(), dbase.c_str()) ;
    if(rc != I3OmDb::OK && rc != I3OmDb::ErrorAlreadyConnected)
    {
        log_warn("Unable to connect to I3Db server \"%s\". DOMCal dates won't be displayed!", host.c_str());
	return;
    }
    else
    {
         log_info("Connected to database \"%s\" on %s", dbase.c_str(), host.c_str()); 
    }
  
    
    // Determine time range
    I3Time minTime(0,0);
    I3Time maxTime(0,0);
    bool isInitialized = false;
    for(unsigned int i=0; i<hist_files.size(); i++)
    {
        std::string fileName = hist_files.at(i);
	fileName.erase(0, fileName.find_last_of("/") + 1);
        
	int year, month, day, hour, minute, second;
	if(sscanf(fileName.c_str(),"VEM_history_%4d-%2d-%2d_%2d%2d%2d.root",
		  &year, &month, &day, &hour, &minute, &second) == 6)
	{
	    I3Time time;
	    time.SetUTCCalDate(year, month, day, hour, minute, second);
	  
	    if(isInitialized)
	    {
	        if(time<minTime) minTime = time;
		if(time>maxTime) maxTime = time;
	    }
	    else
	    {
	        minTime = time;
		maxTime = time;
		isInitialized = true;
	    }
	    
	    //cout << time.GetUTCString() << endl;
	}
    }
    
    // Look for DOMcals in the database
    
    std::string sqlCmd = "SELECT ValidityStartDate FROM CalibrationDetail WHERE TypeId=51 AND ValidityStartDate>'";
    sqlCmd += minTime.GetUTCString("%Y-%m-%d %H:%M:%S");
    sqlCmd += "' AND ValidityStartDate<'";
    sqlCmd += maxTime.GetUTCString("%Y-%m-%d %H:%M:%S");
    sqlCmd += "' ORDER by CaId";
    
    int numRows;
    if(omDb->ExecSqlQuery(sqlCmd.c_str(), &numRows)!=I3OmDb::OK)
    {
      cout << "Error" << endl;
      return;
    }
    
    char startDate[32] = {0};
    ZSqlMap_t map[] = {{"ValidityStartDate", startDate, ZSQL_CHAR, sizeof(startDate)}};
    
    while(omDb->ExecSqlGetNextRow(map, sizeof(map)/sizeof(*map))==I3OmDb::OK)
    {
        int year, month, day, hour, minute, second;
	if(sscanf(startDate,"%4d-%2d-%2d %2d:%2d:%2d",
		  &year, &month, &day, &hour, &minute, &second) == 6)
	{
	    I3Time calTime;
	    calTime.SetUTCCalDate(year, month, day, hour, minute, second);
	    domCalDates.push_back(calTime);
	}
    }
}


void GetDOMNamesFromDB(const std::string host, std::map<OMKey, std::string>& domNames)
{
    // Connect to database
    int dbRevisionID = 0;
    
    // Create I3OmDb Object
    //I3OmDb* omDb = I3OmDb::InitInstance(0);
    I3OmDb* omDb = I3OmDb::InitInstance();
    
    std::string user  = "www";
    std::string pword = "";
    std::string dbase = "I3OmDb";
    
    // Connect to database
    log_info("Trying to connect to IceCube database ..."); 
    I3OmDb::Result_t rc = omDb->Connect(host.c_str(), dbRevisionID, user.c_str(), pword.c_str(), dbase.c_str()) ;
    if(rc != I3OmDb::OK && rc != I3OmDb::ErrorAlreadyConnected)
    {
        log_warn("Unable to connect to I3Db server \"%s\". DOM names won't be displayed!", host.c_str());
	return;
    }
    else
    {
         log_info("Connected to database \"%s\" on %s", dbase.c_str(), host.c_str()); 
    }
    
    
    std::string sqlCmd = "SELECT DomLocation.StringId, DomLocation.TubeId, DomName.NickName "
                         "FROM DomLocation, DomName WHERE DomLocation.Serial=DomName.Serial "
                         "AND StringId>0 AND TubeId>60";
    int numRows;
    if(omDb->ExecSqlQuery(sqlCmd.c_str(), &numRows)!=I3OmDb::OK)
    {
      cout << "Error" << endl;
      return;
    }
    
    int str, om;
    char nickName[256] = {0};
    ZSqlMap_t map[] = 
    {
      {0, &str, ZSQL_INTEGER, 0},
      {0, &om, ZSQL_INTEGER, 0},
      {0, nickName, ZSQL_CHAR, sizeof(nickName)}
    };
    
    domNames.clear();
    while(omDb->ExecSqlGetNextRow(map, sizeof(map)/sizeof(*map))==I3OmDb::OK)
    {
        OMKey omKey(str, om);
	domNames[omKey] = nickName;
    }
}


void DrawCalLines(const I3Time& time_offset, std::vector<I3Time>& domCalDates, double ymin, double ymax, bool drawLabel=false)
{
    TLine line;
    line.SetLineColor(3);
    line.SetLineWidth(2);
    
    for(unsigned int i=0; i<domCalDates.size(); i++)
    {
        double x = static_cast<double>(domCalDates.at(i).GetUnixTime() - time_offset.GetUnixTime());
	line.DrawLine(x, ymin, x, ymax);
    }
    
    if(!domCalDates.empty() && drawLabel)
    {
        TText text;
	text.SetNDC(true);
	text.SetTextColor(3);
	text.DrawText(1 - gPad->GetRightMargin() - 0.08, gPad->GetBottomMargin() + 0.06, "DOMCal");
    }
}


void DivideCanvas(TCanvas* canv, const int& npads)
{
    canv->Divide(1, npads,0,0);
    
    for(int i=1; i<=npads; i++)
    {
        canv->cd(i);
	
	if(i==1)
	{
	    gPad->SetTopMargin(0.1);
	    gPad->SetBottomMargin(0);
	}
	else if(i==npads)
	{
	    gPad->SetTopMargin(0);
	    gPad->SetBottomMargin(0.25);
	}
	else
	{
	    gPad->SetTopMargin(0);
	    gPad->SetBottomMargin(0);
	}
	
	gPad->SetBorderSize(0);
	gPad->SetGridx(1);
	gPad->SetGridy(1);
    }
}


double RoundValue(double value, double base=5.0)
{
  return (value/std::fabs(value))*(std::ceil(std::fabs(value)/base))*base;
}


TH2F* DrawAxis(const I3Time& timeOffset, int tmin, int tmax, double ymin, double ymax, bool drawXaxis=false)
{
    TH2F* hist = new TH2F("axis","", 100, tmin, tmax, 100, ymin, ymax); 
    hist->SetDirectory(0);
    hist->SetBit(kCanDelete);
    
    // No statistics box
    hist->SetStats(false);
    hist->Draw();
    
    // Set axis divisions
    int numWeeks = (tmax-tmin)/(7*86400);
    int ndiv = 700 + numWeeks;
    if(numWeeks > 12)
    {
      ndiv = 400 + numWeeks/4;
    }
    hist->GetXaxis()->SetNdivisions(ndiv, false);
    
    // Disable standard axis labels
    hist->GetXaxis()->SetLabelSize(0);
    
    if(drawXaxis)
    {
        /*
	  I3Time time;
	  time.SetUnixTime(timeOffset.GetUnixTime() + tmin);
	  cout << time.GetUTCString("%Y-%m-%d %H:%M:%S") << endl;
	*/
      
        // Create special time axis
        TTimeAxis *taxis = new TTimeAxis(tmin, ymin, tmax, ymin, tmin, tmax, ndiv, "+tN");
	taxis->SetTimeFormat(timeOffset.GetUTCString("#splitline{%%m/%%d}{%%Y}%%F%Y-%m-%d %H:%M:%S").c_str());
	taxis->SetLineColor(hist->GetXaxis()->GetAxisColor());
	taxis->SetLabelColor(hist->GetXaxis()->GetLabelColor());
	taxis->SetLabelFont(hist->GetXaxis()->GetLabelFont());
	taxis->SetLabelSize(0.093);
	taxis->SetLabelOffset(0.09);
	
	taxis->Draw();
    }
    
    return hist;
}


void GetRangeY(TGraph* graph, double& ymin, double& ymax)
{
    if(!graph) return;
    if(graph->GetN()<1) return;
    
    double x, y;
    for(int i=0; i<graph->GetN(); i++)
    {
        graph->GetPoint(i, x, y);
	
	if(i==0) ymin = y;
	else     ymin = std::min(ymin, y);
	
	if(i==0) ymax = y;
	else     ymax = std::max(ymax, y);
    }
}


void DrawHistory(const OMKey& omKey, TList& graphs, const I3Time& time_offset, int minTime, int maxTime,
		 std::vector<I3Time>& domCalDates, std::string domName="")
{
    // Ensure start and stop times to be on Mondays at midnight  
    I3Time startTime;
    startTime.SetUnixTime(time_offset.GetUnixTime() + static_cast<time_t>(minTime));
    int weekDay = static_cast<int>(startTime.GetUTCWeekday());
    if(weekDay==0) weekDay = 7;
    minTime -= (startTime.GetModJulianSec() + (weekDay-1)*86400);
    
    I3Time stopTime;
    stopTime.SetUnixTime(time_offset.GetUnixTime() + static_cast<time_t>(maxTime));
    weekDay = static_cast<int>(stopTime.GetUTCWeekday());
    if(weekDay==0) weekDay = 7;
    maxTime += ((86400 - stopTime.GetModJulianSec()) + (6 - (weekDay-1))*86400);
    
    // Round to full 4-Weeks intervals
    int timeRange = maxTime - minTime;
    int numMonths = timeRange/(86400*7*4);
    int rest = (numMonths+1)*(86400*7*4) - timeRange;
    maxTime += rest;
    
    /*
    I3Time startTest;
    startTest.SetUnixTime(time_offset.GetUnixTime() + static_cast<time_t>(minTime));
    cout << startTest.GetUTCString("Start: %A %Y-%m-%d %H:%M:%S") << endl;

    I3Time stopTest;
    stopTest.SetUnixTime(time_offset.GetUnixTime() + static_cast<time_t>(maxTime));
    cout << stopTest.GetUTCString("Stop: %A %Y-%m-%d %H:%M:%S") << endl;
    */
    
    char histTitle[512];
    if(domName.empty()) sprintf(histTitle, "History of DOM %02d-%02d ", omKey.GetString(), omKey.GetOM());
    else sprintf(histTitle, "History of DOM %02d-%02d  (%s)", omKey.GetString(), omKey.GetOM(), domName.c_str());
    
    TCanvas* canv = new TCanvas("tmpcanv", histTitle, 1000, 1000);
    DivideCanvas(canv, 6);
    
    TText text;
    text.SetNDC(true);
    text.SetTextColor(4);
    
    canv->cd(1);
    TGraph* graph = static_cast<TGraph*>(graphs.FindObject("vem"));
    if(graph)
    {
        double ymin = 0, ymax=0;
	GetRangeY(graph, ymin, ymax);
	TH2F* axis = DrawAxis(time_offset, minTime, maxTime, 0, RoundValue(1.1*ymax, 10.0));
	axis->SetTitleFont(42);
	axis->SetTitle(histTitle);
	axis->GetYaxis()->SetTitle("[PE]");
	axis->GetYaxis()->SetNdivisions(505, false);
	graph->SetLineColor(kOrange+9);
	graph->SetLineWidth(2);
	graph->Draw("L");
	
	// Draw first points
	TGraph* first = static_cast<TGraph*>(graphs.FindObject("vem_first"));
	if(first)
	{
	    first->SetMarkerStyle(20);
	    first->SetMarkerSize(0.7);
	    first->SetMarkerColor(kOrange+9);
	    first->Draw("P");
	}

	DrawCalLines(time_offset, domCalDates, axis->GetYaxis()->GetXmin(), axis->GetYaxis()->GetXmax());
	text.DrawText(gPad->GetLeftMargin() + 0.04, gPad->GetBottomMargin() + 0.06, "PE per VEM");
    }
    
    canv->cd(2);
    bool isHG = false;
    graph = static_cast<TGraph*>(graphs.FindObject("hglg_co"));
    if(graph)
    {   
        double ymin=0, ymax=0;
	GetRangeY(graph, ymin, ymax);
	TH2F* axis = DrawAxis(time_offset, minTime, maxTime, 0, RoundValue(1.1*ymax, 100.0));
	axis->GetYaxis()->SetTitle("[PE]");
	axis->GetYaxis()->SetNdivisions(505, false);

	graph->SetLineColor(kOrange+9);
	graph->SetLineWidth(2);
	graph->Draw("L");
	
	// Draw first points
	TGraph* first = static_cast<TGraph*>(graphs.FindObject("hglg_co_first"));
	if(first)
	{
	    first->SetMarkerStyle(20);
	    first->SetMarkerSize(0.7);
	    first->SetMarkerColor(kOrange+9);
	    first->Draw("P");
	}
	
	DrawCalLines(time_offset, domCalDates, axis->GetYaxis()->GetXmin(), axis->GetYaxis()->GetXmax());
	text.DrawText(gPad->GetLeftMargin() + 0.04, gPad->GetBottomMargin() + 0.06, "HG-LG crossover");

	isHG = true;
    }
    else
    {
        TH2F* axis = DrawAxis(time_offset, minTime, maxTime, 0, RoundValue(3000.0, 100.0));
	axis->GetYaxis()->SetTitle("[PE]");
	axis->GetYaxis()->SetNdivisions(505, false);
	
	DrawCalLines(time_offset, domCalDates, axis->GetYaxis()->GetXmin(), axis->GetYaxis()->GetXmax());
	text.DrawText(gPad->GetLeftMargin() + 0.04, gPad->GetBottomMargin() + 0.06, "HG-LG crossover");
	
	TText infotext;
	infotext.SetNDC(true);
	infotext.SetTextColor(1);
	infotext.SetTextSize(0.125);
	infotext.SetTextAlign(22);
	infotext.DrawText(0.5, 0.5, "Not available for LG DOM");
    }
    
    canv->cd(3);
    graph = static_cast<TGraph*>(graphs.FindObject("sb_ratio"));
    if(graph)
    {
        double ymin=0, ymax=0;
	GetRangeY(graph, ymin, ymax);
	TH2F* axis = DrawAxis(time_offset, minTime, maxTime, 0, RoundValue(1.1*ymax, 5.0));
	axis->GetYaxis()->SetTitle("[1]");
	axis->GetYaxis()->SetNdivisions(505, false);
	graph->SetLineColor(kOrange+9);
	graph->SetLineWidth(2);
	graph->Draw("L");
	
	TGraph* first = static_cast<TGraph*>(graphs.FindObject("sb_ratio_first"));
	if(first)
	{
	    first->SetMarkerStyle(20);
	    first->SetMarkerSize(0.7);
	    first->SetMarkerColor(kOrange+9);
	    first->Draw("P");
	}
	
	DrawCalLines(time_offset, domCalDates, axis->GetYaxis()->GetXmin(), axis->GetYaxis()->GetXmax());
	text.DrawText(gPad->GetLeftMargin() + 0.04, gPad->GetBottomMargin() + 0.06, "S/B between [0.3, 2.0] VEM");
    }

    canv->cd(4);
    if(isHG) graph = static_cast<TGraph*>(graphs.FindObject("mpe"));
    else     graph = static_cast<TGraph*>(graphs.FindObject("spe"));
    if(graph)
    {
        double ymin=0, ymax=0;
 	GetRangeY(graph, ymin, ymax);
	TH2F* axis = DrawAxis(time_offset, minTime, maxTime, 0, RoundValue(1.1*ymax, 100.0));
	axis->GetYaxis()->SetTitle("[Hz]");
	axis->GetYaxis()->SetNdivisions(505, false);
	graph->SetLineColor(kOrange+9);
	graph->SetLineWidth(2);
	graph->Draw("L");
	
	DrawCalLines(time_offset, domCalDates, axis->GetYaxis()->GetXmin(), axis->GetYaxis()->GetXmax());
	
	if(isHG) text.DrawText(gPad->GetLeftMargin() + 0.04, gPad->GetBottomMargin() + 0.06, "MPE rate");
	else     text.DrawText(gPad->GetLeftMargin() + 0.04, gPad->GetBottomMargin() + 0.06, "SPE rate");
    }

    canv->cd(5);
    graph = static_cast<TGraph*>(graphs.FindObject("hv"));
    if(graph)
    {
        double ymin=0, ymax=0;
	GetRangeY(graph, ymin, ymax);
	TH2F* axis = DrawAxis(time_offset, minTime, maxTime, 0, RoundValue(1.1*ymax, 100.0));
	axis->GetYaxis()->SetTitle("[V]");
	axis->GetYaxis()->SetNdivisions(505, false);
	graph->SetLineColor(kOrange+9);
	graph->SetLineWidth(2);
	graph->Draw("L");
	
	DrawCalLines(time_offset, domCalDates, axis->GetYaxis()->GetXmin(), axis->GetYaxis()->GetXmax());
	text.DrawText(gPad->GetLeftMargin() + 0.04, gPad->GetBottomMargin() + 0.06, "High Voltage");
    }

    canv->cd(6);
    graph = static_cast<TGraph*>(graphs.FindObject("temp"));
    if(graph)
    {
        double ymin=0, ymax=0;
	GetRangeY(graph, ymin, ymax);
	TH2F* axis = DrawAxis(time_offset, minTime, maxTime, RoundValue(1.1*ymin, 10.0), 0, true);
	axis->GetYaxis()->SetTitle("[deg C]");
	axis->GetYaxis()->SetNdivisions(505, false);
	graph->SetLineColor(kOrange+9);
	graph->SetLineWidth(2);
	graph->Draw("L");
	
	DrawCalLines(time_offset, domCalDates, axis->GetYaxis()->GetXmin(), axis->GetYaxis()->GetXmax(), true);
	text.DrawText(gPad->GetLeftMargin() + 0.04, gPad->GetBottomMargin() + 0.06, "DOM temperature");
    }
    
    char historyName[256];
    sprintf(historyName,"%02d%s_history_%02d_%02d.png", omKey.GetString(), (omKey.GetOM()<63?"A":"B"), omKey.GetString(), omKey.GetOM()); 
    
    canv->SaveAs(historyName);
    
    delete canv;
}

/*
void AddGraphPoint(TList& graphs,  std::string name, double time, double value)
{
    if(!finite(value)) return;
    
    TGraph* graph = static_cast<TGraph*>(graphs.FindObject(name.c_str()));
    if(!graph)
    {
        graph = new TGraph(1);
	graph->SetName(name.c_str());
	graphs.Add(graph);
    }
    else
    {
        graph->Set(graph->GetN() + 1);
    }
    
    graph->SetPoint(graph->GetN()-1, time, value);
}
*/

void AddGraphPoint(TList& graphs,  std::string name, double time, double value, bool isFirstRun=false)
{
    if(!finite(value)) return;
    
    TGraph* graph = static_cast<TGraph*>(graphs.FindObject(name.c_str()));
    if(!graph)
    {
        graph = new TGraph(1);
	graph->SetName(name.c_str());
	graphs.Add(graph);
    }
    else
    {
        graph->Set(graph->GetN() + 1);
    }
    
    if(isFirstRun && graph->GetN()>1)
    {
        double x, y;
	graph->GetPoint(graph->GetN()-2, x, y);  
	graph->SetPoint(graph->GetN()-1, time, y);  
	graph->Set(graph->GetN()+1);
    }

    graph->SetPoint(graph->GetN()-1, time, value);  
}


void GenerateHistory(std::vector<std::string>& histfiles, std::vector<I3Time>& domCalDates, std::map<OMKey, std::string>& domNames)
{
    TChain* chain = new TChain("tree");
    
    // Add hist files to chain
    std::sort(histfiles.begin(), histfiles.end());
    std::vector<std::string>::const_iterator hist_iter;
    for(hist_iter=histfiles.begin(); hist_iter!=histfiles.end(); ++hist_iter)
    {
      chain->Add(hist_iter->c_str());
    }
    

    std::map<OMKey, TList*> graphMap;
     
    double year, sec, str, om, is_first, vem, sb_ratio, hglg_co, mpe, spe, hv, temp;  
    chain->SetBranchAddress("StartYear", &year);
    chain->SetBranchAddress("StartSec", &sec);
    chain->SetBranchAddress("Str", &str);
    chain->SetBranchAddress("Om", &om);
    chain->SetBranchAddress("PEperVEM", &vem);
    chain->SetBranchAddress("IsFirstRun", &is_first);
    chain->SetBranchAddress("MuEmRatio", &sb_ratio);
    chain->SetBranchAddress("HGLGCrossOver", &hglg_co);
    chain->SetBranchAddress("MPERate", &mpe);
    chain->SetBranchAddress("SPERate", &spe);
    chain->SetBranchAddress("PMTVoltage", &hv);
    chain->SetBranchAddress("DOMTemperature", &temp);

    I3Time time_offset;
    time_offset.SetUTCCalDate(2009,01,01,00,00,00);
    
    int minTime = -1;
    int maxTime = -1;
    for(int i=0; i<chain->GetEntries(); i++)
    {
        chain->GetEntry(i);

	OMKey omKey(static_cast<int>(str), static_cast<int>(om));
	I3Time time(static_cast<int>(year), static_cast<int64_t>(sec*1e10));
	
	int time_diff = static_cast<int>(time.GetUnixTime() - time_offset.GetUnixTime()); 
	if(time_diff < 0)
	{
	    log_error("Invalid date/time. Must be greater that 2009-01-01 00:00:00!");
	    return;
	}
	
	TList* graphs;
	std::map<OMKey, TList*>::iterator iter = graphMap.find(omKey);
	if(iter==graphMap.end())
	{
	    graphs = new TList();
	    graphMap[omKey] = graphs;  
	}
	else
	{
	  graphs = iter->second;
	}

	AddGraphPoint(*graphs, "vem", time_diff, vem, (is_first>0));
	if(is_first>0) AddGraphPoint(*graphs, "vem_first", time_diff, vem);
	
	if(hglg_co>0)  AddGraphPoint(*graphs, "hglg_co", time_diff, hglg_co, (is_first>0));
	if(hglg_co>0 && is_first>0) AddGraphPoint(*graphs, "hglg_co_first", time_diff, hglg_co);
	
	AddGraphPoint(*graphs, "sb_ratio", time_diff, sb_ratio, (is_first>0));
	if(is_first>0) AddGraphPoint(*graphs, "sb_ratio_first", time_diff, sb_ratio);
	
	AddGraphPoint(*graphs, "spe", time_diff, spe);
	AddGraphPoint(*graphs, "mpe", time_diff, mpe);
	AddGraphPoint(*graphs, "hv", time_diff, hv);
	AddGraphPoint(*graphs, "temp", time_diff, temp);
	
	if(minTime<0) minTime = time_diff;
	else minTime = min(minTime, time_diff);
      
	if(maxTime<0) maxTime = time_diff;
	else maxTime = max(maxTime, time_diff);
    }
    
    std::cout << "Drawing graphs ...." << std::endl;
    
    // Draw Graphs
    std::map<OMKey, TList*>::iterator graph_iter;
    for(graph_iter = graphMap.begin(); graph_iter!=graphMap.end(); ++graph_iter)
    {
        // Look for DOM name
        std::string domName = "";
        std::map<OMKey, std::string>::const_iterator name_iter = domNames.find(graph_iter->first);
	if(name_iter!=domNames.end()) domName = name_iter->second;
      
        DrawHistory(graph_iter->first, *(graph_iter->second), time_offset, minTime, maxTime, domCalDates, domName);
    }
}


int main(int argc, char *argv[])
{
    std::vector<std::string> hist_files;
    for(int i=1; i<argc; i++)
    {
        std::string arg(argv[i]);
	if(arg.compare(arg.size()-5,arg.size(),".root")==0)
	{
	    hist_files.push_back(argv[i]);
	}
    }
    
    if(hist_files.empty())
    {
        std::cout << "Please provide list of VEMCal history files as input!" << std::endl;
	return 0;
    }
    
    gROOT->SetBatch(true);
    
     // Choose a nice style
    gROOT->SetStyle("Plain");
    gStyle->SetFillColor(0);
    gStyle->SetPalette(1,0);
    gStyle->SetOptStat("e");
    gStyle->SetStatX(0.96);
    gStyle->SetStatY(0.89);
    gStyle->SetPadRightMargin(0.03);
    gStyle->SetPadLeftMargin(0.07);
    gStyle->SetPadBottomMargin(0.1);
    gStyle->SetPadTopMargin(0.1);
    
    // Set Font (ARIAL normal)
    gStyle->SetTextFont(42);
    gStyle->SetTextSize(0.1);
    gStyle->SetTitleFont(42);
    gStyle->SetTitleFont(42,"xyz");
    gStyle->SetTitleOffset(0.9,"x");
    gStyle->SetTitleOffset(0.33,"y");
    gStyle->SetTitleSize(0.11,"xyz");
    gStyle->SetTitleFontSize(0.11);
    gStyle->SetLabelFont(42,"xyz");
    gStyle->SetLabelSize(0.1,"xyz");
    gStyle->SetLabelOffset(0.03,"x");
    gStyle->SetLabelOffset(0.007,"y");
    gStyle->SetStatFont(42);
    gStyle->SetTitleAlign(21);
    gStyle->SetTitleBorderSize(0);
    gStyle->SetTitleX(0.5);
    gStyle->SetTitleY(0.94);
    
    std::vector<I3Time> domCalDates;
    GetDOMCalDatesFromDB("dbs2.icecube.wisc.edu", hist_files, domCalDates);
    if(!domCalDates.empty())
    {
        cout << "Found " << domCalDates.size() << " DOMCal date(s):" << endl; 
	for(unsigned int i=0; i<domCalDates.size(); i++)
	{ 
	    cout << domCalDates.at(i).GetUTCString() << endl;
	}
    }
    
    std::map<OMKey, std::string> domNames;
    GetDOMNamesFromDB("dbs2.icecube.wisc.edu", domNames);
    
    GenerateHistory(hist_files, domCalDates, domNames);
    
    return 0;
}
