								   ASSIGNMENT-2 
							        OPERATING SYSTEM 2
							      SURBHI(CS22BTECH11057)
							   PROGRAMMING ASSIGNMENT- Thread Affinity
							       
HOW TO RUN THE ABOVE CODE IN TERMINAL?
g++ Assgn2_Src-CS22BTECH11057.cpp
./a.out

Input Format:-
The input is read from a file named inp.txt. The format of this file should be as follows:
The first line contains four integers separated by spaces: N K C BT.
N: Size of the square matrix (N x N).
K: Number of threads to be used.
C: Number of available CPU cores.
BT: Number of threads to bind to CPU cores.
The next N lines contain N integers each, representing the elements of the square matrix.

Output:-
The output is written to a file named out.txt. It contains the following information:
The resultant matrix after multiplication.
Time taken for matrix multiplication using the chunk-based strategy.
Time taken for matrix multiplication using the mixed strategy.

Thread Affinity:-
The program utilizes thread affinity to bind threads to specific CPU cores. This is done using the pthread_setaffinity_np function.

EXAMPLE OF INPUTS:-
EXAMPLE-1 

INPUT:-
3 2 4 2
1 2 3
4 5 6
7 8 9

OUTPUT:-
Resultant Matrix:
30 36 42 
66 81 96 
102 126 150 
Time Taken (Chunk): 0.000337 seconds
Time Taken (Mixed): 0.000167 seconds

