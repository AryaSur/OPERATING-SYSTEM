								  ASSIGNMENT-4
							        OPERATING SYSTEM 2
							      SURBHI(CS22BTECH11057)
			 Implement solutions to Readers-Writers (writerpreference) and Fair Readers-Writers problems using Semaphores


HOW TO RUN THE ABOVE CODE IN TERMINAL?
FOR RW:-

g++ rw-CS22BTECH11057-.cpp
./a.out

Output file:-
AverageTimeRW.txt
RW-log.txt

FOR FRW:-
g++ frw-CS22BTECH11057-.cpp
./a.out

Output file:-
AverageTimeFairRW.txt
FairRW-log.txt

INPUT FORMAT:-
nw nr kw kr μCS μRem

nw: the number of writer threads, 
nr: the number of reader threads, 
kw: the number of times each writer thread tries to enter the CS, 
kr: the number of times each reader thread tries to enter the CS,
μCS: Here randCSTime is delay value exponentially distributed with an average of μCS milli-seconds,
μRem: Here randRemTime is delay value exponentially distributed with an average of μRem milli-seconds.

AVERAGE OUTPUT FILE FORMAT:-
Average time for Fair RW:  In milliseconds

