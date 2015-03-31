## Tools
Playing around with C++11/14 and TMP

## Setup the environment
Visual studio 2013 (vc120) project

extra dependencies (such as boost) in dependencies (see dependencies/README.MD)

## TODO
http://www.cl.cam.ac.uk/~pes20/weakmemory/

comment std::call_once (if called and throw exception, call_once will be called again)

move all the project to subfolder vc120

default (everywhere there's NOTHING destructor for instance) / delete method (everywhere private constructor)

Comment for ATOMIC : (http://stackoverflow.com/questions/15056237/which-is-more-efficient-basic-mutex-lock-or-atomic-integer)

Locks actually suspend thread execution, freeing up cpu resources for other tasks, but incurring in obvious context-switching overhead when stopping/restarting the thread. On the contrary, threads attempting atomic operations don't wait and keep trying until success (so-called busy-waiting), so they don't incur in context-switching overhead, but neither free up cpu resources.

Summing up, in general atomic operations are faster if contention between threads is sufficiently low. You should definitely do benchmarking as there's no other reliable method of knowing what's the lowest overhead between context-switching and busy-waiting.
