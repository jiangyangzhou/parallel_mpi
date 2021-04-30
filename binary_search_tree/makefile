all: parallel_bstree bstree
bstree: binary_search_tree.c
	gcc binary_search_tree.c -o bstree
parallel_bstree:  parallel_bstree.o
	mpicc parallel_bstree.o -o parallel_bstree  -lm
parallel_bstree.o:   parallel_bstree.c
	mpicc -c parallel_bstree.c -o parallel_bstree.o
clean:
	rm parallel_bstree bstree parallel_bstree.o