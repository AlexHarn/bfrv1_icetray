#include <gulliver-bootstrap/BootstrapParamsConverter.h>

I3TableRowDescriptionPtr BootstrapParamsConverter::CreateDescription(const BootstrapParams& p){
	I3TableRowDescriptionPtr desc(new I3TableRowDescription());
	MAKE_ENUM_VECTOR(status, BootstrapParams,
					 BootstrapParams::ResultStatus, (OK)(Underflow)(Overflow)(NoValidFits));
	desc->AddEnumField<BootstrapParams::ResultStatus>("status", status, "", "Status of Error Estimation");
	desc->AddField<uint32_t>("successfulFits", "", "Number of input fits which succeeded");
	desc->AddField<uint32_t>("totalFits", "", "Total number of input fits");
	return(desc);
}

size_t BootstrapParamsConverter::FillRows(const BootstrapParams& p, I3TableRowPtr rows){
	rows->Set<BootstrapParams::ResultStatus>("status",p.status);
	rows->Set<uint32_t>("successfulFits", p.successfulFits);
	rows->Set<uint32_t>("totalFits", p.totalFits);
	return 1;
}
