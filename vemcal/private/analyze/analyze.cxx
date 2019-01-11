#include <iostream>
#include <vector>
#include <string>

#include <vemcal/VEMCalAnalyzer.h>


int main(int argc, char *argv[])
{
    std::vector<std::string> file_names;
    
    // Define options and default values
    std::map<std::string, std::string> options;
    options["-pic"]="";
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
    
    VEMCalAnalyzer analyzer;
    analyzer.OpenFiles(file_names);
    analyzer.Analyze(options["-dir"]);
    //analyzer.CreatePlots(options["-pic"], options["-dir"]);
}
