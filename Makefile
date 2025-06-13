all: cpu gpu generate

cpu:
	gcc corma.c -lm -Wall -W -pedantic -fopenmp -o corma
gpu:
	nvcc corma.cu -o corma.gpu
generate:
	gcc generate.c -o generate
two:
	gcc corr2ma.c -lm -Wall -W -pedantic -fopenmp -o corr2ma
