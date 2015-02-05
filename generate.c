#include <stdio.h>
#include <stdlib.h>

/**
 * This program generates a very big matrix to test my new correlation
 * calculator
*/

int main(int argc, char **argv){
	int height = 20530;
	int width = 4000;
	int i,j;

	for(i = 0; i < height; i++){
		for(j = 0; j < width-1; j++){
			printf("%.3f\t",drand48());
		}
		printf("%.3f\n", drand48());
	}
}
