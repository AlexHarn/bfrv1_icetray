#include <I3Test.h>
#include <icetray/I3Logging.h>

#include "ddddr/I3MuonEnergyProfile.h"
#include "ddddr/I3MuonEnergy.h"

TEST_GROUP(basic_examples);

TEST(basic_binwidth_divisible)
{
	/* The number of bins is defined by the X values of the data points 
	 * and the bin width. A bin contains all values 
	 * bin_lower_edge <= X < bin_higher_edge. If the total width (X_max - X_min) 
	 * is divisble by the bin width, we need an extra bin containing X_max 
	 * as lower edge.  In the case below, X = 100 should be in the bin 
	 * X = [100, 150).
	 * We should get the bins [[0,50), [50, 100), [100, 150)] and X_max is the 
	 * higher edge of the last bin.
	 */

	DOMDATA dom1;
	dom1.charge = 0;
	dom1.time = 0;
	dom1.depth = 0;
	dom1.lambda = 0;
	dom1.impact = 0;
	dom1.slant = 0;
	dom1.dEdX = 100;

	DOMDATA dom2;
	dom2.charge = 0;
	dom2.time = 0;
	dom2.depth = 0;
	dom2.lambda = 0;
	dom2.impact = 0;
	dom2.slant = 100;
	dom2.dEdX = 200;

	double binw = 50;

	std::vector<DOMDATA> icdata;
	icdata.push_back(dom1);
	icdata.push_back(dom2);

	I3MuonEnergyProfile eprofile(icdata, binw);

	int sup_nBins = 3;
	double sup_xmin = 0;
	double sup_xmax = 150;

	ENSURE_EQUAL(eprofile.GetNBins(), sup_nBins, "Number of bins is not correct");
	ENSURE_DISTANCE(eprofile.GetXMin(), sup_xmin, 0.0000001, "X_min not correct");
	ENSURE_DISTANCE(eprofile.GetXMax(), sup_xmax, 0.0000001, "X_max not correct");
}

TEST(basic_binwidth_not_divisible)
{
	/* The number of bins is defined by the X values of the data points 
	 * and the bin width. A bin contains all values 
	 * bin_lower_edge <= X < bin_higher_edge. If the total width (X_max - X_min) 
	 * is not divisble by the bin width, X_max is in the last bin.  
	 * In the case below, X = 100 should be in the bin X = [90, 120).
	 * We should get the bins [[0,30), [30, 60), [60, 90), [90,190)]
	 * and X_max is the last bin.
	 */
	DOMDATA dom1;
	dom1.charge = 0;
	dom1.time = 0;
	dom1.depth = 0;
	dom1.lambda = 0;
	dom1.impact = 0;
	dom1.slant = 0;
	dom1.dEdX = 100;

	DOMDATA dom2;
	dom2.charge = 0;
	dom2.time = 0;
	dom2.depth = 0;
	dom2.lambda = 0;
	dom2.impact = 0;
	dom2.slant = 100;
	dom2.dEdX = 200;

	double binw = 30;

	std::vector<DOMDATA> icdata;
	icdata.push_back(dom1);
	icdata.push_back(dom2);

	I3MuonEnergyProfile eprofile(icdata, binw);

	int sup_nBins = 4;
	double sup_xmin = 0;
	double sup_xmax = 120;

	ENSURE_EQUAL(eprofile.GetNBins(), sup_nBins, "Number of bins is not correct");
	ENSURE_DISTANCE(eprofile.GetXMin(), sup_xmin, 0.0000001, "X_min not correct");
	ENSURE_DISTANCE(eprofile.GetXMax(), sup_xmax, 0.0000001, "X_max not correct");
}


TEST(GetNBins)
{
	DOMDATA dom1;
	dom1.charge = 0;
	dom1.time = 0;
	dom1.depth = 0;
	dom1.lambda = 0;
	dom1.impact = 0;
	dom1.slant = 0;
	dom1.dEdX = 100;

	DOMDATA dom2;
	dom2.charge = 0;
	dom2.time = 0;
	dom2.depth = 0;
	dom2.lambda = 0;
	dom2.impact = 0;
	dom2.slant = 100;
	dom2.dEdX = 200;

	double binw = 30;

	std::vector<DOMDATA> icdata;
	icdata.push_back(dom1);
	icdata.push_back(dom2);

	I3MuonEnergyProfile eprofile_notdiv(icdata, binw);

	int sup_nBins = 4;

	ENSURE_EQUAL(eprofile_notdiv.GetNBins(), sup_nBins, "Number of bins is not correct");

	binw = 50;
	I3MuonEnergyProfile eprofile_div(icdata, binw);

	sup_nBins = 3;

	ENSURE_EQUAL(eprofile_div.GetNBins(), sup_nBins, "Number of bins is not correct");
}

TEST(GetXminXmax)
{
	DOMDATA dom1;
	dom1.charge = 0;
	dom1.time = 0;
	dom1.depth = 0;
	dom1.lambda = 0;
	dom1.impact = 0;
	dom1.slant = 0;
	dom1.dEdX = 100;

	DOMDATA dom2;
	dom2.charge = 0;
	dom2.time = 0;
	dom2.depth = 0;
	dom2.lambda = 0;
	dom2.impact = 0;
	dom2.slant = 100;
	dom2.dEdX = 200;

	double binw = 30;

	std::vector<DOMDATA> icdata;
	icdata.push_back(dom1);
	icdata.push_back(dom2);

	I3MuonEnergyProfile eprofile_notdiv(icdata, binw);

	double sup_xmin = 0;
	double sup_xmax = 120;

	ENSURE_DISTANCE(eprofile_notdiv.GetXMin(), sup_xmin, 0.0000001, "X_min not correct");
	ENSURE_DISTANCE(eprofile_notdiv.GetXMax(), sup_xmax, 0.0000001, "X_max not correct");

	binw = 50;
	I3MuonEnergyProfile eprofile_div(icdata, binw);

	sup_xmin = 0;
	sup_xmax = 150;

	ENSURE_DISTANCE(eprofile_div.GetXMin(), sup_xmin, 0.0000001, "X_min not correct");
	ENSURE_DISTANCE(eprofile_div.GetXMax(), sup_xmax, 0.0000001, "X_max not correct");
}

TEST(GetBinCenter)
{
	DOMDATA dom1;
	dom1.charge = 0;
	dom1.time = 0;
	dom1.depth = 0;
	dom1.lambda = 0;
	dom1.impact = 0;
	dom1.slant = 0;
	dom1.dEdX = 100;

	DOMDATA dom2;
	dom2.charge = 0;
	dom2.time = 0;
	dom2.depth = 0;
	dom2.lambda = 0;
	dom2.impact = 0;
	dom2.slant = 100;
	dom2.dEdX = 200;

	double binw = 30;

	std::vector<DOMDATA> icdata;
	icdata.push_back(dom1);
	icdata.push_back(dom2);

	I3MuonEnergyProfile eprofile_notdiv(icdata, binw);

	double binCenters_notdiv [4] = {15., 45., 75., 105.};
	for (int i = 0; i < int(sizeof(binCenters_notdiv)/sizeof(binCenters_notdiv[0])); i++)
	{
		ENSURE_DISTANCE(eprofile_notdiv.GetBinCenter(i), binCenters_notdiv[i], 0.0000001, "bin center not found correctly");
	}

	binw = 50;
	I3MuonEnergyProfile eprofile_div(icdata, binw);

	double binCenters_div [3] = {25., 75., 125.};

	for (int i = 0; i < int(sizeof(binCenters_div)/sizeof(binCenters_div[0])); i++)
	{
		ENSURE_DISTANCE(eprofile_div.GetBinCenter(i), binCenters_div[i], 0.0000001, "bin center not foudn correctly");
	}
}

TEST(GetBinContent)
{
	DOMDATA dom1;
	dom1.charge = 0;
	dom1.time = 0;
	dom1.depth = 0;
	dom1.lambda = 0;
	dom1.impact = 0;
	dom1.slant = 0;
	dom1.dEdX = 100;

	DOMDATA dom2;
	dom2.charge = 0;
	dom2.time = 0;
	dom2.depth = 0;
	dom2.lambda = 0;
	dom2.impact = 0;
	dom2.slant = 100;
	dom2.dEdX = 200;

	double binw = 30;

	std::vector<DOMDATA> icdata;
	icdata.push_back(dom1);
	icdata.push_back(dom2);

	I3MuonEnergyProfile eprofile_notdiv(icdata, binw);

	int binContents_notdiv [4] = {100,0,0,200};
	for (int i = 0; i < int(sizeof(binContents_notdiv)/sizeof(binContents_notdiv[0])); i++)
	{
		ENSURE_EQUAL(eprofile_notdiv.GetBinContent(i), binContents_notdiv[i], "bin content wrong");
	}

	binw = 50;
	I3MuonEnergyProfile eprofile_div(icdata, binw);

	int binContents_div [3] = {100,0,200};

	for (int i = 0; i < int(sizeof(binContents_div)/sizeof(binContents_div[0])); i++)
	{
		ENSURE_EQUAL(eprofile_div.GetBinContent(i), binContents_div[i], "bin content wrong");
	}
}

TEST(GetBins)
{
	DOMDATA dom1;
	dom1.charge = 0;
	dom1.time = 0;
	dom1.depth = 0;
	dom1.lambda = 0;
	dom1.impact = 0;
	dom1.slant = 0;
	dom1.dEdX = 100;

	DOMDATA dom2;
	dom2.charge = 0;
	dom2.time = 0;
	dom2.depth = 0;
	dom2.lambda = 0;
	dom2.impact = 0;
	dom2.slant = 100;
	dom2.dEdX = 200;

	double binw = 30;

	std::vector<DOMDATA> icdata;
	icdata.push_back(dom1);
	icdata.push_back(dom2);

	I3MuonEnergyProfile eprofile_notdiv(icdata, binw);

	double bins_notdiv [5] = {0.,30.,60.,90.,120.};
	for (int i = 0; i < int(sizeof(bins_notdiv)/sizeof(bins_notdiv[0])); i++)
	{
		ENSURE_DISTANCE(eprofile_notdiv.GetBins()[i], bins_notdiv[i], 0.0000001, "bin edge not correct");
	}

	binw = 50;
	I3MuonEnergyProfile eprofile_div(icdata, binw);

	double bins_div [4] = {0.,50.,100.,150.};

	for (int i = 0; i < int(sizeof(bins_div)/sizeof(bins_div[0])); i++)
	{
		ENSURE_DISTANCE(eprofile_div.GetBins()[i], bins_div[i], 0.0000001, "bin edge not correct");
	}
}

TEST(FindBin)
{
	DOMDATA dom1;
	dom1.charge = 0;
	dom1.time = 0;
	dom1.depth = 0;
	dom1.lambda = 0;
	dom1.impact = 0;
	dom1.slant = 0;
	dom1.dEdX = 100;

	DOMDATA dom2;
	dom2.charge = 0;
	dom2.time = 0;
	dom2.depth = 0;
	dom2.lambda = 0;
	dom2.impact = 0;
	dom2.slant = 100;
	dom2.dEdX = 200;

	double binw = 30;

	std::vector<DOMDATA> icdata;
	icdata.push_back(dom1);
	icdata.push_back(dom2);

	I3MuonEnergyProfile eprofile_notdiv(icdata, binw);

	double x1 = 10.;
	int bin1 = 0;

	double x2 = 50.;
	int bin2 = 1;

	double x3 = 90.;
	int bin3 = 3;

	ENSURE_EQUAL(eprofile_notdiv.FindBin(x1), bin1, "Found X in the wrong bin");
	ENSURE_EQUAL(eprofile_notdiv.FindBin(x2), bin2, "Found X in the wrong bin");
	ENSURE_EQUAL(eprofile_notdiv.FindBin(x3), bin3, "Found X in the wrong bin");


	binw = 50;
	I3MuonEnergyProfile eprofile_div(icdata, binw);

	bin1 = 0;
	bin2 = 1;
	bin3 = 1;

	ENSURE_EQUAL(eprofile_div.FindBin(x1), bin1, "Found X in the wrong bin");
	ENSURE_EQUAL(eprofile_div.FindBin(x2), bin2, "Found X in the wrong bin");
	ENSURE_EQUAL(eprofile_div.FindBin(x3), bin3, "Found X in the wrong bin");
}

TEST(GetBinContent2)
{
	double charge = 0, time = 0, depth = 0, lambda = 0, impact = 0;

	double dEdXdefault = 100.;
	int stepsize = 10;
	std::vector<DOMDATA> icdata;
	for (int i = 0; i < 1000; i+=stepsize) 
	{
		DOMDATA dom;
		dom.charge = charge;
		dom.time = time;
		dom.depth = depth;
		dom.lambda = lambda;
		dom.impact = impact;
		dom.slant = double(i);
		dom.dEdX = dEdXdefault;
		icdata.push_back(dom);
	}

	double binw = 30;
	I3MuonEnergyProfile eprofile_notdiv(icdata, binw);

	for (int i = 0; i < eprofile_notdiv.GetNBins(); i++)
	{
		ENSURE_DISTANCE(eprofile_notdiv.GetBinContent(i), 100., 0.000001, "bin content wrong");
	}


	binw = 50;
	I3MuonEnergyProfile eprofile_div(icdata, binw);

	for (int i = 0; i < eprofile_div.GetNBins(); i++)
	{
		ENSURE_DISTANCE(eprofile_div.GetBinContent(i), 100., 0.000001, "bin content wrong");
	}
}

TEST(GetBinContent3)
{
	double charge = 0, time = 0, depth = 0, lambda = 0, impact = 0;

	double dEdXdefault = 100.;
	std::vector<DOMDATA> icdata;
	for (int i = 0; i < 1000; i++) 
	{
		DOMDATA dom;
		dom.charge = charge;
		dom.time = time;
		dom.depth = depth;
		dom.lambda = lambda;
		dom.impact = impact;
		dom.slant = double(i);
		dom.dEdX = double(i*dEdXdefault);
		icdata.push_back(dom);
	}

	double binw = 30;
	I3MuonEnergyProfile eprofile_notdiv(icdata, binw);

	for (int i = 0; i < eprofile_notdiv.GetNBins() - 1; i++)
	{
		double sum = 0;
		for (int j = i*binw; j < (i+1)*binw; j++)
		{
			sum += j*dEdXdefault;
		}
		ENSURE_DISTANCE(eprofile_notdiv.GetBinContent(i), sum/binw, dEdXdefault/2, "bin content wrong");
	}


	binw = 50;
	I3MuonEnergyProfile eprofile_div(icdata, binw);

	for (int i = 0; i < eprofile_div.GetNBins() - 1; i++)
	{
		double sum = 0;
		for (int j = i*binw; j < (i+1)*binw; j++)
		{
			sum += j*dEdXdefault;
		}
		ENSURE_DISTANCE(eprofile_div.GetBinContent(i), sum/binw, dEdXdefault/2, "bin content wrong");
	}
}

TEST(MaxBin)
{
	double charge = 0, time = 0, depth = 0, lambda = 0, impact = 0;

	double dEdXdefault = 100.;
	std::vector<DOMDATA> icdata;
	for (int i = 0; i < 1000; i++) 
	{
		DOMDATA dom;
		dom.charge = charge;
		dom.time = time;
		dom.depth = depth;
		dom.lambda = lambda;
		dom.impact = impact;
		dom.slant = double(i);
		if (i < 499)
			dom.dEdX = double((i/50)*dEdXdefault+10);
		else
			dom.dEdX = double((1000-i)/50*dEdXdefault);

		icdata.push_back(dom);
	}

	double binw = 50;
	I3MuonEnergyProfile eprofile_notdiv(icdata, binw);

	ENSURE_EQUAL(eprofile_notdiv.GetMaxBin(), 9, "found the wrong max bin.");
}
