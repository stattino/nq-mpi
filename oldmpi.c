#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

typedef int bool;
#define true 1
#define false 0


bool threatens(const int row_1, const int row_2, const int col_1, const int col_2) {
    return (row_1==row_2||abs(row_1-row_2)==abs(col_1-col_2)) ? true:false;
}

bool is_checked(const int counter, int board[]) {
    int row_1, row_2;
    for (int col_1=0; col_1<counter; col_1++){
        row_1 = board[col_1];
        for (int col_2=col_1+1; col_2<counter; col_2++) {
            row_2 = board[col_2];
            if (threatens(row_1, row_2, col_1, col_2)) return false;
        }
    }
    return true;
}

/* Used to return the partial solutions from a sub-tree of the complete search-tree. Returns the solutions recursively.
 */
int worker_recursion(int counter, int board[], int boardSize){
    int workerSols = 0;
    if (counter >= boardSize) {
        if (is_checked(counter,board)) {
            workerSols++;
            return workerSols;
        }
    }
    else if (is_checked(counter,board)){
        for(int row=0; row<boardSize; row++){
            board[counter] = row;
            workerSols += worker_recursion(counter+1, board, boardSize);
        }
    }
    return workerSols;
}

/* Master mpi function. Called by only one process. Deals out work to worker processes. Right now, N < boardsize, and it deals out N tasks in total, where each task is the board with the first queen on a row.
 */
int nq_recursion_master(int myRank, int mySize, int boardSize, double timeVec[]) {
    int buf[2], row, activeWorkers, totalSolutions, sender;
    double tBuf[2];
    MPI_Status status;
    totalSolutions = 0;
    
    printf("... Entered master process ... \n");
    
    // Send one job to each process in the commgroup. Keep row as a tracker of where the queen has been placed.
    for (row=0; row<mySize-1; row++) {
        buf[0] = row;
        buf[1] = 1;
        MPI_Send(&buf, 2, MPI_INT, row+1, 0, MPI_COMM_WORLD);
        printf("Master dealt out job %d of %d \n ", row+1, boardSize);
    }
    activeWorkers = row;
    
    // Controls communication to the workers while they are active. When a solution is received: either send more work to, or kill, the process.
    while (activeWorkers>0) {
        MPI_Recv(&buf, 2, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
        sender = status.MPI_SOURCE;
        totalSolutions += buf[0];
        // If received solution was to the final task: initialize a count down, and start killing processes. Otherwise keep sending out work.
        if (row == boardSize) {
            buf[1] = 0;
            MPI_Send(&buf, 2, MPI_INT, sender, 0, MPI_COMM_WORLD); // kill
            MPI_Recv(&tBuf, 2, MPI_DOUBLE, sender, 1, MPI_COMM_WORLD, &status); // get time
            timeVec[2*sender] = tBuf[0];
            timeVec[2*sender+1] = tBuf[1];
            activeWorkers--;
        }
        else {
            buf[0] = row;
            buf[1] = 1;
            MPI_Send(&buf, 2, MPI_INT, sender, 0, MPI_COMM_WORLD);
            printf("Master dealt out job %d of %d \n", row+1, boardSize);
            row++;
        }
    }
    printf("Master process terminated \n");
    return totalSolutions;
}
/* Worker mpi function. Receives a position on the board and starts worker_recursions on them. Terminates only when they receive a termination message in buf[1].
 */
void nq_recursion_worker(int myRank, int mySize, int boardSize) {
    int buf[2], row, partialSolutions;
    double tBuf[2], nPlacements, tStart, tEnd;
    tStart = MPI_Wtime();
    
    MPI_Status status;
    // Receive initial work
    MPI_Recv(&buf, 2, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
    
    // Receive more work until termination message received
    while (buf[1]==1) {
        row = buf[0];
        int chessboard[boardSize];
        chessboard[0] = row;
        partialSolutions = worker_recursion(1, chessboard, boardSize);
        buf[0] = partialSolutions;
        MPI_Send(&buf, 2, MPI_INT, 0, 0, MPI_COMM_WORLD); // Could use sendrecv?
        MPI_Recv(&buf, 2, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
        nPlacements++;
    }
    
    tEnd = MPI_Wtime();
    
    tBuf[0] = tEnd-tStart; // Could make an MPI struct just to try it out, with one double and one int
    tBuf[1] = nPlacements; // instead of having nPlacements as a double, when it only takes int values.
    MPI_Send(&tBuf, 2, MPI_DOUBLE, 0, 1, MPI_COMM_WORLD); // tag 1 for timing
    return;
}

/* Main. Initalizes MPI, starts a master and some workers, waits results and finalizes MPI. Also times the whole computation. TO DO -- Change timing to double MPI_Wtime( void ) to use wall time instead of cpu time.
 */
int main(int argc, char *argv[]) {
    int myRank, mySize, solutions, boardSize;
    double tStart, tEnd;
    
    (argc<2) ? boardSize=10 : sscanf(argv[1],"%d",&boardSize);
    
    
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &myRank);
    MPI_Comm_size(MPI_COMM_WORLD, &mySize);
    if (mySize-1>boardSize) {if (myRank==0) printf("use mpimain.c instead \n"); return 0;}
    // Do a comparison of speed for same n and boardsize, then keep the faster version.
    if (myRank==0){
        double timeVec[2*mySize];
        tStart = MPI_Wtime();
        solutions = nq_recursion_master(myRank, mySize, boardSize, timeVec);
        tEnd = MPI_Wtime();
        printf("Solutions: %d  Total time elapsed:  %10.4f \n The worker processes' timings below \n", solutions, tEnd-tStart);
        for (int i=0; i<mySize; i++) {
            printf("Process %d: %10.4f s, %1.0f positions \n", i, timeVec[2*i], timeVec[2*i+1]);
        }
        else {
            nq_recursion_worker(myRank, mySize, boardSize);
        }
        
        MPI_Finalize();
        
        return 0;
    }
}
