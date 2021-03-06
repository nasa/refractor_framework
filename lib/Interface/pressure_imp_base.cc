#include "pressure_imp_base.h"
#include "fp_serialize_support.h"

using namespace FullPhysics;

#ifdef FP_HAVE_BOOST_SERIALIZATION

SUB_STATE_VECTOR_ARRAY_SERIALIZE(Pressure, SubStateVectorArrayPressure);

template<class Archive>
void PressureImpBase::serialize(Archive & ar, const unsigned int version)
{
  FP_GENERIC_BASE(Pressure);
  FP_BASE(PressureImpBase, Pressure);
  ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(SubStateVectorArrayPressure);
}

FP_IMPLEMENT(PressureImpBase);
#endif
