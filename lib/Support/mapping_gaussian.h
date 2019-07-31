#ifndef MAPPING_GAUSSIAN_H
#define MAPPING_GAUSSIAN_H

#include <blitz/array.h>

#include "array_ad.h"
#include "mapping_imp_base.h"
#include "pressure.h"

namespace FullPhysics {
/****************************************************************//**
  This class implements Gaussian parameterization of coeffs for the
  retrieval view  while using the calculated values for the forward
  model view.

  For additional information see docs for MappingImpBase class.
*******************************************************************/
class MappingGaussian : public MappingImpBase  {
public:
    //-----------------------------------------------------------------------
    /// Default Constructor.
    //-----------------------------------------------------------------------

    MappingGaussian(boost::shared_ptr<Pressure>& in_press) : press(in_press), map_name("Gaussian") {};

    const boost::shared_ptr<Pressure>& pressure() const
    {
      return press;
    }


    //-----------------------------------------------------------------------
    /// Total aerosol optical depth of the extinction values in aext.
    //-----------------------------------------------------------------------
      virtual AutoDerivative<double> total_component() const
      {
        mutable ArrayAd<double, 1> component;
        ArrayAd<double, 1> pressure_grid = pressure()->pressure_grid().value;
        AutoDerivative<double> tot_component = 0.0;
        for(int layer_idx = 0; layer_idx < pressure()->number_layer(); layer_idx++) {
          AutoDerivative<double> delta_press = (pressure_grid(layer_idx + 1) - pressure_grid(layer_idx)) / 2.0;
          tot_component += (delta_press * (component(layer_idx) + component(layer_idx + 1) ));
        }
        return tot_component;
      }
    //-----------------------------------------------------------------------
    /// Calculation of forward model view of coeffs with mapping applied
    //-----------------------------------------------------------------------

    virtual const ArrayAd<double, 1> fm_view(ArrayAd<double, 1> const& updated_coeff) const {
        mutable ArrayAd<double, 1> component;
        AutoDerivative<double> desired_val;

        desired_val = updated_coeff(0);

        // Don't let component value go lower than a minimum value. Not clear if this
        // is actually what we want to do, see ticket #2252 for this
        // suggestion. We might instead want to use a constrained solver.
        if(desired_val < min_desired) {
            desired_val = min_desired;
        }

        int ngaussians = int((updated_coeff.rows() - 1) / 2);

        ArrayAd<double, 1> pressure_grid = pressure()->pressure_grid().value;
        AutoDerivative<double> surface_press = pressure()->surface_pressure().value;

        component.resize(pressure()->number_level(), updated_coeff.number_variable());

        for(int g_idx = 0; g_idx < ngaussians; g_idx++) {
            AutoDerivative<double> p0 = updated_coeff(g_idx * 2 + 1);
            AutoDerivative<double> sigma = updated_coeff(g_idx * 2 + 2);

            for(int lev = 0; lev < component.rows(); lev++) {
                AutoDerivative<double> p = pressure_grid(lev) / surface_press;
                AutoDerivative<double> g_eval = std::exp( -1 * (p - p0) * (p - p0) / (2 * sigma * sigma) );

                // Because its not that easy to init ArrayAd from python to 0.0
                if (g_idx == 0) {
                    component(lev) = g_eval;
                } else {
                    component(lev) = component(lev) + g_eval;
                }
            }
        }

        AutoDerivative<double> scaling_N;

        // Protect against a divide by zero
        if (total_component().value() != 0.0) {
            scaling_N = desired_val / total_component();
        } else {
            scaling_N = 0.0;
        }

        for(int lev = 0; lev < component.rows(); lev++) {
            component(lev) = component(lev) * scaling_N;
        }
        return component;
    };

    //-----------------------------------------------------------------------
    /// Calculation of initial retrieval view  of coeffs
    //-----------------------------------------------------------------------
    virtual const ArrayAd<double, 1> retrieval_init(ArrayAd<double, 1> const& initial_coeff) const {
        return initial_coeff;
    };

    //-----------------------------------------------------------------------
    /// Assigned mapping name
    //-----------------------------------------------------------------------

    virtual std::string name() const { return map_name; }

    //-----------------------------------------------------------------------
    /// Pressure portion of the state
    /// Note that levels that define the layers used in the Radiative Transfer
    /// calculation may vary as we do a retrieval.
    //-----------------------------------------------------------------------
    boost::shared_ptr<Pressure> press;



    virtual ~MappingGaussian() {};

private:
    std::string map_name;
    static const double min_desired;

};
const double MappingGaussian::min_desired = 1e-9;
}

#endif
