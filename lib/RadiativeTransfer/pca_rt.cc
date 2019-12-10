#include "ostream_pad.h"

#include "pca_rt.h"

#include "optical_properties_wrt_rt.h"

using namespace FullPhysics;
using namespace blitz;

PCARt::PCARt(const boost::shared_ptr<AtmosphereStandard>& Atm,
             const std::string Primary_absorber,
             const PCABinning::Method Bin_method, const int Num_bins,
             const int Num_eofs,
             const boost::shared_ptr<StokesCoefficient>& Stokes_coef,
             const blitz::Array<double, 1>& Sza, 
             const blitz::Array<double, 1>& Zen, 
             const blitz::Array<double, 1>& Azm,
             int Number_streams, 
             int Number_moments, 
             bool do_solar_sources, 
             bool do_thermal_emission) 
: RadiativeTransferFixedStokesCoefficient(Stokes_coef), atm(Atm), primary_absorber(Primary_absorber), 
  bin_method(Bin_method), num_bins(Num_bins), num_eofs(Num_eofs)
{
    lidort_rt.reset(new LidortRt(Atm, Stokes_coef, Sza, Zen, Azm, false, Number_streams, Number_moments, true, do_solar_sources, do_thermal_emission, do_thermal_emission));

    twostream_rt.reset(new TwostreamRt(Atm, Stokes_coef, Sza, Zen, Azm, true, do_solar_sources, do_thermal_emission));

    first_order_rt.reset(new FirstOrderRt(Atm, Stokes_coef, Sza, Zen, Azm, Number_streams, Number_moments, do_solar_sources, do_thermal_emission));

}

blitz::Array<double, 2> PCARt::stokes(const SpectralDomain& Spec_domain, int Spec_index) const
{

    // Our final output from this routine
    Array<double, 2> stokes_output(Spec_domain.data().rows(), number_stokes());

    // Convenience since other RTs take wavenumbers as input
    Array<double, 1> wavenumbers(Spec_domain.wavenumber());

    // Compute optical properties for the whole band
    pca_opt.clear();

    for (int dom_idx = 0; dom_idx < Spec_domain.data().rows(); dom_idx++) {
        boost::shared_ptr<OpticalPropertiesWrtRt> point_opt_prop(new OpticalPropertiesWrtRt());
        point_opt_prop->initialize(DoubleWithUnit(Spec_domain.data()(dom_idx), Spec_domain.units()), Spec_index,
                                   atm->absorber_ptr(), atm->rayleigh_ptr(), atm->aerosol_ptr());
        pca_opt.push_back(point_opt_prop);
    }

    int primary_abs_index = atm->absorber_ptr()->gas_index(primary_absorber);

    // Compute bins
    pca_bin.reset(new PCABinning(pca_opt, bin_method, num_bins, primary_abs_index));
    auto bins = pca_bin->bin_indexes();
    Array<int, 1> num_bin_points = pca_bin->num_bin_points();

    // Extract optical properties intermediate variable values for each bin
    int num_layers = pca_opt[0]->number_layers();
    int num_iv_var = pca_opt[0]->number_aerosol_particles() + 2;

    std::vector<Array<double, 3> > bin_inp_iv;
    for (int bin_idx = 0; bin_idx < bins.size(); bin_idx++) {
        Array<double, 3> bin_iv(num_layers, num_iv_var, bins[bin_idx].rows());

        // Copy out the intermediate variable values for the spectral domain values in the current bin
        // This requires us to copy into a new array since the bin locations won't be contiguous
        for(int dom_idx = 0; dom_idx < num_bin_points(bin_idx); dom_idx++) {
            int src_idx = bins[bin_idx](dom_idx);

            bin_iv(Range::all(), 0, dom_idx) = pca_opt[src_idx]->gas_optical_depth_per_layer().value();
            bin_iv(Range::all(), 1, dom_idx) = pca_opt[src_idx]->rayleigh_optical_depth().value();

            Array<double, 2> aerosol_ext_od(pca_opt[src_idx]->aerosol_scattering_optical_depth_per_particle().value());
            for(int aer_idx = 0; aer_idx < pca_opt[src_idx]->number_aerosol_particles(); aer_idx++) {
                bin_iv(Range::all(), 2 + aer_idx, dom_idx) = aerosol_ext_od(Range::all(), aer_idx);
            }
        }

        bin_inp_iv.push_back(bin_iv);
    }

    // Compute values for each bin
    pca_bin_solvers.clear();
    for (int bin_idx = 0; bin_idx < bins.size(); bin_idx++) {

        // Skip empty bins
        if (num_bin_points(bin_idx) == 0) {
            continue;
        }

        // Solve PCA solution for intermediate variable optical properties for the bin

        // Copy out the intermediate variable values for the spectral domain values in the current bin
        // This requires us to copy into a new array since the bin locations won't be contiguous
        std::vector<blitz::Array<double, 2> > data_list;
        for (int var_idx = 0; var_idx < num_iv_var; var_idx++) {
            Array<double, 2> var_bin_vals(bin_inp_iv[bin_idx](Range::all(), var_idx, Range::all()));
            data_list.push_back(var_bin_vals);
        }

        // Create bin averaged spectral point
        Array<double, 1> bin_wns(num_bin_points(bin_idx));
        for(int dom_idx = 0; dom_idx < num_bin_points(bin_idx); dom_idx++) {
            bin_wns(dom_idx) = wavenumbers(bins[bin_idx](dom_idx));
        }
        double avg_wn = mean(bin_wns);

        // Create PCA solution from binned intermediate variables
        boost::shared_ptr<PCAEigenSolver> pca_solver;
        pca_solver.reset(new PCAEigenSolverGeneric(data_list, num_eofs));
        pca_bin_solvers.push_back(pca_solver);
        
        // Create EOF intermediate values for mean and plus and minus each eof
        // We need to translate back from the eigen solvers vectors to arrays for each intermediate variable index
        
        // Compute mean IV values
        Array<double, 2> bin_mean_iv(num_layers, num_iv_var);
        for (int var_idx = 0; var_idx < num_iv_var; var_idx++) {
            bin_mean_iv(Range::all(), var_idx) = exp(pca_solver->data_mean()[var_idx]);
        }
        
        // Compute bin mean intensity from each RT 
        ArrayAd<double, 2> bin_mean_iv_ad(bin_mean_iv);

        Array<double, 1> lidort_mean(lidort_rt->stokes_single_wn(avg_wn, Spec_index, bin_mean_iv_ad));
        Array<double, 1> twostream_mean(twostream_rt->stokes_single_wn(avg_wn, Spec_index, bin_mean_iv_ad));
        Array<double, 1> first_order_mean(first_order_rt->stokes_single_wn(avg_wn, Spec_index, bin_mean_iv_ad));

        // Compute bin mean plus eof and mean minus eof intensity from each RT
        Array<double, 2> lidort_plus(num_eofs, number_stokes());
        Array<double, 2> twostream_plus(num_eofs, number_stokes());
        Array<double, 2> first_order_plus(num_eofs, number_stokes());

        Array<double, 2> lidort_minus(num_eofs, number_stokes());
        Array<double, 2> twostream_minus(num_eofs, number_stokes());
        Array<double, 2> first_order_minus(num_eofs, number_stokes());

        for(int eof_idx = 0; eof_idx < num_eofs; eof_idx++) {
            Array<double, 2> bin_plus_iv(num_layers, num_iv_var);
            Array<double, 2> bin_minus_iv(num_layers, num_iv_var);

            for (int var_idx = 0; var_idx < num_iv_var; var_idx++) {
                bin_plus_iv(Range::all(), var_idx) = 
                    exp(pca_solver->data_mean()[var_idx] + pca_solver->eof_properties()[var_idx](Range::all(), eof_idx));

                bin_plus_iv(Range::all(), var_idx) = 
                    exp(pca_solver->data_mean()[var_idx] - pca_solver->eof_properties()[var_idx](Range::all(), eof_idx));
            }

            ArrayAd<double, 2> bin_plus_iv_ad(bin_plus_iv);
            ArrayAd<double, 2> bin_minus_iv_ad(bin_minus_iv);


            lidort_plus(eof_idx, Range::all()) = lidort_rt->stokes_single_wn(avg_wn, Spec_index, bin_plus_iv_ad);
            lidort_minus(eof_idx, Range::all()) = lidort_rt->stokes_single_wn(avg_wn, Spec_index, bin_minus_iv_ad);

            twostream_plus(eof_idx, Range::all()) = twostream_rt->stokes_single_wn(avg_wn, Spec_index, bin_plus_iv_ad);
            twostream_minus(eof_idx, Range::all()) = twostream_rt->stokes_single_wn(avg_wn, Spec_index, bin_minus_iv_ad);

            first_order_plus(eof_idx, Range::all()) = first_order_rt->stokes_single_wn(avg_wn, Spec_index, bin_plus_iv_ad);
            first_order_minus(eof_idx, Range::all()) = first_order_rt->stokes_single_wn(avg_wn, Spec_index, bin_minus_iv_ad);

        }

        // Compute correction factors
        Array<double, 2> bin_corrections(num_bin_points(bin_idx), number_stokes());
        bin_corrections = pca_solver->correction(
                lidort_mean, twostream_mean, first_order_mean,
                lidort_plus, twostream_plus, first_order_plus,
                lidort_minus, twostream_minus, first_order_minus);

        // Compute 2stream and first order for all points in the bin and use correction to get final values
        for(int dom_idx = 0; dom_idx < num_bin_points(bin_idx); dom_idx++) {
            int grid_idx = bins[bin_idx](dom_idx);
            int dom_wn = wavenumbers(grid_idx);

            Array<double, 2> dom_iv( bin_inp_iv[bin_idx](Range::all(), Range::all(), dom_idx) );
            ArrayAd<double, 2> dom_iv_ad(dom_iv);

            Array<double, 1> twostream_full(twostream_rt->stokes_single_wn(dom_wn, Spec_index, dom_iv_ad));
            Array<double, 1> first_order_full(first_order_rt->stokes_single_wn(dom_wn, Spec_index, dom_iv_ad));

            stokes_output(grid_idx, Range::all()) = 
                bin_corrections(dom_idx, Range::all()) * (twostream_full(Range::all()) + first_order_full(Range::all()));
        }
    }

    return stokes_output;
}

ArrayAd<double, 2> PCARt::stokes_and_jacobian (const SpectralDomain& Spec_domain, int Spec_index) const
{
    return ArrayAd<double, 2>(stokes(Spec_domain, Spec_index));
} 

void PCARt::print(std::ostream& Os, bool Short_form) const 
{
    OstreamPad opad(Os, "    ");
    Os << "PCARt\n";
    Os << "  ";
    RadiativeTransferFixedStokesCoefficient::print(opad, Short_form);
    opad.strict_sync();
    Os << "\nPrimary absorber: " << primary_absorber << "\n"
       << "Number EOFs: " << num_eofs << "\n";

    Os << "\n  LIDORT RT:\n";
    lidort_rt->print(opad, Short_form);
    opad.strict_sync();

    Os << "\n  2stream RT:\n";
    twostream_rt->print(opad, Short_form);
    opad.strict_sync();

    Os << "\n  First Order RT:\n";
    first_order_rt->print(opad, Short_form);
    opad.strict_sync();
}
