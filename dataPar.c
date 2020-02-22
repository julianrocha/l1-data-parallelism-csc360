#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <time.h>

#define MAX_LINE_LEN 100 // Assume no file line will ever be longer than 100 chars

typedef struct data_point {
	float d;
	int t;
} data_point;

typedef struct line {
	float y_int;
	float slope;
} line;

int num_data_points;
data_point *data_set;

void load_file_into_memory(char* input_file){
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


int main(int argc, char *argv[]) { 
	if(argc < 3){
		puts("Must provide input file AND number of data points in input_file!");
		return 1;
	}
	num_data_points = atoi(argv[2]);
	load_file_into_memory(argv[1]);

	// Time the execution
	clock_t begin = clock();

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

	// Time the execution
	clock_t end = clock();
	printf("input file is:\t\t\t\t%s\nnumber of data points is:\t\t%d\n", argv[1], num_data_points);
	printf("SAR of the best L1 line is:\t\t%f\ny-intercept of the best L1 line is:\t%f\nslope of the best L1 line is:\t\t%f\n", min_sum, best_line.y_int, best_line.slope);
	printf("time spent finding L1 best line was:\t%f\n", ((double)(end - begin) / CLOCKS_PER_SEC));
	
	free(data_set);
	return 0;
}
