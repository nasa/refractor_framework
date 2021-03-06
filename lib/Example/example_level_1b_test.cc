#include <boost/make_shared.hpp>

#include "unit_test_support.h"
#include "hdf_file.h"
#include "example_level_1b.h"

using namespace FullPhysics;
using namespace blitz;

BOOST_FIXTURE_TEST_SUITE(example_level_1b, GlobalFixture)

BOOST_AUTO_TEST_CASE(load_file)
{
    boost::shared_ptr<HdfFile> h_file = boost::make_shared<HdfFile>(test_data_dir() + "in/common/l1b_example_data.h5");
    ExampleLevel1b l1b_file(h_file, "2014090915251774");

    BOOST_CHECK_EQUAL(l1b_file.number_spectrometer(), 3);

    for(int spec_idx = 0; spec_idx < l1b_file.number_spectrometer(); spec_idx++) {
        double latitude_read = h_file->read_field<double, 2>("/Level1b/latitude")(0, spec_idx);
        BOOST_CHECK_CLOSE(l1b_file.latitude(spec_idx).value, latitude_read, 1e-12);

        double longitude_read = h_file->read_field<double, 2>("/Level1b/longitude")(0, spec_idx);
        BOOST_CHECK_CLOSE(l1b_file.longitude(spec_idx).value, longitude_read, 1e-12);

        double solar_zenith_read = h_file->read_field<double, 2>("/Level1b/solar_zenith")(0, spec_idx);
        BOOST_CHECK_CLOSE(l1b_file.solar_zenith(spec_idx).value, solar_zenith_read, 1e-12);

        double solar_azimuth_read = h_file->read_field<double, 2>("/Level1b/solar_azimuth")(0, spec_idx);
        BOOST_CHECK_CLOSE(l1b_file.solar_azimuth(spec_idx).value, solar_azimuth_read, 1e-12);

        double observation_zenith_read = h_file->read_field<double, 2>("/Level1b/observation_zenith")(0, spec_idx);
        BOOST_CHECK_CLOSE(l1b_file.sounding_zenith(spec_idx).value, observation_zenith_read, 1e-12);

        double observation_azimuth_read = h_file->read_field<double, 2>("/Level1b/observation_azimuth")(0, spec_idx);
        BOOST_CHECK_CLOSE(l1b_file.sounding_azimuth(spec_idx).value, observation_azimuth_read, 1e-12);

        double relative_velocity_read = h_file->read_field<double, 2>("/Level1b/relative_velocity")(0, spec_idx);
        BOOST_CHECK_CLOSE(l1b_file.relative_velocity(spec_idx).value, relative_velocity_read, 1e-12);

        double time_read = h_file->read_field<double, 2>("/Level1b/time_tai93")(0, spec_idx);
        BOOST_CHECK_CLOSE(l1b_file.time(spec_idx).pgs_time(), time_read, 1e-12);

        Array<double, 1> stokes_coefficient_read = h_file->read_field<double, 3>("/Level1b/stokes_coefficient")(0, spec_idx, Range::all());
        BOOST_CHECK_MATRIX_CLOSE_TOL(l1b_file.stokes_coefficient(spec_idx), stokes_coefficient_read, 1e-12);

        Array<double, 1> spectral_coefficient_read = h_file->read_field<double, 3>("/Level1b/spectral_coefficient")(0, spec_idx, Range::all());
        BOOST_CHECK_MATRIX_CLOSE_TOL(l1b_file.spectral_coefficient(spec_idx).value, spectral_coefficient_read, 1e-12);

        std::stringstream rad_ds_name;
        rad_ds_name << "/Level1b/radiance_" << (spec_idx + 1);
        Array<double, 1> radiance_read = h_file->read_field<double, 2>(rad_ds_name.str())(0, Range::all());

        std::stringstream uncert_ds_name;
        uncert_ds_name << "/Level1b/uncertainty_" << (spec_idx + 1);
        Array<double, 1> uncertainty_read = h_file->read_field<double, 2>(uncert_ds_name.str())(0, Range::all());

        BOOST_CHECK_MATRIX_CLOSE_TOL(l1b_file.radiance(spec_idx).data(), radiance_read, 1e2);
        BOOST_CHECK_MATRIX_CLOSE_TOL(l1b_file.radiance(spec_idx).uncertainty(), uncertainty_read, 1e2);
    }
}

BOOST_AUTO_TEST_SUITE_END()
