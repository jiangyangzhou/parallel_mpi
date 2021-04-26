all: perfect_number 
perfect_number: MY_MPI.o perfect_number.o
	mpicc MY_MPI.o perfect_number.o -o perfect_number  -lm
MY_MPI.o: MY_MPI.h MY_MPI.c
	mpicc -c MY_MPI.c -o MY_MPI.o
perfect_number.o: perfect_number.c
	mpicc -c perfect_number.c -o perfect_number.o -lm
clean:
	rm perfect_number.c  perfect_number.o MY_MPI.o  \
