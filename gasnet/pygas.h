#ifndef _PYGAS_PYGAS_H_
#define _PYGAS_PYGAS_H_

#include "Python.h"
#include "gasnet.h"

/* make this code a little more UPC-like */
#define THREADS (gasnet_nodes())
#define MYTHREAD (gasnet_mynode())

/* Active message handler id's. */
#define APPLY_DYNAMIC_REQUEST_HIDX 144
#define APPLY_DYNAMIC_REPLY_HIDX   145
#define RMALLOC_REQUEST_HIDX       146
#define RMALLOC_REPLY_HIDX         147

/* Redefine BLOCKUNTIL macro to include calls to Py_MakePendingCalls
 * so interpreter can make progress when blocked. */
// TODO check implications of this with gasnet team
#define PYGAS_GASNET_BLOCKUNTIL(cond) \
    while (!(cond)) {             \
        gasnet_AMPoll();          \
	    Py_MakePendingCalls();    \
    }

/* Reimplementation of gasnet_barrier_wait that allows pending
 * Python calls created by incoming AMs to be serviced by the
 * interpreter while waiting.
 * XXX: spins. check with gasnet team for advice here.
 * TODO: support for other gasnet_barrier_try return codes. */
#define PYGAS_GASNET_BARRIER_WAIT(id, flags) \
    while (gasnet_barrier_try((id), (flags)) != GASNET_OK) { \
        Py_MakePendingCalls(); \
    }

/* Useful macros */
#define max(i,j) (((i)>(j))?(i):(j))
#define min(i,j) (((i)<(j))?(i):(j))

/* Controls how big messages are handled. */
// TODO autotune to find optimal threshold.
#define PYGAS_BIGMSG_PIPELINE 1
#define PYGAS_BIGMSG_RMALLOC (!(PYGAS_BIGMSG_PIPELINE))

#endif /* define _PYGAS_PYGAS_H_ */
