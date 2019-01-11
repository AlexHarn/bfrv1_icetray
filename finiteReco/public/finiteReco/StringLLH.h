/**
 * @brief declaration of the StringLLH class
 *
 * @file StringLLH.h
 * @version $Revision$
 * @date $Date$
 * @author Sebastian Euler <sebastian.euler@icecube.wisc.edu>
 *
 * This class provides an algorithm to account for hard local coincidences when calculating DOM hit probabilities. The probability is calculated string-wise. For more information about the class and the algorithm see <https://wiki.icecube.wisc.edu/index.php/FiniteReco.StringLLH>.
 */

#ifndef STRINGLLH_H_INCLUDED
#define STRINGLLH_H_INCLUDED

class StringLLH
{

 public:
  
  /// Constructor: takes a string number.
  StringLLH(const int& stringNr);
  StringLLH();
  ~StringLLH();

  /// Adds a DOM to the LLH. probHit: complete probability for the DOM to have a hit. hit: true if there was a hit. broken: true if the DOM is broken
  void AddOM(double probHit,bool hit, const bool& broken);

  /// Returns the LogProb for the string.
  double GetProb();

  /// Deletes the information of the actual string and sets all values to default.
  void NewString(const int& stringNr);

  /// Returns the number of the string.
  double GetStringNr();


 private:
  StringLLH(const StringLLH&);
  StringLLH operator= (const StringLLH& rhs);
  

  /// Number of DOMs added to the string
  int numOM_;

  /// Type of the last DOM: 1=Hit, 0=NoHit, -1=NoHit but not the first one
  int typeOM_;

  /// True if the last DOM was broken
  bool brokenOM_;

  /// Logarithmic probability for the string
  double logProb_;

  /// Probability for a hit in the last DOM
  double probLast_;

  /// Probability for a chain ending at the 2nd-to-last DOM with no removed hit in the last DOM
  double probChainNO_;

  /// Probability for a chain ending at the 2nd-to-last DOM with a removed hit in the last DOM
  double probChainYES_;

  
  /// String number
  int stringNr_;

};

#endif /*STRINGLLH*/
