#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <time.h>

#define MAX_LINE_LEN 100 // Assume no file line will ever be longer than 100 chars
#define MAX_THREADS 4

typedef struct data_point {
	float d;
	int t;
} data_point;

typedef struct line {
	float y_int;
	float slope;
} line;

char* input_file;
int num_data_points;
data_point *data_set;

pthread_t threads[MAX_THREADS];
int thread_numbers[MAX_THREADS];
float min_sums[MAX_THREADS];
line best_lines[MAX_THREADS];

void load_file_into_memory(){
	// malloc an in-memory dataset for the number of data points passed as arg to main
	data_set = malloc(num_data_points * sizeof(data_point));
	FILE *file = fopen(input_file, "r");
	if(file != NULL){
		char line[MAX_LINE_LEN];
		// Discard first line containing csv headers
		fgets(line, sizeof line, file);
		int i = 0;
		while(fgets(line, sizeof line, file) != NULL && i < num_data_points){
			// Strip '\n' from line
			if(line[strlen(line) - 1] == '\n'){
				line[strlen(line) - 1] = '\0';
			}

			// Discard first column (Assume data is ascending order, evenly spaced)
			char *tok;
			tok = strtok(line, ",");
			data_set[i].t = i + 1;

			// Cast second column to float
			tok = strtok(NULL, ",");
			data_set[i].d = strtod(tok, NULL);
			i++;
		}
	} else {
		perror("Problem opening file!");
		exit(1);
	}
}

line compute_line(data_point pt1, data_point pt2){
	line l;
	l.slope = (pt1.d - pt2.d) / (pt1.t - pt2.t);
	l.y_int = pt1.d - (pt1.t * l.slope);
	// printf("For points (%f,%d) and (%f,%d) the slope is %f and y_int is %f\n", pt1.d, pt1.t, pt2.d, pt2.t, l.slope, l.y_int);
	return l;
}

float compute_distance_from_line(line l, data_point pt){
	return fabs(pt.d - (l.y_int + (l.slope * pt.t)));
}

float compute_sum(data_point pt1, data_point pt2){
	line l = compute_line(pt1, pt2);
	float total_distance_from_line = 0;
	for(int pt3 = 0; pt3 < num_data_points; pt3++){
		total_distance_from_line += compute_distance_from_line(l, data_set[pt3]);
	}
	// printf("Sum of abs residuals is %f\n", total_distance_from_line);
	return total_distance_from_line;
}

void* l1_worker(void* param){
	int* temp = param;
    int thread = *temp;

	float min_sum = compute_sum(data_set[0], data_set[1]);
	line best_line = compute_line(data_set[0], data_set[1]);
	for(int pt1 = thread; pt1 < num_data_points; pt1+=4){
		for(int pt2 = pt1 + 1; pt2 < num_data_points; pt2++){
			float sum = compute_sum(data_set[pt1], data_set[pt2]);
			if(sum < min_sum){
				min_sum = sum;
				best_line = compute_line(data_set[pt1], data_set[pt2]);
			}
		}
	}
	min_sums[thread] = min_sum;
	best_lines[thread] = best_line;
	return NULL;
}

// How many threads to use: https://askubuntu.com/questions/668538/cores-vs-threads-how-many-threads-should-i-run-on-this-machine
// Linux Machine: 4 sockets x 1 cores/socket x 1 threads/core = 4 threads
// Macbook:
void multi_threaded_l1(){
	int i;

	for(i = 0; i < MAX_THREADS; i++){
        thread_numbers[i] = i;
    }

    clock_t begin = clock(); // Time the execution
    for(i = 0; i < MAX_THREADS; i++){
        pthread_create(&threads[i], NULL, l1_worker, &thread_numbers[i]);
    }

    for(i = 0; i < MAX_THREADS; i++){
        pthread_join(threads[i], NULL);
    }

    float min_sum = min_sums[0];
    line best_line = best_lines[0];
    for(i = 1; i < MAX_THREADS; i++){
    	if(min_sums[i] < min_sum){
    		min_sum = min_sums[i];
    		best_line = best_lines[i];
    	}
    }

	clock_t end = clock(); // Time the execution
	printf("input file is:\t\t\t\t%s\nnumber of data points is:\t\t%d\n", input_file, num_data_points);
	printf("SAR of the best L1 line is:\t\t%f\ny-intercept of the best L1 line is:\t%f\nslope of the best L1 line is:\t\t%f\n", min_sum, best_line.y_int, best_line.slope);
	printf("time spent finding L1 best line was:\t%f\n", ((double)(end - begin) / CLOCKS_PER_SEC));
}

void single_threaded_l1(){
	clock_t begin = clock(); // Time the execution

	float min_sum = compute_sum(data_set[0], data_set[1]);
	line best_line = compute_line(data_set[0], data_set[1]);
	for(int pt1 = 0; pt1 < num_data_points; pt1++){
		for(int pt2 = pt1 + 1; pt2 < num_data_points; pt2++){
			float sum = compute_sum(data_set[pt1], data_set[pt2]);
			if(sum < min_sum){
				min_sum = sum;
				best_line = compute_line(data_set[pt1], data_set[pt2]);
			}
		}
	}
	clock_t end = clock(); // Time the execution
	printf("input file is:\t\t\t\t%s\nnumber of data points is:\t\t%d\n", input_file, num_data_points);
	printf("SAR of the best L1 line is:\t\t%f\ny-intercept of the best L1 line is:\t%f\nslope of the best L1 line is:\t\t%f\n", min_sum, best_line.y_int, best_line.slope);
	printf("time spent finding L1 best line was:\t%f\n", ((double)(end - begin) / CLOCKS_PER_SEC));
}


int main(int argc, char *argv[]) { 
	if(argc < 3){
		puts("Must provide input file AND number of data points in input_file!");
		return 1;
	}
	num_data_points = atoi(argv[2]);
	input_file = argv[1];
	load_file_into_memory();

	printf("MULTI THREADED IMPLEMENTATION RESULTS:\n");
	multi_threaded_l1();
	printf("END OF MULTI THREADED IMPLEMENTATION\n");


	printf("SINGLE THREADED IMPLEMENTATION RESULTS:\n");
	single_threaded_l1();
	printf("END OF SINGLE THREADED IMPLEMENTATION\n");
	
	free(data_set);
	return 0;
}
