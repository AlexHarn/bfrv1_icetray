#include "tableio/I3Converter.h"
#include "stochastics/I3EnergyLoss.h"

class I3EnergyLossConverter : public I3ConverterImplementation<I3EnergyLoss > {
private:
    I3TableRowDescriptionPtr CreateDescription(const I3EnergyLoss &params); 
    size_t FillRows(const I3EnergyLoss &params, I3TableRowPtr rows);
};
