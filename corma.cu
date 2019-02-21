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

__global__ void calcmean(float *matrix, float *mean){

}

void calcmean(float *matrix, float *mean){
	int i,j;
	float sum;

	for(i = 0; i < H; i++){
		sum = 0.0;
		for(j = 0; j < W; j++){
			sum += matrix[i * W + j];
		}
		mean[i] = sum / (float)W;
	}
}

void calc_mm_std(float *matrix, float *mean, float *mm, float *std){
	int i,j;
	float sum, diff;

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

__global__ void pearson(float *mm, float *std){
	int i, sample1, sample2;
	float sum,r;


	for(sample1 = 0; sample1 < H-1; sample1++){
		for(sample2 = sample1+1; sample2 < H; sample2++){
			sum = 0.0;
			for(i = 0; i < W; i++){
				sum += mm[sample1 * W + i] * mm[sample2 * W + i];
			}
			r = sum / (std[sample1] * std[sample2]);
		}
		printf("%d\n", sample1);
	}
}

int main(int argc, char **argv){
	float *matrix, *minusmean, *mean, *std;

	cudaMallocManaged(&matrix, sizeof(float) * W * H);

	if(matrix == NULL){
		return(1);
	}

	cudaMallocManaged(&minusmean, sizeof(float) * W * H);
	if(minusmean == NULL){
		return(1);
	}

	cudaMallocManaged(&mean, sizeof(float) * H);
	cudaMallocManaged(&std, sizeof(float) * H);
	if(mean == NULL || std == NULL){
		return(1);
	}

	if(argc != 2){
		printf("Missing matrix file\n");
	}
	else{
		readmatrix(argv[1], matrix);
		calcmean(matrix, mean);
		//calc_mm_std(matrix, mean, minusmean, std);
		//pearson(minusmean, std);
	}

	cudaFree(mean);
	cudaFree(std);
	cudaFree(matrix);
	cudaFree(minusmean);
	return(EXIT_SUCCESS);
}
