#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
const int BOARDSIZE=10;// Add this as input-argument to automize

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

// Could hybridize w. OpenMP
bool is_checked(const int counter, int board[]) {
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

// Used to return the partial solutions from a sub-tree of the complete
// search-tree. Returns the solutions recursively.
int worker_recursion(int counter, int board[]){
    int workerSols = 0;
    if (counter >= BOARDSIZE) {
        if (is_checked(counter,board)) {
            workerSols++;
            return workerSols;
        }
    }
    else if (is_checked(counter,board)){
        for(int row=0; row<BOARDSIZE; row++){
            board[counter] = row;
            workerSols += worker_recursion(counter+1, board);
        }
    }
    return workerSols;
}

// Master mpi function. Called by only one process.
// Deals out work to worker processes.
// Right now, N < boardsize, and it deals out N tasks in total, where each task is
// the board with the first queen on a row.
int nq_recursion_master(int myRank, int mySize) {
    if (mySize>BOARDSIZE) {     // Fix when N>boardsize
        printf("Not implemented yet! \n");
        return 0;
    }
    
    int buf[2], row, activeWorkers, totalSolutions;
    totalSolutions = 0;

    // Send one job to each process in the commgroup.
    // Keep i as tracker of where the queen has been placed.
    for (row=0; row<mySize-1; row++) {
        buf[0] = row;
        buf[1] = 1;
        MPI_Send(&buf, 2, MPI_INT, i, 0, MPI_COMM_WORLD);
        printf("Master dealing out job %d of %d \n ", row+1, BOARDSIZE);
    }
    activeWorkers = i;

    // Controls communication to the workers while they are active.
    // When a solution is received: either send more work to, or kill, the process.
    while (activeWorkers>0) {
        int sender;
        MPI_Status status;

        MPI_Recv(&buf, 2, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
    
        sender = status.MPI_SOURCE;
        // tag=status.MPI_Probe; // Is this needed? just keep to try it out
        
        totalSolutions += buf[0];
        printf("Worker no. %d found %d solutions \n ", sender, buf[0]);

        // If received solution was to the final task: initialize a count down, and start killing
        // processes. Otherwise keep sending out work.
        if (i == BOARDSIZE) {
            activeWorkers = mySize-2; // mySize - (thisprocess+masterprocess)
            buf[1] = 0;
            MPI_Send(&buf, 2, MPI_INT, sender, 0, MPI_COMM_WORLD);
            row++;
        }
        else if (i>BOARDSIZE) {
            buf[1] = 0;
            MPI_Send(&buf, 2, MPI_INT, sender, 0, MPI_COMM_WORLD);
            activeWorkers--;
        }
        else {
            buf[0] = row;
            buf[1] = 1;
            MPI_Send(&buf, 2, MPI_INT, sender, 0, MPI_COMM_WORLD);
            printf("Master dealing out job %d of %d \n", row+1, BOARDSIZE);
            row++;
        }
    }
    printf("Master process terminated \n");
    return totalSolutions;
}

// Worker mpi function.
// Receives a position on the board and starts worker_recursions on them.
// Terminates only when they receive a termination message in buf[1].
void nq_recursion_worker(int myRank,int mySize) {
    int buf[2], row, partialSolutions;
    MPI_Status status;

    // Receive initial work
    MPI_Recv(&buf, 2, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
    if (buf[1]==0) {
        printf("Process %d of %d terminated before any work was done! \n", myRank, mySize);
        return;
    }
    
    // Receive more work until termination message received
    while (buf[1]==1) {
        row = buf[0];
        int chessboard[BOARDSIZE];
        chessboard[1] = row;
        partialSolutions = worker_recursion(1, chessboard);
        buf[0] = partialSolutions;
        MPI_Send(&buf, 2, MPI_INT, 0, 0, MPI_COMM_WORLD);
        MPI_Recv(&buf, 2, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
    }
    
    printf("Process %d of %d terminated \n", myRank, mySize);
    return;
}

// Main. Initalizes MPI, starts a master and some workers, waits results and finalizes MPI.
// Also times the whole computation.
// TO DO -- Change timing to double MPI_Wtime( void ) to use wall time instead of cpu time.
int main(int argc, char *argv[]) {
    int chessboard[BOARDSIZE], myRank, mySize, solutions;
    printf("Program started \n");
    
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &myRank);
    MPI_Comm_size(MPI_COMM_WORLD, &mySize);
    if (myRank==0){
        printf("Master initiated \n");
        solutions = nq_recursion_master(myRank, mySize);
    }
    else {
        nq_recursion_worker(myRank, mySize);
    }

    MPI_Finalize();
    printf("Solutions: %d seconds \n ", solutions);
    return 0;
}


