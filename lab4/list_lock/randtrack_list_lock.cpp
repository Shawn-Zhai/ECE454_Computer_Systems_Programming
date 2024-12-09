
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "defs.h"
#include "hash.h"

#define SAMPLES_TO_COLLECT   10000000
#define RAND_NUM_UPPER_BOUND   100000
#define NUM_SEED_STREAMS            4

/* 
 * ECE454 Students: 
 * Please fill in the following team struct 
 */
team_t team = {
    "Aminuos",                  /* Team name */

    "Shawn Zhai",                    /* Member full name */
    "1006979389",                 /* Member student number */
    "shawn.zhai@mail.utoronto.ca",                 /* Member email address */
};

unsigned num_threads;
unsigned samples_to_skip;

class sample;

class sample {
  unsigned my_key;
 public:
  sample *next;
  unsigned count;

  sample(unsigned the_key){my_key = the_key; count = 0;};
  unsigned key(){return my_key;}
  void print(FILE *f){printf("%d %d\n",my_key,count);}
};

// This instantiates an empty hash table
// it is a C++ template, which means we define the types for
// the element and key value here: element is "class sample" and
// key value is "unsigned".  
hash<sample,unsigned> h;

pthread_mutex_t list_locks[16384];

void* process_stream(void* arg) {
    int thread_id = *(int*)arg;

    for (int i = thread_id; i < NUM_SEED_STREAMS; i += num_threads) {
        int rnum = i; 
        unsigned key;
        sample *s;

        // collect samples for this stream
        for (int j = 0; j < SAMPLES_TO_COLLECT; j++) {

            // kkip a number of samples
            for (int k = 0; k < samples_to_skip; k++) {
                rnum = rand_r((unsigned int*)&rnum);
            }

            // force the sample to be within the range
            key = rnum % RAND_NUM_UPPER_BOUND;

            // lock the mutex for the specific hash bucket
            int bucket_index = key % 16384;
            pthread_mutex_lock(&list_locks[bucket_index]);

            s = h.lookup(key);
            if (!s) {
                s = new sample(key);
                h.insert(s);
            }
            s->count++;

            pthread_mutex_unlock(&list_locks[bucket_index]);
        }
    }
    return NULL;
}

int  
main (int argc, char* argv[]){
  // int i,j,k;
  // int rnum;
  // unsigned key;
  // sample *s;

  // Print out team information
  printf( "Team Name: %s\n", team.team );
  printf( "\n" );
  printf( "Student 1 Name: %s\n", team.name1 );
  printf( "Student 1 Student Number: %s\n", team.number1 );
  printf( "Student 1 Email: %s\n", team.email1 );
  printf( "\n" );

  // Parse program arguments
  if (argc != 3){
    printf("Usage: %s <num_threads> <samples_to_skip>\n", argv[0]);
    exit(1);  
  }
  sscanf(argv[1], " %d", &num_threads); // not used in this single-threaded version
  sscanf(argv[2], " %d", &samples_to_skip);

  // initialize a 16K-entry (2**14) hash of empty lists
  h.setup(14);

  for (int i = 0; i < 16384; i++) {
      pthread_mutex_init(&list_locks[i], NULL);
  }

  pthread_t threads[num_threads];
  int thread_ids[num_threads];

  for (int i = 0; i < num_threads; i++) {
      thread_ids[i] = i;
      pthread_create(&threads[i], NULL, process_stream, &thread_ids[i]);
  }

  for (int i = 0; i < num_threads; i++) {
      pthread_join(threads[i], NULL);
  }

  for (int i = 0; i < 16384; i++) {
      pthread_mutex_destroy(&list_locks[i]);
  }

  // print a list of the frequency of all samples
  h.print();
}
