#ifndef GULLIVER_BOOTSTRAP_BOOTSTRAPPARAMSCONVERTER_H
#define GULLIVER_BOOTSTRAP_BOOTSTRAPPARAMSCONVERTER_H

#include <tableio/I3Converter.h>
#include <gulliver-bootstrap/BootstrapParams.h>

class BootstrapParamsConverter : public I3ConverterImplementation<BootstrapParams> {
private:
	I3TableRowDescriptionPtr CreateDescription(const BootstrapParams& p);
	size_t FillRows(const BootstrapParams& p, I3TableRowPtr rows);
};

#endif //GULLIVER_BOOTSTRAP_BOOTSTRAPPARAMSCONVERTER_H