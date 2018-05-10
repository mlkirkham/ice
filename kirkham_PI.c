/*  @author: Michael Kirkham
    @version: 4/9/2018
    CS 435 Program 4: A Concurrent Computation of Pi

    
*/

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <math.h>
#include <stdbool.h>

#define THREAD_CREATION_FAILED -1
#define THREAD_JOIN_FAILED -2
#define RADIUS 0.5

void *worker_thread (void *arg);
void *control_thread (void *arg);

pthread_mutex_t *rng_lock;
pthread_mutex_t *var_lock;

pthread_barrier_t *worker_barrier;
pthread_barrier_t *control_barrier;

int iterations;
double delta;
int successes;
int total;
bool done = true;

int main(int argc, char *argv[]){
   int num_workers;
   srand(1);

   sscanf(argv[1], "%d", &num_workers);
   sscanf(argv[2], "%d", &iterations);
   sscanf(argv[3], "%lf", &delta);

   printf("Number of threads: %d\n", num_workers);
   printf("Number of iterations: %d\n", iterations);
   printf("Delta value: %lf\n", delta);

   rng_lock = (pthread_mutex_t *) malloc (sizeof (pthread_mutex_t));
   if (pthread_mutex_init (rng_lock, NULL)) {
      fprintf (stderr, "Error initializing rng_lock.\n");
      exit (-1);
   }

   var_lock = (pthread_mutex_t *) malloc (sizeof (pthread_mutex_t));
   if (pthread_mutex_init (var_lock, NULL)) {
      fprintf (stderr, "Error initializing var_lock.\n");
      exit (-1);
   }

   worker_barrier = (pthread_barrier_t *) malloc (sizeof (pthread_barrier_t));
   if (pthread_barrier_init (worker_barrier, NULL, num_workers + 1)) {
      fprintf (stderr, "Error initializing work_bar.\n");
      exit (-1);
   }

   control_barrier = (pthread_barrier_t *) malloc (sizeof (pthread_barrier_t));
   if (pthread_barrier_init (control_barrier, NULL, num_workers + 1)) {
      fprintf (stderr, "Error initializing control_bar.\n");
      exit (-1);
   }

   pthread_t *tid[num_workers];

   int i;
   for(i = 0; i < num_workers; i++){
        tid[i] = (pthread_t *) malloc(sizeof(pthread_t));

        if (pthread_create (tid[i],
			NULL,
			worker_thread,
			(void *) argv[i+2]) ) {
            fprintf (stderr, "Error creating thread %d.\n", i);
            exit (THREAD_CREATION_FAILED);
        }
    }

   pthread_t *con_thread = (pthread_t *) malloc(sizeof(pthread_t));

    if (pthread_create (con_thread,
			NULL,
			control_thread,
			(void *) NULL) ) {
            fprintf (stderr, "Error creating control thread.\n");
            exit (THREAD_CREATION_FAILED);
        }

   int j;
   for (j = 0; j < num_workers; j++) {

        if (pthread_join (*tid[j], (void *) NULL)) {
            fprintf (stderr, "Error joining with worker %d.\n", i);
            exit (THREAD_JOIN_FAILED);
        
        } else {
        
            //printf("threads joined\n");
            free(tid[j]);
        }
    }

    if (pthread_join (*con_thread, (void *) NULL)) {
            fprintf (stderr, "Error joining with control thread.\n");
            exit (THREAD_JOIN_FAILED);
        
        } else {
        
            //printf("threads joined\n");
            free(con_thread);
        }

   //printf("%lf\n", (double)successes);
   //printf("%lf\n", (double)total);
   //printf("pi is approximately: %lf\n", 4.0*((double)successes/(double)total));

   free(rng_lock);
   free(var_lock);
   free(worker_barrier);
   free(control_barrier);
   
   return 0;
}

void *worker_thread (void *arg){

   double point_x, point_y;
   double dist;
   while(done){
      double success;
      double fail;
      int k;
      for(k = 0; k < iterations; k++){
         pthread_mutex_lock(rng_lock);
         point_x = (double)rand() / (double)RAND_MAX;
         point_y = (double)rand() / (double)RAND_MAX;
         //printf("x:%lf y:%lf\n", point_x, point_y);
         pthread_mutex_unlock(rng_lock);
         dist = ((point_x - RADIUS) * (point_x - RADIUS)) + ((point_y - RADIUS) * (point_y - RADIUS));
         //printf("dist: %lf\n", dist);
         if(dist <= (RADIUS*RADIUS)){
            success++;
         }else{
            fail++;
         }
      }

      pthread_mutex_lock(var_lock);
      successes += success;
      total += (success + fail);
      pthread_mutex_unlock(var_lock);

      pthread_barrier_wait(worker_barrier);
      pthread_barrier_wait(control_barrier);
   }

   pthread_exit ((void *) NULL);
}

void *control_thread (void *arg){

   double exper = 0.0;
   double final = 0.0;
   while(done){
      pthread_barrier_wait(worker_barrier);
      pthread_mutex_lock(var_lock);
      final = 4.0*((double)successes/(double)total);
      pthread_mutex_unlock(var_lock);
      if(fabs(final - exper) <= delta){
         printf("PI is approximately: %lf\n", final);
         done = false;
      }else{
         exper = final;
      }
      pthread_barrier_wait(control_barrier);
   }

   pthread_exit ((void *) NULL);
}
