#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <string.h>

typedef struct data_point {
	int t;
	float d;
} data_point;

data_point *data_set;

int main(int argc, char *argv[]) { 
	if(argc < 3){
		puts("Must provide input file AND number of data points in input_file!");
		return 1;
	}
	char* input_file = argv[1];
	int num_data_points = atoi(argv[2]);
	printf("the input file is: %s\nthe number of data points is: %d\n", input_file, num_data_points);

	data_set = malloc(num_data_points * sizeof(data_point));
	FILE *file = fopen(input_file, "r");
	if(file != NULL){
		char line[100];
		fgets(line, sizeof line, file);
		int i = 0;
		while(fgets(line, sizeof line, file) != NULL){
			if(line[strlen(line) - 1] == '\n'){
				line[strlen(line) - 1] = '\0';
			}
			char *tok;
			tok = strtok(line, ",");
			data_set[i].t = atoi(tok);
			tok = strtok(NULL, ",");
			data_set[i].d = strtod(tok, NULL);
			i++;
		}
	}

	for(int i = 0; i < num_data_points; i++){
		printf("%d,%f\n", data_set[i].t, data_set[i].d);
	}

	float y_int = 0;
	float slope = 0;
	printf("the y-intercept of the best L1 line is: %f\nthe slope of the best L1 line is: \t%f\n", y_int, slope);
	free(data_set);
	return 0;
}
