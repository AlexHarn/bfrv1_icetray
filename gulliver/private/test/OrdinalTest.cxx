#include <I3Test.h>
#include <gulliver/utilities/ordinal.h>
#include <string>

using std::string;

TEST_GROUP(Ordinal);

TEST(first_nine){
  ENSURE("0th" == string(ordinal(0)));
  ENSURE("1st" == string(ordinal(1)));
  ENSURE("2nd" == string(ordinal(2)));
  ENSURE("3rd" == string(ordinal(3)));
  ENSURE("4th" == string(ordinal(4)));
  ENSURE("5th" == string(ordinal(5)));
  ENSURE("6th" == string(ordinal(6)));
  ENSURE("7th" == string(ordinal(7)));
  ENSURE("8th" == string(ordinal(8)));
  ENSURE("9th" == string(ordinal(9)));
}

TEST(other_edge_cases){
  ENSURE("11th" == string(ordinal(11)));
  ENSURE("19th" == string(ordinal(19)));
  ENSURE("21st" == string(ordinal(21)));
  ENSURE("22nd" == string(ordinal(22)));
  ENSURE("23rd" == string(ordinal(23)));
  ENSURE("24th" == string(ordinal(24)));
  ENSURE("101st" == string(ordinal(101)));
  ENSURE("102nd" == string(ordinal(102)));
  ENSURE("103rd" == string(ordinal(103)));
  ENSURE("104th" == string(ordinal(104)));
}