/**
 * @author <a href="mailto:andre.anjos@idiap.ch">Andre Anjos</a> 
 * @brief blitz::Array<std::complex<long double>,2> to and from python converters
 */
#include "core/python/array.h"
declare_complex_array(std::complex<long double>, 2, complex256, bind_core_array_complex256_2)