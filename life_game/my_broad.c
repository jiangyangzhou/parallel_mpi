void my_bcast(void* data, int count, MPI_Datatype datatype, int root,
              MPI_Comm communicator) {
  int id;
  MPI_Comm_rank(communicator, &id);
  int print_board;
  MPI_Comm_size(communicator, &p);

  if (id == root) {
    // If we are the root process, send our data to everyone
    int i;
    for(i=1; i<p && i<=2; i++)
        MPI_Send(data, count, datatype, i, 0, communicator);
  } else {
    // If we are a receiver process, receive the data from the root
    int new_id = id;
    if(id==0) new_id = root;
    int re_id = new_id / 2;
    if(re_id==0) re_id = root;
    MPI_Recv(data, count, datatype, re_id, 0, communicator,
             MPI_STATUS_IGNORE);
    if(id*2<p)
        MPI_Send(data, count, datatype, id*2, 0, communicator);
    if(id*2+1<p)
        MPI_Send(data, count, datatype, id*2+1, 0, communicator);
  }
}