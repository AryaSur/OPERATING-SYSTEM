								   ASSIGNMENT-3
							        OPERATING SYSTEM 2
							      SURBHI(CS22BTECH11057)
							     Dynamic Matrix Squaring
HOW TO RUN THE ABOVE CODE IN TERMINAL?
g++ Assgn3_Src-CS22BTECH11057-.cpp
./a.out

EXPLANATION OF CODE:-
TASLock: Implements a Test-and-Set lock mechanism.
CASLock: Implements a Compare-and-Swap lock mechanism.
BoundedCASLock: Implements a Bounded Compare-and-Swap lock mechanism.
AtomicIncrement: Implements atomic increment operation.
MultiplyChunkTAS(): Function to perform matrix multiplication in a chunk with TAS lock.
MultiplyChunkCAS(): Function to perform matrix multiplication in a chunk with CAS lock.
MultiplyChunkBoundedCAS(): Function to perform matrix multiplication in a chunk with Bounded CAS lock.
MultiplyChunkAtomic(): Function to perform matrix multiplication in a chunk with atomic increment.


Input Format:-
N: Size of the square matrix (N x N).
K: Number of threads to use for parallel execution.
rowInc: Number of rows each thread should process.
aij: Elements of the matrix, where i represents the row number and j represents the column numbe

Output:-
out.txt file contains the resultant square matrix. 
Time taken to compute the square matrix of TAS, CAS, BOUNDED CAS, ATOMIC

EXAMPLE OF INPUTS:-
EXAMPLE-1 

INPUT:-
3 2 1
1 2 3
4 5 6
7 8 9


OUTPUT:-
Resultant Matrix:
30 36 42 
66 81 96 
102 126 150 
Time Taken (TAS): 0.000369 seconds
Time Taken (CAS): 0.00026 seconds
Time Taken (Bounded CAS): 0.000238 seconds
Time Taken (Atomic): 0.000166 seconds

