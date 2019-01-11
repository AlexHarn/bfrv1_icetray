#include "ddddr/I3MuonEnergyProfile.h"

I3MuonEnergyProfile::I3MuonEnergyProfile(std::vector<DOMDATA> icdata, double binWidth, 
		const std::string errors)
{
	//We need at least two DOMs to construct a profile 
	if (icdata.size() < 2)
	{
		throw (int)icdata.size();
	}

	//The only error options are default and spread
	if (errors != "default" && errors != "spread")
	{
		throw (std::string) errors;
	}
	else
	{
		errors_ = errors;
	}

	binWidth_ = binWidth;

	sumWeightsy_ = 0;

	std::vector<double> slant;
	std::vector<double> dEdX;	

	for (std::vector<DOMDATA>::const_iterator it = icdata.begin();
			it != icdata.end(); ++it)
	{
		slant.push_back(it->slant);
		dEdX.push_back(it->dEdX);
	}
	xmin_ = *std::min_element(slant.begin(), slant.end());
	xmax_ = *std::max_element(slant.begin(), slant.end());
	
	nBins_ = int((xmax_ - xmin_)/binWidth_) + 1;
	nBinEntries_ = 0;

	//set xmax_ to higher edge of last bin
	xmax_ = xmin_ + nBins_ * binWidth_;

	createEmptyBins(nBins_);

	for (std::vector<double>::size_type i = 0; i != slant.size(); i++)
	{
		Fill(slant[i], dEdX[i]);
	}

}

I3MuonEnergyProfile::I3MuonEnergyProfile(std::vector<double> slant, std::vector<double> dEdX, 
		double binWidth, const std::string errors)
{
	//We need at least two DOMs to construct a profile 
	if (slant.size() < 2)
	{
		throw (int)slant.size();
	}

	//The only error options are default and spread
	if (errors != "default" && errors != "spread")
	{
		throw (std::string) errors;
	}
	else
	{
		errors_ = errors;
	}

	binWidth_ = binWidth;

	sumWeightsy_ = 0;

	xmin_ = *std::min_element(slant.begin(), slant.end());
	xmax_ = *std::max_element(slant.begin(), slant.end());
	
	nBins_ = int((xmax_ - xmin_)/binWidth_) + 1;
	nBinEntries_ = 0;
	nEnergyLosses_ = 0;

	//set xmax_ to higher edge of last bin
	xmax_ = xmin_ + nBins_ * binWidth_;

	createEmptyBins(nBins_);

	for (std::vector<double>::size_type i = 0; i != slant.size(); i++)
	{
		Fill(slant[i], dEdX[i]);
	}

}

I3MuonEnergyProfile::I3MuonEnergyProfile(double slant_min, double slant_max, double binWidth,
		std::string const errors)
{
	if (errors != "default" && errors != "spread")
	{
		throw (std::string) errors;
	}
	else
	{
		errors_ = errors;
	}

	binWidth_ = binWidth;

	sumWeightsy_ = 0;

	xmin_ = slant_min;
	xmax_ = slant_max;
	
	// slant_max is the max slant that can still be filled in the histogram
	nBins_ = int((xmax_ - xmin_)/binWidth_) + 1;
	nBinEntries_ = 0;

	//set xmax_ to higher edge of last bin
	xmax_ = xmin_ + nBins_ * binWidth_;

	createEmptyBins(nBins_);
}

I3MuonEnergyProfile::I3MuonEnergyProfile(std::vector<double> binEdges, double binWidth,
		std::string const errors)
{
	if (errors != "default" && errors != "spread")
	{
		throw (std::string) errors;
	}
	else
	{
		errors_ = errors;
	}

	binWidth_ = binWidth;
	sumWeightsy_ = 0;
	nBins_ = binEdges.size() - 1;
	nBinEntries_ = 0;

	xmin_ = binEdges[0];
	xmax_ = binEdges.back();

	createEmptyBins(binEdges);
}

I3MuonEnergyProfile::~I3MuonEnergyProfile() {}

void I3MuonEnergyProfile::createEmptyBins(int n)
{
	for (int i = 0; i <= n; i++)
	{
		binEdges_.push_back(xmin_ + i*binWidth_);
	}

	for (int i = 0; i < n; i++)
	{
		binContent_.push_back(0.);
		binContent2_.push_back(0.);
		binEntries_.push_back(0);
	}
}

void I3MuonEnergyProfile::createEmptyBins(std::vector<double> binEdges)
{
	int n = binEdges.size() - 1;
	binEdges_ = binEdges;

	for (int i = 0; i < n; i++)
	{
		binContent_.push_back(0.);
		binContent2_.push_back(0.);
		binEntries_.push_back(0);
	}
}

int I3MuonEnergyProfile::Fill(double x, double y)
{
	if (x >= xmin_ and x < xmax_)
	{
		int bin = FindBin(x);
		AddBinContent(bin, y);

		sumWeightsy_  += y;

		return bin;
	}
	else
		return -1;
}

double I3MuonEnergyProfile::GetBinCenter(int bin)
{
	return binEdges_[bin] + binWidth_/2.;
}

std::vector<double> I3MuonEnergyProfile::GetBinCenters()
{
	std::vector<double> binCenters;
	for (std::vector<double>::size_type i = 0; i < binEdges_.size() - 1; i++)
	{
		binCenters.push_back(binEdges_[i] + binWidth_/2.);
	}
	return binCenters;
}

int I3MuonEnergyProfile::GetNBins()
{
	return nBins_;
}

int I3MuonEnergyProfile::GetNNonZeroBins()
{
	int nonZeroBins_ = 0;
	for (int i = 0; i < (int)binContent_.size(); i++)
	{
		if (GetBinContent(i) > 0.) 
		{
			nonZeroBins_++;
		}
	}
	return nonZeroBins_;
}

int I3MuonEnergyProfile::GetNEntries()
{
	return nBinEntries_;
}

int I3MuonEnergyProfile::GetNEnergyLosses()
{
	return nEnergyLosses_;
}

double I3MuonEnergyProfile::GetBinContent(int bin)
{
	if (binEntries_[bin] == 0)
	{
		return 0.;
	}
	else
	{
		return binContent_[bin]/binEntries_[bin];
	}
}

std::vector<double> I3MuonEnergyProfile::GetBinContents()
{
	std::vector<double> binContents;
	for (int i = 0; i < (int)binContent_.size(); i++)
	{
		binContents.push_back(GetBinContent(i));
	}
	return binContents;
}

/* The error calculation is the same as in
 * http://root.cern.ch/root/html/TProfile.html. A more detailed 
 * explanation can be found in the ROOT Histogram guide at
 * http://root.cern.ch/root/doc/RootDoc.html
 */
double I3MuonEnergyProfile::GetBinError(int bin)
{

	if (binEntries_[bin] == 0)
	{
		return 0.;
	}

	double spread =  sqrt(binContent2_[bin]/binEntries_[bin] 
			- pow(binContent_[bin]/binEntries_[bin],2));
	
	if (spread == 0.0)
	{
		/* If spread is zero and there's only one entry in the bin, 
		 * return the average error of all bins (with more then one entry, 
		 * to avoid recursion). Otherwise return 
		 * sqrt(binContent)/sqrt(binEntries) (default errors)
		 * sqrt(binContent) (otherwise)
		 */
		if (binEntries_[bin] == 1)
		{
			int bins_nzero = 0;
			double ssum = 0;
			for (int i = 0; i < nBins_; i++)
			{
				if (binEntries_[i] > 1)
				{
					if (errors_ == "spread")
					{
						ssum +=  sqrt(binContent2_[i]/binEntries_[i] 
								- pow(binContent_[i]/binEntries_[i],2));
					}
					else
					{
						ssum +=  sqrt(binContent2_[i]/binEntries_[i] 
								- pow(binContent_[i]/binEntries_[i],2))
								/ sqrt(binEntries_[i]);
					}
					bins_nzero++;
				}
			}
			return ssum/bins_nzero;
		}
		else
		{
			if (errors_ == "spread") 
			{
				return sqrt(binContent_[bin]);
			}
			else 
			{
				return sqrt(binContent_[bin])/sqrt(binEntries_[bin]);
			}
		}
	} // spread is zero
	else 
	{
			if (errors_ == "spread") 
			{
				return spread;
			}
			else 
			{
				return spread/sqrt(binEntries_[bin]);
			}
	}
}

/* The number of entries is nbins+1, entry i is the 
 * left edge of bin i.
 */
std::vector<double> I3MuonEnergyProfile::GetBinEdges()
{
	return binEdges_;
}

std::vector<int> I3MuonEnergyProfile::GetBinEntries()
{
	return binEntries_;
}

std::vector<double> I3MuonEnergyProfile::GetBins()
{
	return binEdges_;
}

double I3MuonEnergyProfile::GetBinWidth()
{
	return binWidth_;
}

int I3MuonEnergyProfile::GetMaxBin()
{
	std::vector<double> bin_contents = GetBinContents();
	std::vector<double>::iterator maxit = 
		std::max_element(bin_contents.begin(), bin_contents.end());
	return (int) std::distance(bin_contents.begin(), maxit);
	//return (int) (maxit - binContent_.begin());
}

/**
 * The mean is calculated using the sum and number of all entries, not as average
 * of the bins.
 *
 * @return mean energy loss
 */
double I3MuonEnergyProfile::GetMeanEnergyLoss()
{
	return sumWeightsy_ / GetNEntries();
}

/**
 * The median of the energy content of all non-empty bins.
 *
 * @return median energy loss
 */
double I3MuonEnergyProfile::GetMedianEnergyLoss()
{
	std::vector<double> non_zero_bins;
	for (std::vector<double>::size_type i = 0; i != binContent_.size(); i++)
	{
		if (binContent_[i] > 0.00000001) //sloppy compensation of floating point errors
		{
			non_zero_bins.push_back(GetBinContent(i));
		}
	}

	if (non_zero_bins.size() > 1)
	{
		return medianOfVector(non_zero_bins);
	}
	else
	{
		return 0;
	}
}

double I3MuonEnergyProfile::medianOfVector(std::vector<double> vec)
{
	if (vec.size() % 2 != 0)
	{
		size_t n = vec.size() / 2;
		std::nth_element(vec.begin(), vec.begin()+n, vec.end());
		return vec[n];
	}
	else
	{
		size_t n = vec.size() / 2;
		std::nth_element(vec.begin(), vec.begin()+n, vec.end());
		std::nth_element(vec.begin(), vec.begin()+(n-1), vec.end());
		return (vec[n]+vec[n-1])/2;
	}
}

double I3MuonEnergyProfile::GetXMin()
{
	return xmin_;
}

double I3MuonEnergyProfile::GetXMax()
{
	return xmax_;
}

int I3MuonEnergyProfile::FindBin(double x)
{
	if (x < xmin_ or x >= xmax_)
	{
		throw std::out_of_range("X is out of range!");
	}
	return int((x-xmin_)/binWidth_);
}

void I3MuonEnergyProfile::AddBinContent(int bin, double y)
{
	binContent_[bin] += y;
	binContent2_[bin] += y*y;
	binEntries_[bin]++;
	nBinEntries_++;

	if (y > 0.)
		nEnergyLosses_++;
}
