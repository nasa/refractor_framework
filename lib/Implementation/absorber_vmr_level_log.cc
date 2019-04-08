#include <boost/bind.hpp>
#include <boost/make_shared.hpp>
#include "absorber_vmr_level_log.h"
#include "ostream_pad.h"
#include "linear_interpolate.h"
#include "mapping_log.h"

using namespace FullPhysics;
using namespace blitz;

//-----------------------------------------------------------------------
/// Constructor.
//-----------------------------------------------------------------------

AbsorberVmrLevelLog::AbsorberVmrLevelLog(const boost::shared_ptr<Pressure>& Press,
                                         const blitz::Array<double, 1>& Vmr,
                                         const blitz::Array<bool, 1>& Vmr_flag,
                                         const std::string& Gas_name)
: AbsorberVmrLevel(Press, Vmr, Vmr_flag, Gas_name, boost::make_shared<MappingLog>())
{
}

void AbsorberVmrLevelLog::print(std::ostream& Os) const
{
    OstreamPad opad(Os, "    ");
    Os << "AbsorberVmrLevelLog:\n"
       << "  Gas name:       " << gas_name() << "\n"
       << "  Coefficient:\n";
    opad << coefficient_unmapped().value() << "\n";
    opad.strict_sync();
    Os << "  Retrieval Flag:\n";
    opad << used_flag << "\n";
    opad.strict_sync();
}
