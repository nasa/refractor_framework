%include "fp_common.i"

%{
#include "optical_properties.h"
#include "sub_state_vector_array.h"
%}

%base_import(generic_object)

%import "array_ad.i"

%fp_shared_ptr(FullPhysics::OpticalProperties)

%include "optical_properties.h"
