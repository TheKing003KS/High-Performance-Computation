#include<iostream>
#include<vector>
#include<amp.h>
using namespace std ;
using namespace concurrency ;

const int Size = 3 ;

void GPURun ()
{
    int ah[] = {1,2,3} ;            // h -> Host (CPU) & d -> GPU
    int bh[] = {7,8,9} ;
    int sumh[Size] ;

    array_view<const,1> ad(size,ah) ;           // An array to be used by the GPU. 'array_view <data type,Dimensions> array name (size,pointer to the arr of CPU)' ;
    array_view<const,1> bd(size,bh) ;           // This fn creates an array for the GPU and copies all the data values form CPU array to GPU array.
    array_view<const,1> sumd(size,sumh) ;
    sumd.discard_data() ;                       // We don't want the data of 'sumh' to be copied (junk values).

    parallel_for_each (sumd.extent,[=](index<1> idx) restrict(amp) {        // Statement 1
        sumd[idx] = ad[idx] + bd[idx] ;
    });
    sumd.synchronize() ;                        // Updates the data in the CPU array.
    
    for (int i = 0 ; i < size ; i++)
    {
        cout << sumh[i] << "\t" ;
    }
    cout << endl ;
}

/* Statement 1
'parallel_for_each()' is a parallelization implemented loop in AMP. It takes the start value, end value & a lambda expression as arguments.

For 'array_view' the start & end values for a loop run can be found using 'extent'. It even supports multiple dimensions.
The extent fn creates an equal no. of threads as the the no. of elements in the extent of the array. If the no. of threads in the system is less, it creates virtual 
threads. Eg. -> if an array has 12 elements or a 2D array of (6x2), 12 threads are created by extent fn for the concurrent execution.

The other argument is the lambda which defines the process which's to be executed in the loop.
'parallel_for_each()' isn't a GPU specific fn. It can also be used on the main CPU & implements parallelization optimizations. It's widely used for iterative containers
such as those provided by the STL to optimize the process. 
In 'parallel_for_each()' we don't 've to write the increment statement for 'index'. It automatically provides different threads with all the diff values of index in
the extent provided & hence the execution for all the diff values of threads is done concurrently. 

But since we're using this fn on the GPU, therefore we use 'restrict(amp)' to restrict the functionality of this fn as per the GPU capabilities.
*/

acclerator_view Setup ()
{
    vector<acclerator> All = acclerator::get_all() ;    // 'get_all()' returns a list of all the acclerators present in the system.
    for (acclerator& a : All)                           // Since we need a list of acclerators, the vector data type is same. Acclerator is a pre-defined class.
    {                                                   // 'a' is a pointer of acclerator type.
        wcout << a.description ;                        // 'description' returns the description of a acclerator device.
        cout << " has " << a.dedicated_memory / 1e6 << " GB Dedicated Memory. " ; // 'dedicated_memory' returns the dedicated memory in KB.
        if (a.supports_cpu_shared_memory)
        {
            cout << "It supports CPU shared memory. " ;
        }
        if (a.supports_double_precision)
        {
            cout << " It supports double precision." ;
        }
        cout << endl << endl ;
    }

    auto x = All[0] ;
    bool success = acclerator::set_default(x.device_path) ;     // fn to change the default acclerator device. It takes path of a device as an argument.

    acclerator GPU = acclerator::default_acclerator ;           // Storing the default acclerator in GPU acclerator type object.
    return GPU.default_view ;                                   // 'default_view' gives the acclerator_view type data. 'acclerator_view' is like a reference to an obj 
}                                  // of acclerator type. i.e. The main data is always stored in the acclerator obj container but view is a wrapper around that container.

// An acclerator_view can be placed in the 'parallel_for_each' fn arguments, which'll explicitely specify the acclerator to run the lambda on.


////////// MATRIX MULTIPLICATION
void native_multiply(float* a, float* b, float* c, const int dim)
{
    array_view<float,2> av(dim,dim,a) ;
    array_view<float,2> bv(dim,dim,b) ;
    array_view<float,2> cv(dim,dim,c) ;
    c.discard_data() ;

    parallel_for_each (cv.extent, [=](index<2> idx) restrict(amp) {
        auto row = idx[0] ;                                         // 'idx' in 2D space has 2 values i.e. x & y co-ordinates. idx[0] -> x co-ordinate.
        auto column = idx[1] ;                                      // idx[1] -> y co-ordinate. Therefore getting the indeces of each element in the computation of that
        auto sum = 0.f ;                                            // element's thread.

        for (int i = 0 ; i < dim ; i++)
        {
            sum += av(row,i)*bv(i,column) ;
        }
        cv[idx] = sum ;
    });
    cv.synchronize() ;
}

template<int ts>                                                     // size of the tile.
void tiled_multiply (float* a, float* b, float* c, const int dim)
{
    array_view<float,2> av(dim,dim,a) ;
    array_view<float,2> bv(dim,dim,b) ;
    array_view<float,2> cv(dim,dim,c) ;
    c.discard_data() ;

    parallel_for_each (cv.extent.tile<ts,ts>, [=](tiled_index<ts,ts> idx) restrict(amp) {       // tiled overload to 'parallel_for_each()'
        tile_static float al [ts][ts] ;
        tile_static float bl [ts][ts] ;

        int rl = idx.local[0] ;             // l -> local
        int cl = idx.local[1] ;
        int rg = idx.global[0] ;            // g -> global
        int cg = idx.global[1] ;
        auto sum = 0.f ;

        for (int i = 0 ; i < dim ; i += ts)     // copying the data from the global GPU memory to its local memory (in tile_static variables).
        {
            al [rl][cl] = av(rg , cl+i) ;
            bl [rl][cl] = bv(rl+i , cg) ;
            idx.barrier.wait() ;                // to wait for all the different threads to finish copying the data before starting computation on the tile.

            for (int j = 0 ; j < ts ; j++)
            {
                sum += al[rl][j] * bl[j][cl] ;
            }
            idx.barrier.wait() ;
        }

        cv[idx.global] = sum ;                  // The result is stored directly in the global memory.
    });
    cv.synchronize() ;
}

/*      //'tiled_index<>'
It's a class for the index functionality in the tiling technique. It takes template arguments for the size of a tile. Max 3 dimensions are allowed.
It splits up the main array in the memory into diff tiles of the size provided by the user. Each element in all these tiles has a 'local' & 'global' index with respect to
a single tile & the main array respectively. These local & global indeces for a element can be accessed using the local & global functions.
*/

/*      //'tile_static'
It's a storage class for data on the GPU registers i.e a memory local to the GPU only. This storage class can only be used in 'restrict(amp)' functions & if the 
'parallel_for_each()' is tiled.
The data of each tile is copied from the global memory to these 'tile_static' variables repeatedly. This copying of data takes time but the data access speed for the 
GPU is very high in the local memory when compared to the global memory, therefore it contributes to a net reduction in time complexity.
*/

// 'extent.tile<>' divides the extent of the container into multiple tiles of the size & dimensions provided in te template arguments.

// www.ampalgorithms.codeplex.com