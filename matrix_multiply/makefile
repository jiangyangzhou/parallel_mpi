all: manage_multiply col_multiply
manage_multiply: MY_MPI.o manage_multiply.o
	mpicc MY_MPI.o manage_multiply.o -o manage_multiply
col_multiply: MY_MPI.o col_multiply.o
	mpicc MY_MPI.o col_multiply.o -o col_multiply
MY_MPI.o: MY_MPI.h MY_MPI.c
	mpicc -c MY_MPI.c -o MY_MPI.o
col_multiply.o: col_multiply.c
	mpicc -c col_multiply.c -o col_multiply.o
manage_multiply.o: manage_multiply.c
	mpicc -c manage_multiply.c -o manage_multiply.o
clean:
	rm col_multiply  col_multiply.o MY_MPI.o  \
	   manage_multiply  manage_multiply.o 
