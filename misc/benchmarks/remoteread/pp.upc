#include <stdlib.h>
#include <assert.h>

#include "upc.h"

#define MAXBYTES (1 << 24)
#define NUMTRIALS 1000

// from http://www.strchr.com/standard_deviation_in_one_pass
double stddev(double a[], int n) {
    if(n == 0)
        return 0.0;
    double sum = 0;
    for(int i = 0; i < n; ++i)
        sum += a[i];
    double mean = sum / n;
    double sq_diff_sum = 0;
    for(int i = 0; i < n; ++i) {
        double diff = a[i] - mean;
        sq_diff_sum += diff * diff;
    }
    double variance = sq_diff_sum / n;
    return sqrt(variance);
}

double average(double a[], int n) {
    if (n <= 0)
        return 0;
    double sum = 0.0;
    for(int i = 0; i < n; i++)
        sum += a[i];
    return sum / (double) n;
}

int main(int argc, char **argv) {
     assert(THREADS == 2);
     shared char *buf = upc_all_alloc(2*MAXBYTES, 1);
     upc_memset(&buf[MYTHREAD], 'c', MAXBYTES);

     double times[NUMTRIALS];

     if (MYTHREAD == 0)
         printf("#size\tavg_time_us\tstddev_times\tavg_rates_gbs\tstdev_rates\n");

     for(int msg_size = 1; msg_size < MAXBYTES; msg_size *= 2) {
         if (MYTHREAD == 0) {
             upc_memput(&buf[1], "foo", 4);
             upc_memput(&buf[1], "bar", 4);
             upc_memput(&buf[1], "baz", 4);

             char *msg = (char*) malloc(msg_size);
             struct timeval t_start, t_end;
             assert(upc_threadof(&buf[1]) == 1);
             for(int trial = 0; trial < NUMTRIALS; trial++) {
                 gettimeofday(&t_start,(struct timezone *)NULL);
                 {
                     upc_memget(msg, &buf[1], msg_size);
                 }
                 gettimeofday(&t_end,(struct timezone *)NULL);
                 times[trial] = 1e6 * (double) (t_end.tv_sec  - t_start.tv_sec) +
                                (double) (t_end.tv_usec - t_start.tv_usec);
             }
             free(msg);

             // avg time
             double total_time = 0.0;
             double avg_time_us = average(times, NUMTRIALS);
             double time_dev_us = stddev(times, NUMTRIALS);

             // avg rate
             double rates[NUMTRIALS];
             for(int trial = 0; trial < NUMTRIALS; trial++)
                 rates[trial] = msg_size * 8e-9 / (times[trial]*1e-6);
             double avg_rate = average(rates, NUMTRIALS);
             double rate_dev = stddev(rates, NUMTRIALS);

             printf("%d %f %f %f %f\n", msg_size, avg_time_us, time_dev_us, 
                                                  avg_rate, rate_dev);
         }
         upc_barrier;
     }

     return 0;
}

