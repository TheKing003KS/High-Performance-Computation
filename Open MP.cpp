#include <iostream>
#include <chrono>
#include <vector>
#include <omp.h>                // For Open MP functionality. // OpenMP functionality has to be explicitily turned on in the project settings.
using namespace std ;

void HelloOpenMP ()
{
    omp_set_num_threads(8) ;                    // Statement 1
    #pragma omp parallel                      // Statement 2
    {
        #pragma omp critical                    // Statement 3
        {
            cout << "Hello, Open MP " << omp_get_thread_num() << "/" << omp_get_num_threads() << endl ;
        }
    }
}
int main ()
{
    HelloOpenMP () ;
    return 0 ;
}

/* 
'#pragma omp parallel' is the explicit way of telling the compiler to implement parallelization.
By default the compiler splits up the task into no. of parts depending upon the no. of cores or threads available in the processor.
i.e. In a quad core processor, there're 4 threads.
'omp_set_num_threads(8)' is the way to change the no. of threads on which parallalization is to be implemented.
'#pragma omp critical' is the way to prevent the concurrent execution of a block of code in diff threads.

When only statement 2 is present, 
The statemet gets printed 4 times. But the complete statement isn't printed together & is jumbled up, bcz all the diff threads are trying to write to the same output
window at the same time.

When statement 1 is mentioned with 2,
The output is 8 times but still jumbeled up due to the same reasons.

When statement 3 is mentioned with 1 & 2,
The output is now processed in only a single thread at once i.e. there's no concurrent execution in multi threads, therefore there's no jumbling up.
But since the output statement is still under the paralle directive, the output statement gets executed in all the threads ony by one, giving 8 statements.
*/



////////////////////////////////////////////////    WORK SHARING

void Sections ()
{
    #pragma omp parallel sections
    {
        #pragma omp section
        {
            for (int i = 1 ; i <= 1000 ; i++)
            {
                cout << i ;
            }
        }
        #pragma omp section
        {
            for (int i = 1 ; i <= 1000 ; i++)
            {
                cout << static_cast<char>('a' + i%26) ;
            }
        }
    }
}

void Single_Master ()
{
    #pragma omp parallel
    {
        #pragma omp single
        {
            cout << "On Single " << omp_get_thread_num() << endl ;
        }
        cout << "On Parallel " << omp_get_thread_num() << endl ;
        #pragma omp barrier 
        #pragma omp master
        {
            cout << "On Master " << omp_get_thread_num() << endl ;
        }
    }
}

/*
'Sections' is the way to assign blocks of code to diff threads. These diff sections are executed concurrently in diff threads.
Therefore the output of the above 2 sections'll be mixed up.
*/
/*
'Single' directive is the instruction to execute a block of code only once through any of the threads. Therefore it's generally used for input & output operations.
It has a barrier at its end by default. i.e. Until the block of code under 'single' is completely executed, the execution won't go further in prog in any of the threads.

'Master' directive is the explicit instruction to execute a block of code through the master thread.

'Barrier' directive is to manually create a barrier before the further execution of the prog. i.e execution'll go further only when all the execution through all the diff
threads before the barrier has been completed.
If the 'barrier' directive wasn't mentioned, then the master statemnt'll be printed in b/w the parallel statements. As if one of the threads gets free it moves on to the
next part of the program.
*/

void Ordered ()
{
    cout << endl << "ORDERED" << endl ;
    vector<int> squares ;
    #pragma omp parallel for ordered
    {
        for (int i = 0 ; i < 20 ; i++)
        {
            cout << omp_get_thread_num() << " : " << i << "\t" ;
            #pragma omp ordered
            squares.push_back(i*i) ;
        }
        for (auto v : squares)
        {
            cout << v << "\t" ;
        }
    }
}
/*
If the ordered pragma isn't mentioned then, the values of squares in the vector will be stored in a random arrangement & not in the order of incrementation of 'i'.
This is because the diff values of 'i' are processed concurrently in diff threads & then they'll be stored in the random manner.
i.e. The ordered directive makes the memory read & write as if the execution was sequential but the computation in the background is concurrent.
*/

/*
'#pragma omp nowait' directive removes the barrier if present by default in some other directive.
*/

void Atomic ()
{
    cout << endl << "ATOMIC" << endl ;
    int sum = 0 ;
    set_omp_num_threads(64) ;
    #pragma omp parallel for
    for (int i = 0 ; i < 100 ; i++)
    {
        #pragma omp atomic
        sum++ ;
    }
    cout << "Sum = " << sum << endl ;
}

/*
if the 'atomic' directive isn't mentioned then there'll be multiple writing in the 'sum' variable by all the diff threads in a single iteration due to which we 
won't get the correct value in the 'sum' variable in the end.
After mentioning the 'atomic' directive, the data'll be written into 'sum' only once in a single iteration & therefore we'll get the desired result.
*/

void DataSharing ()
{
    int i = 10 ;
    #pragma omp parallel for shared                 // Statement 1
    #pragma omp parallel for private(i)             // Statement 2
    #pragma omp parallel for firstprivate(i)        // Statement 3
    #pragma omp parallel for lastprivate(i)         // Statement 4
    {
        for (int a = 1 ; a < 10 ; a++)
        {
            cout << omp_get_thread_num() << " : i = " << i << endl ;
            i = 1000 +  omp_get_thread_num() ;
        }
    }
    cout << i ;                                     // Statement 5
}

/*
If 'Statement 1' mentioned, then the variable 'i' 'll 've only a single copy in the memory & all the diff threads will read & write on it.
And therefore the output of statement 5 'll be the last change made to the variable by any of the threads.
All the variables except the loop counter variable 're shared by default.

If 'Statement 2' mentioned, then each thread'll 've it's own copy of 'i'. But these private copies of 'i' of each of the threads won't be initialized & hence would be
some junk value. But these private vars can be written on by each of the threads.
The output of statement 5 'll the value of 'i' local to the fn 'DataSharing()' (i.e. initial value of 10) as the scope of the 'i' private to threads finishes at the end 
of the loop.

If 'Statement 3' mentioned, then each thread'll 've its own copy of 'i' but these 'i' 'll be initialised by the initial val of 'i' in the fn.
The output of statement 5 'll be 10.

If 'Statement 4' mentioned, then each thread'll 've its own un-initialized copy of 'i'.
But at the end of the loop, the value of the fn 'i' will be updated to the value of the 'i' of the last thread which executes.
*/

// If parallelization is done on a loop then, the diff threads execute diff iterations of the loop concurrently & the same iteration isn't repeated by any of the threads.