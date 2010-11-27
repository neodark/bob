/**
 * @author <a href="mailto:andre.anjos@idiap.ch">Andre Anjos</a> 
 * @brief blitz::Array<std::complex<float>,4> to and from python converters
 */
#include "core/python/array.h"
declare_complex_array(std::complex<float>, 4, complex64, bind_core_array_complex64_4)