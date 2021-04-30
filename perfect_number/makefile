all: perfect_number perfect_number2
perfect_number: MY_MPI.o perfect_number.o
	mpicc MY_MPI.o perfect_number.o -o perfect_number  -lm
perfect_number2: MY_MPI.o perfect_number2.o
	mpicc MY_MPI.o perfect_number2.o -o perfect_number2  -lm
MY_MPI.o: MY_MPI.h MY_MPI.c
	mpicc -c MY_MPI.c -o MY_MPI.o
perfect_number.o: perfect_number.c
	mpicc -c perfect_number.c -o perfect_number.o -lm
perfect_number2.o: perfect_number2.c
	mpicc -c perfect_number2.c -o perfect_number2.o -lm
clean:
	rm perfect_number.c  perfect_number.o MY_MPI.o  \
