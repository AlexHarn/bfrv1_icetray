#include <icetray/load_project.h>

void register_I3TopPulseInfo();
void register_I3IceTopBaseline();

BOOST_PYTHON_MODULE(tpx)
{
  register_I3TopPulseInfo();
  register_I3IceTopBaseline();
}
