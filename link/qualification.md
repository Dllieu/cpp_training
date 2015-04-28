## Qualifications

#GS
- Good understanding of x86 micro-architecture, smp/numa, barriers, caches, simd, affinity, etc. 
- Linux NAPI, TCP/UDP, multi-cast, sources of latency, etc.  Binary and ASCII networking protocols. 
- Financial services, telecom or game-dev background. Experience working on large source base (>0.5m loc). 
- C and C++, inline asm, modern C++ - boost/C++11. Hands on binutils, gdb, perf or similar. 
- Hands on experience with lock-free, cas, aba, actor model, etc. 
- Tibco RV 
- Scripting (e.g. perl, python, lua). 

#MS
- Advanced knowledge of C++, including templates.
- TCP and Multicast communications.
- Multithreading, low-level primitives, atomic variables, mutex, condition variable, understanding of pthreads even if using C++ libraries like Boost.
- STL (Standard Template Library).
- Boost libraries including algorithms, asio, date_time, and threads
- Knowledge and experience on formal software development practices.
- Understand debugging with gdb.
- Unix low-level.
- Superlative English verbal and written communication.
- Quick to learn and curious about all things.
- Understand performance verification and optimization tools like Purify and Quantify.

Desired Skills:
- Knowledge of low latency market data vendors (eg. Exegy, SR Labs, Redline ?etc)
- Knowledge of common formats used such as FIX, FAST, etc.
- Perl and shell scripting.
- Knowledge of finance - various asset classes and their characteristics, particularly with respect to market data, settlement and clearing.
- Ability to function across geographically diverse team (NY, Toronto, Montreal,Europe,Asia).

#Scott Meyers (Fastware C++ training)
Treating speed as a correctness criterion.
Why "first make it right, then make it fast" is misguided.
Latency, initial and total.
Other performance measures.
Designing for speed.
Optimizing systems versus optimizing programs.
Most system components are "foreign."
Exercising indirect control over "foreign" components.
Examples.
CPU Caches and why they're important.
Data caches, instruction caches, TLBs.
Cache hierarchies, cache lines, prefetching, and traversal orders.
Cache coherency and false sharing.
Cache associativity.
Guidelines for effective cache usage.
Optimizing C++ usage:
Move semantics.
Avoiding unnecessary object creations.
When custom heap management can make sense.
Optimizing STL usage:
reserve and shrink_to_fit.
Range member functions.
Using function objects instead of function pointers.
Using sorted vectors instead of associative containers.
A comparison of STL sorting-related algorithms.
An overview of concurrent data structures.
Meaning of "concurrent data structure."
Use cases.
Common offerings in TBB and PPL.
Writing your own.
An overview of concurrent STL-like algorithms.
Thread management and exception-related issues.
Common offerings in TBB and PPL.
OpenMP.
Other TBB and PPL offerings.
Exploiting "free" concurrency.
Meaning of "free."
Multiple-approach problem solving.
Speculative execution.
Making use of PGO (profile-guided optimization) and WPO (whole-program optimization).
Resources for further information.
