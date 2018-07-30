#define _GNU_SOURCE

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include <sched.h>
#include <pthread.h>

#define log(x) (x)

/* This variable is module-global to be visible inside plog */
static bool is_verbose = false;

/* This is better done with macro, as it allows to easily
 * concatenate format strings and pass variable arguments
 * to a foreign function. But here's a different approach
 */
#ifdef DEBUG
static inline int plog(const char *fmt, ...)
{
	if (!is_verbose)
		return 0;
	va_list arg;
	va_start(arg, fmt);	/* second arg is last known param */
	int ret = fprintf(stderr, ">>LOG: ");
	/* printf is not guaranteed to use this scheme */
	ret += vfprintf(stderr, fmt, arg);	/* uses varargs, see man */
	va_end(arg);	/* Each va_start must be smwhere matched with va_end */
	return ret;
}
#else
/* This stuff allows us to easily strip all unneeded stuff at compile-time */
#define plog(...)
#endif

static const char help_str[] = {
  "[-h] [-v] -t NUM_THREADS -n ARRAY_SIZE\n"
  "Evaluate the time required to do a simple threaded map-reduce operation"
  "on randomly generated array of doubles"
};

static cpu_set_t all_cores(void)
{
	cpu_set_t cpuset;
	pthread_t this = pthread_self();
	pthread_getaffinity_np(this, sizeof(cpu_set_t), &cpuset);
	int numcores = sysconf(_SC_NPROCESSORS_ONLN);
	for (int id = 0; id < numcores; id++) {
		plog("Core %d was %d\n", id, CPU_ISSET(id, &cpuset));
		CPU_SET(id, &cpuset);
	}
	pthread_setaffinity_np(this, sizeof(cpu_set_t), &cpuset);
	return cpuset;
}

/**
 * timespec_diff() - returns time difference in milliseconds for two timespecs.
 * @stop:	time after event.
 * @start:	time before event.
 *
 * Uses difftime() for time_t seconds calcultation.
 */
static double timespec_diff(struct timespec *stop, struct timespec *start)
{
	double diff = difftime(stop->tv_sec, start->tv_sec);
	diff *= 1000;
	diff += (stop->tv_nsec - start->tv_nsec) / 1e6;
	return diff;
}

/* Error-related stuff */
enum _errors {
	E_OK = 0,
	E_FOPEN,
	E_FREAD,
	E_ALLOC,
	E_CPUSET
};

static const char * const _error_msg[] = {
	[E_OK] = "Success",
	[E_FOPEN] = "Failed to open '/dev/random'",
	[E_FREAD] = "Failed to read from '/dev/random'",
	[E_ALLOC] = "Failed to allocate memory",
	[E_CPUSET] = "Could not link thread to all CPU cores"
};


struct thread_data {
	struct timespec start_time, end_time;
	double *arrptr;		/* Points to start of array slice */
	long long num_items;	/* Elements in slice */
	double *resptr;		/* Pointer to result(shared) */
	pthread_mutex_t *lock;	/* Lock for result */
	
};


/* This function runs in each thread */
void *threadfunc(void *args)
{
	/* Struct is passed via args at pthread_create so its type is known */
	struct thread_data *data = args;

	/* We check the time spent in each thread and the global time */
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &data->start_time);

	double r = 0.;
	for (int i = 0; i < data->num_items; i++)
		r += log(data->arrptr[i]);      /* arrptr is a slice of original array */

	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &data->end_time);
	pthread_mutex_lock(data->lock); /* wait till acquire */
	/* Now we own a lock */
	*data->resptr += r;	/* manipulate the shared data */
	pthread_mutex_unlock(data->lock);      /* release lock for the others */

	return 0;
}


int main(int argc, char *argv[])
{
	int num_threads = 0;
	long long arr_size = 0;

	plog("Arguments given:\n");
	for (int i = 0; i < argc; i++)
		plog("\t[%d]  '%s'\n", i, argv[i]);

	/* For moar on getopt: http://bit.ly/getopt_rus */
	opterr = 0; /* No getopt def err out -- we do it manually */
	int argopt;
	/* "hvt:n:" means h,v,t,n switches, t & n require argument */
	while ((argopt = getopt(argc, argv, "hvt:n:")) != -1) {
		switch(argopt) {
		case 'h':
			printf("Usage: %s %s\n", argv[0], help_str);
			exit(0);
		case 'v':
			is_verbose = true;
			break;
		case 't':
			num_threads = atoi(optarg);
			break;
		case 'n':
			arr_size = atoll(optarg);
			break;
		default:
			fprintf(stderr, "Unknown option '%s'\n", optarg);
			exit(EXIT_FAILURE);
		}
	}
	/* There's some redundancy in arguments parsing */
	plog("Parsed arguments:\n"
             "\tis_verbose\t= %s\n\tnum_threads\t= %d\n\tarr_size\t= %lld\n",
	     (is_verbose ? "true" : "false"), num_threads, arr_size);
	if (argc <= 1) {
		printf("Usage: %s %s\n", argv[0], help_str);
		exit(0);
	}
	if (num_threads <= 0 || arr_size <= 0) {
		fprintf(stderr, "NUM_THREADS and ARRAY_SIZE aren't ints > 0\n");
		exit(EXIT_FAILURE);
	}
	if (arr_size % num_threads) {
		fprintf(stderr, "NUM_THREADS is not a divisor of ARRAY_SIZE\n");
		exit(EXIT_FAILURE);
	}

	pthread_t threads[num_threads];
	struct thread_data th_dat[num_threads];

	enum _errors errlvl = E_OK;
	/* Fill array with randoms */
	FILE *fp_rand = fopen("/dev/random", "rb");
	if (NULL == fp_rand) {
		errlvl = E_FOPEN;	/* Notice how error handling is done */
		goto exc_fopen;
	}
	unsigned int seed;
	fread(&seed, sizeof(seed), 1, fp_rand);
	if (ferror(fp_rand)) {
		errlvl = E_FREAD;
		goto exc_fread;
	}
	srand(seed);
	plog("Random seed set to: 0x%X\n", seed);

	double *array = malloc(arr_size * sizeof *array);
	for (long long i = 0; i < arr_size; i++)
		array[i] = (2. / RAND_MAX) * rand();

	/* Configure thread flags */
	/* Moar: http://maxim.int.ru/bookshelf/PthreadsProgram/htm/r_37.html */
	pthread_attr_t thread_attrs;
	pthread_attr_init(&thread_attrs); /* fill with default attributes */
	
	// Set scheduler to FIFO for spawned threads
	// This allows for less strict implementation requirements
	pthread_attr_setschedpolicy(&thread_attrs, SCHED_FIFO);
	// Set maximum priority for main and other threads
	// As long as on Linux they compete for overall system resources
	pthread_setschedprio(pthread_self(), sched_get_priority_max(SCHED_FIFO));
	struct sched_param param;
	pthread_attr_getschedparam(&thread_attrs, &param);
	param.sched_priority = sched_get_priority_max(SCHED_FIFO);
	pthread_attr_setschedparam(&thread_attrs, &param);
	
	
	// See what happens if uncomment this
	cpu_set_t cpuset = all_cores();
	int ret = pthread_attr_setaffinity_np(&thread_attrs, sizeof(cpu_set_t), &cpuset);
	if (ret < 0) {
		errlvl = E_CPUSET;
		goto exc_aff;
	}


	/* Now spawn threads */
	pthread_mutex_t sharedlock;
	pthread_mutex_init(&sharedlock, NULL);

	double result = 0.;
	struct timespec time_now, time_after;
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time_now);
	for (int i = 0; i < num_threads; i++) {
		long long slice = arr_size / num_threads;
		th_dat[i].arrptr = &(array[i * slice]);	/* Points to start of array slice */
		th_dat[i].num_items = slice;		/* Elements in slice */
		th_dat[i].resptr = &result;		/* Pointer to result(shared) */
		th_dat[i].lock = &sharedlock;		/* Lock for result */
		pthread_create(&threads[i], &thread_attrs,
                               &threadfunc, &th_dat[i]);
	}
	plog("Threads spawned. Performing join\n");
	for (int i = 0; i < num_threads; i++)
		pthread_join(threads[i], NULL);

	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time_after);

	/* Calculate the resulting times */
	double took_global = timespec_diff(&time_after, &time_now);
	double took_avg = 0.;
	for (int i = 0; i < num_threads; i++) {
		took_avg += timespec_diff(&(th_dat[i].end_time), 
					  &(th_dat[i].start_time));	
	}
	took_avg /= num_threads;

	printf("Numbers: %lld\nThreads: %d\nValue (result): %g\n"
	       "Average thread time, ms: %g\nCalculation took, ms: %g\n", 
	       arr_size, num_threads, result, took_avg, took_global);
	
	pthread_mutex_destroy(&sharedlock);
	free(array);

	/* This stuff should appear in opposite direction */
	exc_aff:
		pthread_attr_destroy(&thread_attrs);
	exc_fread:
		fclose(fp_rand);
	exc_fopen:

	if (E_OK == errlvl)
		return 0;

	/* Error handler itself */
	fprintf(stderr, "Error: %s\n", _error_msg[errlvl]);
	exit(EXIT_FAILURE);
}
