#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include "io_funct.h"

typedef int bool;
#define true 1
#define false 0


bool threatens(const int row_1, const int row_2, const int col_1, const int col_2) {
    return (row_1==row_2||abs(row_1-row_2)==abs(col_1-col_2));
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

/* Changes values in nextPos to a valid configuration, ready for a worker. Returns non-valid position (out of board range) if it has gone through all valid already.
 */
void next_position(int board[], int* nextPos, int boardSize) {
    // Start by incrementing by one so not to return original position
    if (board[1]==boardSize-1) {
        board[0]++;
        board[1]=0;
    }
    else board[1]++;
    
    // Then keep changing as long as its not valid
    while (!is_checked(2,board)) {
        if (board[0] == boardSize) break;
        else if (board[1]>=boardSize-1) {
            board[0]++;
            board[1]=0;
        }
        else board[1]++;
    }
    nextPos[0] = board[0];
    nextPos[1] = board[1];
}

/* Master for when number of proc. are greater than the board.
 */
int nq_recursion_master_bigboard(int myRank, int mySize, int boardSize, double timeVec[]) {
    
    if (mySize>((boardSize-1)*(boardSize-2))) {printf("Not implemented yet! \n");return 0;}
    printf("  ... Entered master process ...   \n");
    int chessboard[boardSize], buf[3], activeWorkers, totalSolutions, pos[2], sender;
    double tBuf[2];
    totalSolutions=0;
    
    buf[2] = 1;
    MPI_Status status;
    chessboard[0] = 0;
    chessboard[1] = 0;
    
    int* ptrPos;
    pos[0] = 0;
    pos[1] = 0;
    ptrPos = pos;
    // Initial workload for all workers
    for (activeWorkers=0; activeWorkers<mySize-1; activeWorkers++) {
        next_position(chessboard, ptrPos, boardSize);
        buf[0] = ptrPos[0];
        buf[1] = ptrPos[1];
        MPI_Send(&buf, 3, MPI_INT, activeWorkers+1, 0, MPI_COMM_WORLD);
    }
    // Start receiving solutions and dish out work until none left. Then kill processes
    while (activeWorkers>0) {
        MPI_Recv(&buf, 3, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
        sender = status.MPI_SOURCE;
        //printf("Worker no. %d found %d solutions \n ", sender, buf[0]);
        totalSolutions += buf[0];
        next_position(chessboard, ptrPos, boardSize);
        if (ptrPos[0]<boardSize){
            buf[0] = ptrPos[0];
            buf[1] = ptrPos[1];
            MPI_Send(&buf, 3, MPI_INT, sender, 0, MPI_COMM_WORLD);
        }
        else {
            buf[2] = 0;
            MPI_Send(&buf, 3, MPI_INT, sender, 0, MPI_COMM_WORLD);
            MPI_Recv(&tBuf, 2, MPI_DOUBLE, sender, 1, MPI_COMM_WORLD, &status); // get time
            timeVec[2*sender] = tBuf[0];
            timeVec[2*sender+1] = tBuf[1];
            activeWorkers--;
        }
    }
    printf(" ... Master process terminated ... \n");
    return totalSolutions;
}

/* Worker for when no. processes>boardsize
 */
void nq_recursion_worker_bigboard(int myRank, int mySize, int boardSize) {
    int buf[3], partialSolutions;
    double tBuf[2], nPlacements, tStart, tEnd;
    tStart = MPI_Wtime();
    
    MPI_Status status;
    // Receive initial work
    MPI_Recv(&buf, 3, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
    // Receive more work until termination message received
    while (buf[2]==1) {
        partialSolutions=0;
        int chessboard[boardSize];
        chessboard[0] = buf[0];
        chessboard[1] = buf[1];
        buf[0] = worker_recursion(2, chessboard, boardSize);
        MPI_Send(&buf, 3, MPI_INT, 0, 0, MPI_COMM_WORLD);
        MPI_Recv(&buf, 3, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
        nPlacements++;
    }
    tEnd = MPI_Wtime();
    
    // Could make an MPI struct just to try it out, with one double and one int instead of having nPlacements as a double, when it only takes int values.
    tBuf[0] = tEnd-tStart;
    tBuf[1] = nPlacements;
    MPI_Send(&tBuf, 2, MPI_DOUBLE, 0, 1, MPI_COMM_WORLD); // tag 1 for timing
    return;
}

/* Main. Initalizes MPI, starts a master and some workers, waits results and finalizes MPI. Also times the whole computation.
 */
int main(int argc, char *argv[]) {
    int myRank, mySize, solutions, boardSize;
    double tStart, tEnd;
    (argc<2) ? boardSize=10 : sscanf(argv[1],"%d",&boardSize);
    
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &myRank);
    MPI_Comm_size(MPI_COMM_WORLD, &mySize);
    
    if (myRank==0){
        double timeVec[2*mySize];
        tStart = MPI_Wtime();
        solutions = nq_recursion_master_bigboard(myRank, mySize, boardSize, timeVec);
        tEnd = MPI_Wtime();
        printf("Solutions: %d  Total time elapsed:  %10.4f \n The worker processes' timings below \n", solutions, tEnd-tStart);
        for (int i=0; i<mySize; i++){
            printf("Process %d: %10.4f s, %1.0f positions \n", i, timeVec[2*i] , timeVec[2*i+1]);
        }
        if (argc>2){
            if (argv[2]==1) write_qualitative(mySize, boardSize,  tEnd-tStart, timeVec, 0);
            else write_quantitative(mySize, boardSize,  tEnd-tStart);
        }
    }
    else {
        nq_recursion_worker_bigboard(myRank, mySize, boardSize);
    }
    MPI_Finalize();
    return 0;
}