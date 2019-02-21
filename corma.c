#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define W 4000
#define H 20530

void fillrandom(float *matrix){
	int i,j;

	for(i = 0; i < H; i++){
		for(j = 0; j < W; j++){
			matrix[i * W + j] = drand48();
		}
	}
}

/**
 * Read tab delimited matrix file
 **/
int readmatrix(const char *filename, float *matrix){
	FILE *input;
	char *line = NULL;
	size_t size;
	char *token;
	int count = 0;

	input = fopen(filename, "r");
	if(input == NULL){
		return(-1);
	}

	while(getline(&line, &size, input) != -1){
		token = strtok(line, "\t");
		matrix[count] = atof(token);
		count++;
		while( (token = strtok(NULL, "\t")) != NULL){
			matrix[count] = atof(token);
			count++;
		}
	}

	fclose(input);
	free(line);
	return(0);
}

/**
 * Calculate row mean
 */
void calcmean(float *matrix, float *mean){
	int i,j;
	float sum;
	#pragma omp parallel for
	for(i = 0; i < H; i++){
		sum = 0.0;
		for(j = 0; j < W; j++){
			sum += matrix[i * W + j];
		}
		mean[i] = sum / (float)W;
	}
}

/**
 * Calculate matrix - rowmean, and standard deviation for every row 
 */
void calc_mm_std(float *matrix, float *mean, float *mm, float *std){
	int i,j;
	float sum, diff;

	#pragma omp parallel for
	for(i = 0; i < H; i++){
		sum = 0.0;
		for(j = 0; j < W; j++){
			diff = matrix[i * W + j] - mean[i];
			mm[i * W + j] = diff;
			sum += diff * diff;
		}
		std[i] = sqrtf(sum);
	}
}

void pearson(float *mm, float *std){
	int i, sample1, sample2;
	float sum,r;
	#pragma omp parallel for
	for(sample1 = 0; sample1 < H-1; sample1++){
		for(sample2 = sample1+1; sample2 < H; sample2++){
			sum = 0.0;
			for(i = 0; i < W; i++){
				sum += mm[sample1 * W + i] * mm[sample2 * W + i];
			}
			r = sum / (std[sample1] * std[sample2]);
			printf("%d %d %f\n", sample1, sample2, r);
		}
	}
}

int main(int argc, char **argv){
	float *matrix, *minusmean, *mean, *std;
	int i;

	matrix = malloc(sizeof(float) * W * H);

	if(matrix == NULL){
		return(1);
	}

	minusmean = malloc(sizeof(float) * W * H);
	if(minusmean == NULL){
		return(1);
	}

	mean = malloc(sizeof(float) * H);
	std  = malloc(sizeof(float) * H);
	if(mean == NULL || std == NULL){
		return(1);
	}

	/*fillrandom(matrix);*TODO later change it to file loader*/
	if(argc != 2){
		printf("Missing matrix file\n");
	}
	else{
		readmatrix(argv[1], matrix);
		calcmean(matrix, mean);
		calc_mm_std(matrix, mean, minusmean, std);
		pearson(minusmean, std);
	}
	free(mean);
	free(std);
	free(matrix);
	free(minusmean);
	return(EXIT_SUCCESS);
}
