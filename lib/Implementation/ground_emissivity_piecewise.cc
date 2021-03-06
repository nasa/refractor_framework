#include "ground_emissivity_piecewise.h"
#include "ostream_pad.h"
#include <boost/lexical_cast.hpp>

using namespace FullPhysics;
using namespace blitz;

GroundEmissivityPiecewise::GroundEmissivityPiecewise(const ArrayWithUnit<double, 1>& spectral_points,
                                                     const blitz::Array<double, 1>& emissivity_values,
                                                     const blitz::Array<bool, 1>& retrieval_flag)
: GroundPiecewise(spectral_points, emissivity_values, retrieval_flag)
{
}

boost::shared_ptr<Ground> GroundEmissivityPiecewise::clone() const
{
    return boost::shared_ptr<Ground>(new GroundEmissivityPiecewise(spectral_points_, coefficient().value(), used_flag));
}

std::string GroundEmissivityPiecewise::sub_state_identifier() const {
    return "ground/emissivity_piecewise";
}

std::string GroundEmissivityPiecewise::state_vector_name_i(int i) const
{
    return "Ground Emissivity Piecewise at " + boost::lexical_cast<std::string>(spectral_points_.value(i)) + " " + spectral_points_.units.name();
}

void GroundEmissivityPiecewise::print(std::ostream& Os) const
{
    OstreamPad opad(Os, "    ");
    Os << "GroundEmissivityPiecewise:" << std::endl;

    Array<double, 1> point_vals = spectral_points_.value;
    opad << point_vals.rows() << " emissivity coefficients (" << point_vals(0) << "-" << point_vals(point_vals.rows()-1) << " " << spectral_points_.units.name() << ")" << std::endl;

    opad.strict_sync();
}

std::string GroundEmissivityPiecewise::desc() const {
    return "GroundEmissivityPiecewise";
}
