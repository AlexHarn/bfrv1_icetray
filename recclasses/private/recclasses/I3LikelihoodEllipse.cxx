#include <icetray/serialization.h>
#include <recclasses/I3LikelihoodEllipse.h>

I3LikelihoodEllipse::~I3LikelihoodEllipse() { }


I3LikelihoodEllipse::I3LikelihoodEllipse(std::vector<double> center, 
					 I3Matrix inverse_cov, 
					 double nllh, 
					 std::vector<std::string> axis_names)
{
  // Copy from the I3Matrix
  uint nrows = inverse_cov.size1();
  uint ncols = inverse_cov.size2();
  std::vector<std::vector<double> > new_matrix;

  if(nrows != ncols){
    log_fatal("Trying to instantiate with an n x m matrix, but n!=m");
  }

  for(uint i=0; i<nrows; ++i){
    std::vector<double> new_row;
    for(uint j=0; j<ncols; ++j){
      new_row.push_back( inverse_cov(i, j) );
    }
    new_matrix.push_back(new_row);
  }  

  center_ = center;
  nllh_ = nllh;
  axis_names_ = axis_names;
  inverse_covariance_ = new_matrix;
}




I3LikelihoodEllipse::I3LikelihoodEllipse(const I3LikelihoodEllipse& ellipse)
{
  center_ = ellipse.GetCenter();
  nllh_ = ellipse.GetNLLH();
  axis_names_ = ellipse.GetAxisNames();
  inverse_covariance_ = ellipse.GetInverseCovariance();
}



bool I3LikelihoodEllipse::Contains(const std::vector<double>& point)
{
  // Need to do 
  // delta = point - center
  // A = inverse_covariance
  // and . = inner product
  // return (delta . A) . delta <= 1

  std::vector<double> delta;
  std::vector<double>::const_iterator point_iter = point.begin();
  std::vector<double>::const_iterator center_iter = center_.begin();
  for(; point_iter!=point.end(); ++point_iter, ++center_iter){
    delta.push_back( (*point_iter) - (*center_iter) );
  }

  std::vector<double> A = I3LikelihoodEllipse::Dot(delta, inverse_covariance_);
  double total = I3LikelihoodEllipse::Dot( A, delta );
  return total <= 1.0;
}

double I3LikelihoodEllipse::Dot(std::vector<double>& vec1, 
			       std::vector<double>& vec2)
{
  
  if(vec1.size() != vec2.size())
    log_fatal("Vector 1 has a different size than vector 2.");
  
  return std::inner_product(vec1.begin(), vec1.end(), vec2.begin(), 0.0);
}


std::vector<double> I3LikelihoodEllipse::Dot(std::vector<double>& vec,
					    std::vector<std::vector<double> >& matrix)
{  
  if(matrix.size() != vec.size())
    log_fatal("Matrix has a different size than the vector to be dotted with.");

  std::vector<double> total;
  for(uint i=0; i<matrix.size(); ++i){
    double x = 0;
    for(uint j=0; j<matrix[0].size(); ++j){
      x += vec[j] * matrix[i][j];
    }
    total.push_back(x);
  }

  return total;
}


I3LikelihoodEllipse I3LikelihoodEllipse::Profile(uint dimension)
{
  std::vector<uint> dimensions;
  dimensions.push_back(dimension);
  return Profile(dimensions);
}



I3LikelihoodEllipse I3LikelihoodEllipse::Profile(std::vector<uint> dimensions)
{
  std::vector<double> center;
  std::vector<std::vector<double> > inverse_cov;
  std::vector<std::string> axis_names;
  double nllh = nllh_;
 
  for(uint i=0; i<center_.size(); ++i){
    if(std::find(dimensions.begin(), dimensions.end(), i)!=dimensions.end()) continue;
    center.push_back(center_[i]);
    axis_names.push_back(axis_names_[i]);
    std::vector<double> new_row;
    
    for(uint j=0; j<center_.size(); ++j){
      if(std::find(dimensions.begin(), dimensions.end(), j)!=dimensions.end()) continue;
      new_row.push_back(inverse_covariance_[i][j]);
    }
    inverse_cov.push_back(new_row);
  }

  return I3LikelihoodEllipse(center, inverse_cov, nllh, axis_names);
}

  
  
 

void I3LikelihoodEllipse::ConvertToUnitVectors(const std::vector<double>& azimuth,
					       const std::vector<double>& zenith,
					       std::vector<double>& nx,
					       std::vector<double>& ny,
					       std::vector<double>& nz)
{

  if( azimuth.size() != zenith.size() )
    log_fatal("Given azimuth and zenith lists have different lengths!");
  
  std::vector<double>::const_iterator azi_iter = azimuth.begin();
  std::vector<double>::const_iterator zen_iter = zenith.begin();
  
  for(; azi_iter!=azimuth.end(); ++azi_iter, ++zen_iter){
    nx.push_back( cos(*azi_iter)*sin(*zen_iter) );
    ny.push_back( sin(*azi_iter)*sin(*zen_iter) );
    nz.push_back( cos(*zen_iter) );
  }
  return;
}


template <class Archive>
void I3LikelihoodEllipse::serialize(Archive& ar, unsigned version)
{
  ar & make_nvp("I3FrameObject", base_object<I3FrameObject>(*this));
  ar & make_nvp("center",center_);
  ar & make_nvp("inverse_covariance",inverse_covariance_);
  ar & make_nvp("nLLH", nllh_);
  ar & make_nvp("Axis_Names", axis_names_);    
}

std::ostream& I3LikelihoodEllipse::Print(std::ostream& oss) const{
  return oss << repr();
}

std::string I3LikelihoodEllipse::repr() const{
  std::ostringstream result;
  result << "{I3LikelihoodEllipse:" << std::endl;
  result << "  Center            :" << GetCenter() << std::endl;
  result << "  InverseCovariance :" << GetInverseCovariance() << std::endl;
  result << "  nLLH              :" << GetNLLH() << std::endl;
  result << "  Axis_Names        :" << GetAxisNames() << "}" << std::endl;
  result << std::endl;
  return result.str();
}


std::ostream& operator<<(std::ostream& oss, const I3LikelihoodEllipse& h){
  return(h.Print(oss));
}






/**
 * @brief Vector container for the generated ellipses to simplify calculations
 * 
 * This class holds multiple ellipsoids and gives some convenience functions to
 * make life a bit simpler.
 */
I3LikelihoodEllipseCollection::~I3LikelihoodEllipseCollection() { }

I3LikelihoodEllipseCollection::I3LikelihoodEllipseCollection(std::vector<I3LikelihoodEllipse> ellipses){
  axis_names_ = ellipses[0].GetAxisNames();

  for(uint i=0; i!=ellipses.size(); ++i){
    bool different_lengths = false;
    std::vector<std::string> current_axis = ellipses[i].GetAxisNames();
    if( (axis_names_.size() != current_axis.size() ) or 
	!std::equal(axis_names_.begin(), axis_names_.end(), current_axis.begin()) ){
      log_fatal("Ellipses have different axes. These should not be in a "
		"single I3LikelihoodEllipseCollection object");
    }
    ellipses_.push_back(ellipses[i]);
  }
}


void I3LikelihoodEllipseCollection::Clear(){ ellipses_.clear(); }

std::ostream& I3LikelihoodEllipseCollection::Print(std::ostream& oss) const{
  return oss << repr();
}


std::string I3LikelihoodEllipseCollection::repr() const{
  std::ostringstream result;
  std::vector<I3LikelihoodEllipse>::const_iterator iter;
  for(iter=ellipses_.begin(); iter!=ellipses_.end(); ++iter){
    result << iter->repr() << std::endl;
  }

  return result.str();
}


std::vector<I3LikelihoodEllipse> I3LikelihoodEllipseCollection::GetEllipses() const { return ellipses_; }
void I3LikelihoodEllipseCollection::SetEllipses(std::vector<I3LikelihoodEllipse> ellipses){ ellipses_ = ellipses; }

uint I3LikelihoodEllipseCollection::GetSize() { return ellipses_.size(); }

bool I3LikelihoodEllipseCollection::In(I3LikelihoodEllipse ellipse) {
  return std::find(ellipses_.begin(), ellipses_.end(), ellipse) == ellipses_.end();
}

std::vector<I3LikelihoodEllipse>::const_iterator I3LikelihoodEllipseCollection::begin() const { return ellipses_.begin(); }
std::vector<I3LikelihoodEllipse>::const_iterator I3LikelihoodEllipseCollection::end() const { return ellipses_.end(); }

void I3LikelihoodEllipseCollection::AddEllipse(I3LikelihoodEllipse ellipse){ 
  if(ellipses_.size() == 0){
    ellipses_.push_back(ellipse);
    axis_names_ = ellipse.GetAxisNames();
    return;
  }

  bool different_lengths = false;
  std::vector<std::string> names = ellipse.GetAxisNames();
  if( (axis_names_.size() != names.size() ) or
      !std::equal(axis_names_.begin(), axis_names_.end(), names.begin()) ){
    log_fatal("Ellipses have different axes. These should not be in a "
	      "single I3LikelihoodEllipseCollection object");
  }
  ellipses_.push_back(ellipse);

}
void I3LikelihoodEllipseCollection::push_back(I3LikelihoodEllipse ellipse){  AddEllipse(ellipse); }

I3LikelihoodEllipse I3LikelihoodEllipseCollection::GetEllipse(uint i) const { return ellipses_[i]; } 
I3LikelihoodEllipse I3LikelihoodEllipseCollection::operator[](uint i) const { return ellipses_[i]; }
void I3LikelihoodEllipseCollection::SetEllipse(uint i, I3LikelihoodEllipse ellipse){ ellipses_[i] = ellipse; }


void I3LikelihoodEllipseCollection::RemoveEllipse(uint i){ ellipses_.erase(ellipses_.begin() + i); }
void I3LikelihoodEllipseCollection::erase(uint i){         ellipses_.erase(ellipses_.begin() + i); }

bool I3LikelihoodEllipseCollection::Contains(const std::vector<double>& point){
   std::vector<I3LikelihoodEllipse>::iterator iter;
   for(iter=ellipses_.begin(); iter!=ellipses_.end(); ++iter){
     if( iter->Contains(point) ) return true;
   } 
   return false;
}


I3LikelihoodEllipseCollection I3LikelihoodEllipseCollection::Profile(uint dimension)
{
  std::vector<uint> dimensions;
  dimensions.push_back(dimension);
  return Profile(dimensions);
}


I3LikelihoodEllipseCollection I3LikelihoodEllipseCollection::Profile(std::vector<uint> dimensions)
{
  std::vector<I3LikelihoodEllipse> new_ellipses;
  
  for(uint i=0; i<ellipses_.size(); ++i){
    new_ellipses.push_back( I3LikelihoodEllipse(ellipses_[i].Profile(dimensions)) );
  }

  return I3LikelihoodEllipseCollection(new_ellipses);
}


I3LikelihoodEllipseCollection I3LikelihoodEllipseCollection::Prune(double maxDeltaNLLH)
{
  // Find the best LLH from the ellipses
  double best_nllh = std::numeric_limits<double>::max();
  
  for(uint i=0; i<ellipses_.size(); ++i){
    double nllh = ellipses_[i].GetNLLH();
    if(nllh <= best_nllh) best_nllh = nllh;
  }

  std::vector<I3LikelihoodEllipse> new_ellipses;
  for(uint i=0; i<ellipses_.size(); ++i){
      double nllh = ellipses_[i].GetNLLH();
      if (nllh < best_nllh + maxDeltaNLLH){
	new_ellipses.push_back(ellipses_[i]);
      }
  }

  // Remove duplicates as well
  std::vector<I3LikelihoodEllipse> unique_ellipses;
  unique_ellipses.push_back(new_ellipses[0]);
  for(uint i=0; i<new_ellipses.size(); ++i){
    bool keep_ellipse = true;
    for(uint j=0; j<unique_ellipses.size(); ++j){
      if(new_ellipses[i] == unique_ellipses[j]){
	keep_ellipse = false;
	break;
      }
    }
    if(keep_ellipse) unique_ellipses.push_back(new_ellipses[i]);
  }

  return I3LikelihoodEllipseCollection(unique_ellipses);
}



void I3LikelihoodEllipseCollection::Extend(std::vector<I3LikelihoodEllipse> ellipses){ 
  std::vector<I3LikelihoodEllipse>::iterator iter;
  for(iter=ellipses.begin(); iter!=ellipses.end(); ++iter){
    ellipses_.push_back(*iter);
  }
}
  
bool I3LikelihoodEllipseCollection::operator!=(const I3LikelihoodEllipseCollection& rhs) const { return !(*this == rhs); } 
bool I3LikelihoodEllipseCollection::operator==(const I3LikelihoodEllipseCollection& rhs) const {
  return std::equal(ellipses_.begin(), ellipses_.end(), rhs.GetEllipses().begin());
}
 

template <class Archive>
void I3LikelihoodEllipseCollection::serialize(Archive& ar, unsigned version)
{
  ar & make_nvp("I3FrameObject", base_object<I3FrameObject>(*this));
  ar & make_nvp("ellipses",  ellipses_ );
  }


std::ostream& operator<<(std::ostream& oss, const I3LikelihoodEllipseCollection& h){
  return(h.Print(oss));
} 



I3_SERIALIZABLE(I3LikelihoodEllipse);
I3_SERIALIZABLE(I3LikelihoodEllipseCollection);
