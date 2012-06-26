#include <stdlib.h>
#include <assert.h>

#include "upc.h"

#define MAXBYTES (1 << 24)
#define NUMTRIALS 100

int main(int argc, char **argv) {
     assert(THREADS == 2);
     shared char *buf = upc_all_alloc(2*MAXBYTES, 1);

     for(int msg_size = 1; msg_size < MAXBYTES; msg_size *= 2) {
         if (MYTHREAD == 0) {
             bupc_tick_t total_time = 0;
             char *msg = (char*) malloc(msg_size);
             for(int trial = 0; trial < NUMTRIALS; trial++) {
                 assert(upc_threadof(&buf[1]) == 1);
                 memset(msg, 'c', msg_size);
                 bupc_tick_t start = bupc_ticks_now();
                 {
                     upc_memput(&buf[1], msg, msg_size);
                 }
                 bupc_tick_t end = bupc_ticks_now();
                 total_time += (end-start);
             }
             free(msg);

             printf("%d %u\n",
                     msg_size, (int) bupc_ticks_to_ns(total_time)/NUMTRIALS);
         }
         upc_barrier;
     }

     return 0;
}

