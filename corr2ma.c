#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

/**
 * Determine the dimension of the matrix
 * **/
int getdim(const char *filename, unsigned int *width, unsigned int *height){
	FILE *input;
	char *line = NULL;
	size_t size;
	char *token;
	unsigned int row = 0, prevcol = 0, col = 0;
	int retcode = 0;

	input = fopen(filename, "r");
	if(input == NULL){
		return(-1);
	}

	while(getline(&line, &size, input) != -1){
		token = strtok(line, "\t");
		col = 0;
		while( (token = strtok(NULL, "\t")) != NULL){
			col++;
		}
		if(col == 0 || (prevcol != col && prevcol != 0)){
			retcode = -2;
			col = 0;
			row = 0;
			goto end;
		}
		prevcol = col;
		row++;
	}

end:
	*width = col;
	*height = row;
	fclose(input);
	free(line);
	return(retcode);
}

/**
 * Read tab delimited matrix file
 **/
int readmatrix(const char *filename, float *matrix, char **id){
	FILE *input;
	char *line = NULL;
	size_t size;
	char *token;
	unsigned int count = 0;
	unsigned int idcount = 0;
	unsigned int n;

	input = fopen(filename, "r");
	if(input == NULL){
		return(-1);
	}

	while(getline(&line, &size, input) != -1){
		token = strtok(line, "\t");
		n = strlen(token);
		id[idcount] = malloc(sizeof(char) * n + 1);
		strncpy(id[idcount], token, n);
		idcount++;
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
void calcmean(float *matrix, float *mean, unsigned int W, unsigned int H){
	unsigned int i,j;
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
void calc_mm_std(float *matrix, float *mean, float *mm, float *std, unsigned int W, unsigned int H){
	unsigned int i,j;
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

void pearson(float *mm1, float *mm2, float *std1, float *std2, unsigned int W, unsigned int H1, unsigned int H2, char **id1, char **id2){
	unsigned int i, sample1, sample2;
	float sum,r;
	#pragma omp parallel for
	for(sample1 = 0; sample1 < H1; sample1++){
		for(sample2 = 0; sample2 < H2; sample2++){
			sum = 0.0;
			for(i = 0; i < W; i++){
				sum += mm1[sample1 * W + i] * mm2[sample2 * W + i];
			}
			r = sum / (std1[sample1] * std2[sample2]);
			printf("%s\t%s\t%d\t%d\t%f\n", id1[sample1], id2[sample2], sample1, sample2, r);
		}
	}
}

void twomatrixcorr(char *file1, unsigned int h1, char *file2, unsigned int h2, unsigned int width){
	float *matrix1 = NULL;
	float *matrix2 = NULL;
	float *mm1 = NULL;
	float *mm2 = NULL;
	float *mean1 = NULL;
	float *mean2 = NULL;
	float *std1 = NULL;
	float *std2 = NULL;
	char **id1;
	char **id2;

	id1 = malloc(sizeof(char *) * h1);
	if(id1 == NULL){
		fprintf(stderr, "Cannot alloc memory\n");
		return;
	}
	matrix1 = malloc(sizeof(float) * width * h1);
	if(matrix1 == NULL){
		fprintf(stderr, "Cannot alloc memory\n");
		return;
	}
	readmatrix(file1, matrix1, id1);
	fprintf(stderr, "Matrix 1 readed\n");

	id2 = malloc(sizeof(char *) * h2);
	if(id2 == NULL){
		fprintf(stderr, "Cannot alloc memory\n");
		return;
	}
	matrix2 = malloc(sizeof(float) * width * h2);
	if(matrix2 == NULL){
		fprintf(stderr, "Cannot alloc memory\n");
		return;
	}
	readmatrix(file2, matrix2, id2);
	fprintf(stderr, "Matrix 2 readed\n");

	mean1 = malloc(sizeof(float) * h1);
	if(mean1 == NULL){
		fprintf(stderr, "Cannot alloc memory\n");
		return;
	}
	calcmean(matrix1, mean1, width, h1);

	mm1 = malloc(sizeof(float) * width * h1);
	if(mm1 == NULL){
		fprintf(stderr, "Cannot alloc memory\n");
		return;
	}
	std1 = malloc(sizeof(float) * h1);
	if(std1 == NULL){
		fprintf(stderr, "Cannot alloc memory\n");
		return;
	}
	calc_mm_std(matrix1, mean1, mm1, std1, width, h1);

	mean2 = malloc(sizeof(float) * h2);
	if(mean2 == NULL){
		fprintf(stderr, "Cannot alloc memory\n");
		return;
	}
	calcmean(matrix2, mean2, width, h2);
	mm2 = malloc(sizeof(float) * width * h2);
	if(mm2 == NULL){
		fprintf(stderr, "Cannot alloc memory\n");
		return;
	}
	std2 = malloc(sizeof(float) * h2);
	if(std2 == NULL){
		fprintf(stderr, "Cannot alloc memory\n");
		return;
	}
	calc_mm_std(matrix2, mean2, mm2, std2, width, h2);

	pearson(mm1, mm2, std1, std2, width, h1, h2, id1, id2);

	free(matrix1);
	free(matrix2);
	free(mean1);
	free(mean2);
	free(std1);
	free(std2);
	free(mm1);
	free(mm2);
}

int main(int argc, char **argv){
	int materror1, materror2;
	unsigned int w1 = 0;
	unsigned int w2 = 0;
	unsigned int h1 = 0;
	unsigned int h2 = 0;

	if(argc != 3){
		fprintf(stderr, "Usage: corr2ma matrix1 matrix2\n");
	}
	else{
		materror1 = getdim(argv[1], &w1, &h1);
		if(materror1 != 0){
			fprintf(stderr, "First matrix has no equal cols\n");
		}
		materror2 = getdim(argv[2], &w2, &h2);
		if(materror2 != 0){
			fprintf(stderr, "Second matrix has no equal cols\n");
		}
		if(w1 != w2){
			fprintf(stderr, "The two matrix not compatible\n");
		}

		if(materror1 == 0 && materror2 == 0 && w1 == w2){
			twomatrixcorr(argv[1], h1, argv[2], h2, w1);
		}
	}
/*
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

	*fillrandom(matrix);*TODO later change it to file loader*
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
	free(minusmean);*/
	return(EXIT_SUCCESS);
}
