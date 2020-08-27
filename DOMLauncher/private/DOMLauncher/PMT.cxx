class PMT
{
public:
	virtual double GetPrePulseProbability() = 0;
	virtual double GetLatePulseProbability() = 0;
	virtual double PMTJitter(boost::shared_ptr<I3RandomService> randomService) = 0;
	virtual boost::shared_ptr<I3SumGenerator> GetSpeDistribution(const I3DOMCalibration& calibration, 
			I3RandomServicePtr r) = 0;
	virtual bool SkipSaturation() = 0;
	virtual bool SkipMergeHits() = 0;
};

class HamamatsuR7081_02PMT : public PMT // IceCube IceTop PDOM
{
private:
	double prePulseProbability_, latePulseProbability_;
	static boost::shared_ptr<I3SumGenerator> genericChargeDistribution_;
	
public:
	HamamatsuR7081_02PMT(double prePulseProbability, double latePulseProbability, I3RandomServicePtr randomService): 
			prePulseProbability_(prePulseProbability), latePulseProbability_(latePulseProbability)
	{
		
	}
	double GetPrePulseProbability()
	{
		return prePulseProbability_;
	}
	double GetLatePulseProbability()
	{
		return latePulseProbability_;
	}

	bool SkipSaturation()
	{
		return false;
	}

	bool SkipMergeHits()
	{
		return false;
	}
		
	//Taken indirectly from the I3HitMaker code
	/**
	 * Fisher-Tippett variables for well behaved time
	 * distribtutions.
	 * These values were obtained by fits to Bricktop running
	 * at 1345 [V] during DFL studies by C. Wendt. The fits were
	 * performed by R. Porrata.
	 */
	double PMTJitter(boost::shared_ptr<I3RandomService> randomService){
	  const double mu=0.15304; //ns
	  const double beta=1.9184; //ns
	  const double ln_time_upper_bound=0.999998;
	  const double ln_time_lower_bound=1e-7;

	  return mu - beta * log(-log(randomService->Uniform(ln_time_lower_bound,ln_time_upper_bound)));
	}	

	boost::shared_ptr<I3SumGenerator> GetSpeDistribution(const I3DOMCalibration& calibration, 
			I3RandomServicePtr randomService)
	{
		boost::shared_ptr<I3SumGenerator> speDistribution;
		auto rawDistribution=calibration.GetCombinedSPEChargeDistribution();
		if(rawDistribution.IsValid()){
			speDistribution=boost::make_shared<I3SumGenerator>(randomService,
			                  calibration.GetCombinedSPEChargeDistribution(),
							  0, //minimum value
							  3, //maximum value
							  1000, //number of bins
							  12 //when to switch to gaussian sampling
							  );
		}
		else{
			if (!genericChargeDistribution_)
			{
				// This generic charge distribution was taken as the average charge distribution from in-ice data. See https://wiki.icecube.wisc.edu/index.php/SPE_templates.
				genericChargeDistribution_ = boost::make_shared<I3SumGenerator>(randomService,
				    SPEChargeDistribution(
				        6.9, //exp1_amp
				        0.027, //exp1_width
				        0.5487936979998159, //exp2_amp
				        0.38303036483159947, //exp2_width
				        0.753056240984677, //gaus_amp
				        1.0037645548431489, //gaus_mean
				        0.3199834176880081, //gaus_width
				        1.25, // compensation factor
				        1. // SLC mean
				    ),
				    0, //minimum value
				    3, //maximum value
				    1000, //number of bins
				    12 //when to switch to gaussian sampling
					);
			}
			//log_warn_stream("Falling back to generic charge distribution for " << pmtKey);
			speDistribution=genericChargeDistribution_;
		}
		return speDistribution;
	}
};

boost::shared_ptr<I3SumGenerator> HamamatsuR7081_02PMT::genericChargeDistribution_ = nullptr;

class HamamatsuR5912_100PMT : public PMT // DEgg
{
private:
	
	static boost::shared_ptr<I3SumGenerator> genericChargeDistribution_;
	
public:
	HamamatsuR5912_100PMT(I3RandomServicePtr randomService)
	{
		
	}

	double GetPrePulseProbability()
	{
		return 0;
	}
	double GetLatePulseProbability()
	{
		return 0;
	}

	bool SkipSaturation()
	{
		return true;
	}

	bool SkipMergeHits()
	{
		return true;
	}

	double PMTJitter(boost::shared_ptr<I3RandomService> randomService)
	{
		const double mu=0.15304; //ns
		const double beta=1.9184; //ns
		const double ln_time_upper_bound=0.999998;
		const double ln_time_lower_bound=1e-7;

		return mu - beta * log(-log(randomService->Uniform(ln_time_lower_bound,ln_time_upper_bound)));
	}	

	boost::shared_ptr<I3SumGenerator> GetSpeDistribution(const I3DOMCalibration& calibration, 
			I3RandomServicePtr randomService)
	{
		boost::shared_ptr<I3SumGenerator> speDistribution;
		auto rawDistribution=calibration.GetCombinedSPEChargeDistribution();
		if(rawDistribution.IsValid()){
			speDistribution=boost::make_shared<I3SumGenerator>(randomService,
			                  calibration.GetCombinedSPEChargeDistribution(),
							  0, //minimum value
							  3, //maximum value
							  1000, //number of bins
							  12 //when to switch to gaussian sampling
							  );
		}
		else{
			if (!genericChargeDistribution_)
			{
				// This generic charge distribution was taken as the average charge distribution from in-ice data. See https://wiki.icecube.wisc.edu/index.php/SPE_templates.
				genericChargeDistribution_ = boost::make_shared<I3SumGenerator>(randomService,
				    SPEChargeDistribution(
				        6.9, //exp1_amp
				        0.027, //exp1_width
				        0.5487936979998159, //exp2_amp
				        0.38303036483159947, //exp2_width
				        0.753056240984677, //gaus_amp
				        1.0037645548431489, //gaus_mean
				        0.3199834176880081, //gaus_width
				        1.25, // compensation factor
				        1. // SLC mean
				    ),
				    0, //minimum value
				    3, //maximum value
				    1000, //number of bins
				    12 //when to switch to gaussian sampling
					);
			}
			//log_warn_stream("Falling back to generic charge distribution for " << pmtKey);
			speDistribution=genericChargeDistribution_;
		}
		return speDistribution;
	}

};

boost::shared_ptr<I3SumGenerator> HamamatsuR5912_100PMT::genericChargeDistribution_ = nullptr;

class HamamatsuR15458_02PMT : public PMT // mDOM
{
private:
	struct ChargeDistribution{
	private:
	    ///The width parameter for the exponential component, with units of the ideal s.p.e. result charge
	    double exp_width;
	    ///The location of the peak of the gaussian component, with units of the ideal s.p.e. result charge
	    double gaus_mean;
	    ///The width parameter for the gaussian component, with units of the ideal s.p.e. result charge
	    double gaus_width;
	    ///The amplitude of the gaussian component divided by the amplitude of the exponential component
	    double amp_ratio;
	    ///Overall multiplicative factor to ensure that the function has integral 1
	    double normalization;
	public:
	    ChargeDistribution(double exp_width_,
						double gaus_mean_,
						double gaus_width_,
						double amp_ratio_):
		exp_width(exp_width_),
		gaus_mean(gaus_mean_),
		gaus_width(gaus_width_),
		amp_ratio(amp_ratio_),
		normalization(1./(exp_width_ + amp_ratio_*boost::math::constants::root_two_pi<double>()*gaus_width_))
		{
		        assert(exp_width>0.0); assert(gaus_mean>0.0);
		        assert(gaus_width>0.0); assert(amp_ratio>0.0);
		}
	    ///Evaulates normalization * (exp(-q/exp_width) + amp_ratio * exp(-.5((q-gaus_mean)/gaus_width)^2))
	    double operator()(double q) const{
			double e=(q-gaus_mean)/gaus_width;
			return(normalization*(exp(-q/exp_width) + amp_ratio*exp(-0.5*e*e)));
		}	
	};
	
public:
	double GetPrePulseProbability()
	{
		return 0;
	}
	double GetLatePulseProbability()
	{
		return 0;
	}	
	double PMTJitter(boost::shared_ptr<I3RandomService> randomService)
	{
		return 0;
	}

	bool SkipSaturation()
	{
		return true;
	}

	bool SkipMergeHits()
	{
		return true;
	}

	boost::shared_ptr<I3SumGenerator> GetSpeDistribution(const I3DOMCalibration& calibration, 
			I3RandomServicePtr r)
	{
		return boost::make_shared<I3SumGenerator>(r,
	            ChargeDistribution(0.5057, //exponential width
	                                  1., //gaussian mean
	                                  0.2916, //gaussian width
	                                  1.624367847034316), //amplitude ratio
	            0, //minimum value
	            3, //maximum value
	            1000, //number of bins
	            12 //when to switch to gaussian sampling
	            );
	}
};