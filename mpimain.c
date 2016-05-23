#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>
const int BOARDSIZE=14;// Add this as input-argument to automize
int solutions=0;

typedef int bool;
#define true 1
#define false 0

bool threatens(const int row_1,const int row_2,const int col_1,const int col_2) {
    if ( row_1==row_2||abs(row_1-row_2)==abs(col_1-col_2) ){
        return true;
    }
    else{
        return false;
    }
}
bool is_checked(const int counter, int board[]) {
    // Could hybridize this w OpenMP
    int row_1, row_2;
    for (int col_1=0; col_1<counter; col_1++){
        row_1 = board[col_1];
        for (int col_2=col_1+1; col_2<counter; col_2++) {
            row_2 = board[col_2];
            if (threatens(row_1, row_2, col_1, col_2)) {return false;}
        }
    }
    return true;
}

void nq_recursion_master(const int counter, int board[]) {
    // Maybe add a "depth" layer to control how deep to parallelize?
    // Fix for when size is bigger than N.
    if (mySize>BOARDSIZE) printf("Not implemented yet! \n");

    int buf[2], i, activeWorkers;
    // Send one job to each process, ie. to mySize-1
    for (i=0; i<mySize-1; i++) {
        buf[0] = i; // The worker places 1st queen on i:th row
        buf[1] = 1;
        MPI_Send(buf, 2, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD);
    }
    activeWorkers = i;
    
    // Wait for processes to answer
    while (activeWorkers>0) {
        // receive solutions from workers
        MPI_Recv(buf, 2, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD);
        solutions += buf[0];
        
        // Once received we have a free worker
        if (i == BOARDSIZE) { // When we have sent out a worker to each
            activeWorkers = mySize; // Start a counter of active processes
            buf[1] = 0;
            MPI_Send(buf, 2, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD);
        }
        else if (i>BOARDSIZE) {
            activeWorkers--;
            buf[1] = 0; // termination message
            MPI_Send(buf, 2, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD);
        }
        else {
            buf[0] = i;
            buf[1] = 1;
            MPI_Send(buf, 2, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD);
            i++;
        }
    }
    
}

void nq_recursion_worker() {
    // Worker mpi function
    // Receives a position on the board. Slaves away to find solutions.
    int buf, row, partialSolutions;
    
    MPI_Recv(buf,100,MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG , MPI_COMM_WORLD)
    if (buf[1]==0) {
        printf("Process %d of %d terminated", myRank, mySize);
        return
    }
    else{
        row = buf[0];
        int chessboard[BOARDSIZE];
        chessboard[1] = row;
        partialSolutions = worker_recursion(0,1, chessboard);
        buf[0] = partialSolutions;
        
    }
}

// Make this functional...
// return int instead of void n globals. FIX!!
int worker_recursion(int solutions, int counter, int board[]){
    if (counter >= BOARDSIZE) {
        if (is_checked()) {
            // Need to rework how to use the solutions
            solutions++;
        }
        return;
    }
    else if (!ischecked(counter,board)){
        return;
    }
    
    else{
        for(int row=0; row<BOARDSIZE; row++){
            board[counter] = row;
            worker_recursion(solutions, counter+1, board);
        }
    }
}

int main(int argc, char *argv[]) {
    int chessboard[BOARDSIZE];
    clock_t time;
    time = clock();
    
    int myRank, mySize;
    MPI_Status status;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &myRank);
    MPI_Comm_size(MPI_COMM_WORLD, &mySize);
    // dynamic stuff? MPI_Comm_spawn()
    // Maybe initialize inside of master instead...
    if (myrank==0){
        nq_recursion_master(0,chessboard);
    }
    else {
        nq_recursion_worker();
    }
    MPI_Finalize();
    
    time = clock() - time;
    double elapsed_time = ((double)(time))/CLOCKS_PER_SEC;
    printf("Solutions: %i in %f seconds", solutions, elapsed_time);
}


