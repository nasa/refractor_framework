#ifndef ABSORBER_VMR_LEVEL_LOG_H
#define ABSORBER_VMR_LEVEL_LOG_H

#include "absorber_vmr_level.h"
#include <boost/lexical_cast.hpp>

namespace FullPhysics {
/****************************************************************//**
  This class maps the state vector to the absorber VMR on each
  level.

  This particular implementation represents VMR values in log space
  in the state vector.
*******************************************************************/
class AbsorberVmrLevelLog : public AbsorberVmrLevel {
public:
    AbsorberVmrLevelLog(const boost::shared_ptr<Pressure>& Press,
                        const blitz::Array<double, 1>& Vmr,
                        const blitz::Array<bool, 1>& Vmr_flag,
                        const std::string& Gas_name);

    virtual ~AbsorberVmrLevelLog() = default;

    virtual void print(std::ostream& Os) const;
    
    virtual std::string sub_state_identifier() const
    {
        return "absorber_log_levels/" + gas_name();
    }

    virtual std::string state_vector_name_i(int i) const
    {
        return gas_name() + " Log VMR for Press Lvl " +
               boost::lexical_cast<std::string>(i + 1);
    }

};
}
#endif
