#include <iostream>
#include <string>
using namespace std ;

string GetCPUName ()
{
    uint32_t Data[4] {0} ;
    
    _asm                    // This is the declaration of a block of data containing 'Assembly Level Lang'.
    {
        cpuid ;                 // This is an assembly lang instruction to get the name of the CPU being currently used.
        mov Data[0], ebx ;      // 'mov' is an instruction to move the data. 'ebx', 'edx' & 'ecx' are the registers which have the data about the CPU. Since 'cpuid'
        mov Data[4], ecx ;      //  instruction is already passed therefore they'll have that info.
        mov Data[8], edx ;      //  The no. in [] aren't same as array indices. In HLL, 1 digit = group of bytes depending upon the data type.
    }                           // Whereas in Assembly lang 1 digit = 1 byte. Therefore to move from 1 int to the next we've to increment 4. i.e. 0 to 4 to 8.

    return string ((const char*)Data) ;
}

void cacl ()
{
    float f1[] = { 1.f, 2.f, 3.f, 4.f } ;
    float f2[] = { 5.f, 6.f, 7.f, 8.f } ;
    float Result[4] = { 0.f } ;

    _asm
    {                       // 'xmm1' & 'xmm2' are SIMD registers. These registers're to be used for calculations with SIMD implementation.
        movups xmm1, f1 ;   // 'movups' is SSI instruction to 'Move Unalligned Packed Structure' from the right argument to the left.
        movups xmm2, f2 ;   // Data is always stored in the left location & therefore the result of any arithematic operation'll also be stored in the left location.
        mulps xmm1, xmm2 ;  // 'mulps' is SSI instruction for 'Multiplication of Packed Structure'.
        movups Result, xmm1 ;   // Moving the result from the register back to the array.
    }

    for (size_t i = 0 ; i < 4 ; i++)
    {
        cout << Result[i] << "\t" ;
    }
    cout << endl ;
}
// Above we've multiplied 2 arrays without an iterative process, therefore instead of each element being multiplied 1 by 1, all the elements are directly multiplied 
// at the same time. This's a faster process.

void FeatureDetection ()
{
    int x,y ;

    _asm 
    {
        mov eax, 1 ;        // 'eax' is a Register whose current value decides the type of data returned by 'cpuid' instruction to 'edx', 'ecx' & 'ebx' registers.
        cpuid ;             // check CPUID documentation for the details.
        mov x, edx ;        // Taking the values returned by the 'cpuid' instruction.
        mov y, ecx ;
    }

    if ( (d & (1 << 26)) != 0 )
    {
        cout << "SSE2 is Supported." << endl ;
    }
    if ( (c & 1) != 0 )
    {
        cout << "SSE3 is Supported." << endl ;
    }
    if ( (c & (1 << 19)) != 0 )
    {
        cout << "SSE4.1 is Supported." << endl ;
    }
    if ( (c & (1 << 20)) != 0 )
    {
        cout << "SSE4.2 is Supported." << endl ;
    }
}

int main ()
{
    cout << "CPU is : " << GetCPUName() << endl ;
    cacl() ;
}