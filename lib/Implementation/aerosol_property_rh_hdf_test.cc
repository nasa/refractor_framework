#include "aerosol_property_rh_hdf.h"
#include "atmosphere_fixture.h"
#include "pressure_sigma.h"
#include "unit_test_support.h"

using namespace FullPhysics;
using namespace blitz;

BOOST_FIXTURE_TEST_SUITE(aerosol_property_rh_hdf, AtmosphereFixture)

BOOST_AUTO_TEST_CASE(basic)
{
  HdfFile h(test_data_dir() + "l2_merra_aerosol_RH.h5");
  AerosolPropertyRhHdf a(h, "SS/Properties", 
			 atm->pressure_ptr(), atm->relative_humidity_ptr());
  BOOST_CHECK_CLOSE(a.extinction_coefficient_each_layer(13000).value()(0), 
		    124.46975086075555, 1e-8);
  BOOST_CHECK_CLOSE(a.extinction_coefficient_each_layer(13000).value()(17), 
		    660.70030186076372, 1e-8);
  BOOST_CHECK_CLOSE(a.scattering_coefficient_each_layer(13000).value()(0), 
		    124.46317242666989, 1e-8);
  BOOST_CHECK_CLOSE(a.scattering_coefficient_each_layer(13000).value()(17), 
		    660.700301860763721, 1e-8);
  // Phase function is large, so we just check the first couple of moments.
  Array<double, 2> pf_expect(2, 6);
  pf_expect =
    1, 0.00539705,  0.911467, 0.0146766, 1,  0.911467,
    2.37245, 0.0742819, 2.4432, -0.0646874,  2.37245, 2.4432;
  BOOST_CHECK_MATRIX_CLOSE_TOL(a.phase_function_moment_each_layer(13000).value()
			       (Range(0,1), 0, Range::all()), pf_expect, 1e-5);
}

BOOST_AUTO_TEST_SUITE_END()

