
In this homework we will work in small teams/pairs to start understanding thread programming using pthreads.

One person submits a program for her team, hw0.cpp, via Canvas that compiles and runs on linux. We should get it substantially finished today in class, but your team can polish it up and submit before the Saturday due date.

You are provided with a zip file, hw0_setup.zip Download hw0_setup.zip that contains:

ThreadGroup.h -- a class that uses pthreads to create threads and allows you to wait for all of them to finish
hw0_setup.cpp -- the sequential version of the program
example.cpp -- example of how to use the ThreadGroup class
Makefile -- in order to build it all
Here's the setup process:

Create a working directory on CS1.
Copy in and unzip the provided file.
Look at the provided files to understand them.
Build and run the example and sequential version of the programs
Here's a dialog of my doing the last three steps of the setup:

$ unzip hw0_setup.zip
Archive:  hw0_setup.zip
inflating: example.cpp             
inflating: hw0_setup.cpp           
inflating: ThreadGroup.h           
inflating: Makefile

$ pwd
/home/st/testlundeenk/cpsc5600/hw0

$ ls
example.cpp  hw0_setup.cpp  hw0_setup.zip  Makefile  ThreadGroup.h

$ make all
g++ -std=c++11 -Wall -Werror -pedantic -pthread example.cpp -o example
g++ -std=c++11 -Wall -Werror -pedantic -pthread hw0_setup.cpp -o hw0_setup
Made it all!

$ ./example
Starting 4 threads.
Hello from thread 2
Hello from thread Hello from thread 0
1
Hello from thread 3
All threads are now done!
thread 0 wrote: done 0
thread 1 wrote: done 1
thread 2 wrote: done 2
thread 3 wrote: done 3

$ time ./hw0_setup
[0]: 6
[500000]: 6
[end]: 2

real	0m9.861s
user	0m9.860s
sys	0m0.001s
Note the interweaving of the various threads' outputs to the console in the example program. Each run will be different.

Your job now is to create hw0.cpp that performs the same task as hw0_setup.cpp, but using a parallel algorithm using two threads to do so.

You are not allowed to modify the main, encode, or decode functions in hw0.cpp, nor anything in ThreadGroup.h.
The sequential program is a kind of prefix sum, except that each value in the array is first (expensively) encoded before it is added into the sum and the prefix sums are then (expensively) decoded before putting them back in the original array as the result. The idea is that we are simulating a more complex encoding and decoding function.

You should be able to substantially speed up the program (can you get a factor of 2?) by using two threads running at the same time.

Here's a hint: First encode all the values in the data array with two threads, then do the accumulation in the main thread, and then use two threads again to decode everything.

image.png

Another hint: Start with a smaller array so you can print it out, etc., during debugging.

If you choose to work on a machine other than CS1, be aware that your program will be tested on CS1 for grading. Also, pthreads does not work natively on Windows, so you'd have to do some things to get it working there. (I haven't tried it for this homework, but cygwin would probably work to get pthreads reasonably emulated on Windows.)

Guidance:
Here's a piece of my solution that is following the hint given above:

    ThreadGroup<EncodeThread> encoders;
    encoders.createThread(0, &ourData);
    encoders.createThread(1, &ourData);
    encoders.waitForAll();
The encoding code is in EncodeThread's ()-operator method. The ourData refers to a structure that contains information about which thread owns which part of the data array and a pointer to the data array itself. There is a similar group of lines for the decoding, though in that case it is ThreadGroup<DecodeThread>, a different but related class that decodes the array entries.

The Unix time Program
I often have questions about the time program's output. Our goal is to reduce the "real" number by 2x and keep the "user" number close to the same as the sequential version of the program.

time real vs user

C++ Style Guidlines:
Here's a quick list:
Doxygen-style (like Javadoc) comments on all public classes and methods. File header comments. Important protected/private members, too, deserve this same treatment.
Consistent indenting and curly-brace placement.
Good variable names (lowercase initial letters for variables, uppercase initial letters for types)
Additional comments to make the code clear. Neither too laconic nor too verbose.
Easy to read. Good use of whitespace around operators, parentheses, between methods (and fairly consistent). No redundant parentheses. Keep private and public sections together rather than interspersed.
Good design. Never have an object that is invalid or fails to satisfy invariants (handle this in the constructor as necessary).
No memory leaks or misuse of memory (use valgrind to check this).
Use constructor initialization lists where possible.
No global variables (except constants).
Prefer methods over inlining code repeatedly. No copied and pasted code segments unless it's actually more clear to read (rarely).
Ask yourself if you'd want your code published for the world to see. You should feel proud of it.

Submit
Submit your group's hw0.cpp to Canvas here (only one member needs to do this).

Keep in mind that the grader will replace the driver, the encode and decode functions, and change the size and initial contents of the array.

Version
Last updated: 8-Jan-2026 - added visual for hint; extended due date; added submission section