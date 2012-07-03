#include <stdlib.h>
#include <assert.h>

#include "upc.h"

#define MAXBYTES (1 << 18)
#define NUMTRIALS 1000

int main(int argc, char **argv) {
     assert(THREADS == 2);
     shared char *buf = upc_all_alloc(2*MAXBYTES, 1);

     for(int msg_size = 1; msg_size < MAXBYTES; msg_size *= 2) {
         if (MYTHREAD == 0) {
             upc_memput(&buf[1], "foo", 4);
             upc_memput(&buf[1], "bar", 4);
             upc_memput(&buf[1], "baz", 4);
             upc_memput(&buf[1], "foo", 4);
             upc_memput(&buf[1], "foo", 4);
             upc_memput(&buf[1], "foo", 4);
             upc_memput(&buf[1], "bar", 4);
             upc_memput(&buf[1], "baz", 4);
             upc_memput(&buf[1], "bar", 4);
             upc_memput(&buf[1], "baz", 4);
             upc_memput(&buf[1], "bar", 4);
             upc_memput(&buf[1], "baz", 4);

             double total_time = 0.0;
             char *msg = (char*) malloc(msg_size);
             struct timeval t_start, t_end;
             assert(upc_threadof(&buf[1]) == 1);
             for(int trial = 0; trial < NUMTRIALS; trial++) {
                 memset(msg, 'c', msg_size);
                 gettimeofday(&t_start,(struct timezone *)NULL);
                 {
                     upc_memput(&buf[1], msg, msg_size);
                 }
                 gettimeofday(&t_end,(struct timezone *)NULL);
                 total_time += (double) (t_end.tv_usec - t_start.tv_usec);
             }
             free(msg);

             printf("%d %f\n", msg_size, total_time/NUMTRIALS);
         }
         upc_barrier;
     }

     return 0;
}

