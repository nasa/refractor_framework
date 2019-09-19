#ifndef PCA_RT_H
#define PCA_RT_H

#include "radiative_transfer_fixed_stokes_coefficient.h"

#include "atmosphere_standard.h"

#include "lidort_rt.h"
#include "twostream_rt.h"
#include "first_order_rt.h"

#include "pca_binning.h"

namespace FullPhysics {

/****************************************************************//**
 Implements a radiative transfer using the PCA method where
 the requested spectral domain is broken up into bins based on
 their optical properties.
 
 Each bin has higher accuracy computations done for the mean value 
 and a certain number of EOF perturbations of the mean. A 
 correction  factor is determined how to combine the binned 
 computations with low accuracy RT computation.

 This class uses LIDORT, 2steam and First Order together to 
 speed up multiple scattering RT computations.
*******************************************************************/

class PCARt : public RadiativeTransferFixedStokesCoefficient {
public:
    PCARt(const boost::shared_ptr<AtmosphereStandard>& Atm,
          const std::string Primary_absorber,
          const PCABinning::Method Bin_method, const int Num_bins,
          const int Num_eofs,
          const boost::shared_ptr<StokesCoefficient>& Stokes_coef,
          const blitz::Array<double, 1>& Sza, 
          const blitz::Array<double, 1>& Zen, 
          const blitz::Array<double, 1>& Azm,
          int Number_streams, 
          int Number_moments, 
          bool do_solar_sources = true, 
          bool do_thermal_emission = false);

    virtual ~PCARt() = default;

    virtual int number_stream() const { lidort_rt->number_stream(); }

    virtual int number_stokes() const { return stokes_coef->stokes_coefficient().cols(); }

    virtual blitz::Array<double, 2> stokes(const SpectralDomain& Spec_domain, int Spec_index) const;
    virtual ArrayAd<double, 2> stokes_and_jacobian (const SpectralDomain& Spec_domain, int Spec_index) const;

    const boost::shared_ptr<AtmosphereStandard>& atmosphere() const { return atm; }
  
    virtual void print(std::ostream& Os, bool Short_form = false) const;

private:

  boost::shared_ptr<AtmosphereStandard> atm;

  std::string primary_absorber;

  PCABinning::Method bin_method;
  int num_bins;
  int num_eofs;

  boost::shared_ptr<LidortRt> lidort_rt;
  boost::shared_ptr<TwostreamRt> twostream_rt;
  boost::shared_ptr<FirstOrderRt> first_order_rt;
};
}
#endif
