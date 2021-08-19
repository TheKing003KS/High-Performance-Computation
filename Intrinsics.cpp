#include <intrin.h>
#include <xmmintrin.h>
#include <iostream>
using namespace std ;

// Intrinsics are the set of CPP compiler functions & instructions to directly work on the registers & hardware. In intrinsics basically a lot of assebly level instructions
// are wrapped into a single fn defined under the intrinsic header files in the compilers. therefore eliminating the need of writing assembly lang instructions in the assmbler.
// So, intrinsics is the way of using assembly fn without actually writing 'em in the code.

// This incerases the compatibility of SIMD instructions with diff systems.

// Intrinsics make operations simpler. For eg. In multiplication of 2 arrays, we don't 've to copy the data from the stack to registers & then back.
// We just 've to use the multiply fn & everything else is done by itself.

// https://software.intel.com/sites/landingpage/IntrinsicsGuide/#techs=SSE,SSE2,SSE3,SSSE3,SSE4_1,SSE4_2&expand=4928
// Link for Intel Intrinsics Guide. For the documentation of all the intrinsics fn and instructions in SSE.

void Intrinsics ()
{
    int CPUInfo[4] ;
    __cpuid(CPUInfo,1) ;        // This is the fn analogous to 'cpuid' fn in ASM. It takes 2 parameters, first one is an array where the data from the 'edx', 'ecx' & 'ebx'
                                // is stored & the 2nd argument is the value for the 'eax' register.
    
    auto a = _mm_set_ps(1, 2, 3, 4) ;   // p -> packed, s -> single precision i.e. 32 bit floating point.
    auto b = _mm_set_ps(5, 6, 7, 8) ;   // This fn returns a data type of '__m128' i.e. 4 32 bit floating points together in 128 bit sequence.
    auto c = _mm_add_ps(a, b) ;

    float f = c.m128_f32[0] ;       // For accessing a particular value out of the array.
}

// 32 bit float is -> 1 bit for the sign, 8 bits for the exponent & 23 bits for mantissa or significant feild.

// Optimization using SIMD can therefore be done by :
// 1. Writng ASM codes to directly acces the SIMD capability of the registers.
// 2. Using Compiler Intrinsics to do the same as above in a simple way.

// 3. Using Compiler Optimization Features & using specially designed optimized libraries which use SIMD techniques. Like - OptiVec, Eigen, Armadillo, LAPACK, IMSL, GMTL. 