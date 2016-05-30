#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
const int BOARDSIZE=8; // Add this as input-argument to automize

typedef int bool;
#define true 1
#define false 0

bool threatens(const int row_1, const int row_2, const int col_1, const int col_2) {
    if ( row_1==row_2||abs(row_1-row_2)==abs(col_1-col_2) ){
        return true;
    }
    else{
        return false;
    }
}

// Could hybridize w. OpenMP?
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

/* Used to return the partial solutions from a sub-tree of the complete search-tree. Returns the solutions recursively.
 */
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

/* Changes values in nextPos to a valid configuration, ready for a worker. Returns non-valid position (out of board range) if it has gone through all valid already.
 */
void next_position(int chessboard[], int* nextPos) {
    // Start by incrementing by one so not to return original position
    if (chessboard[1]==BOARDSIZE-1) {
        chessboard[0]++;
        chessboard[1]=0;
    }
    else {
        chessboard[1]++;
    }
    
    // Then keep changing as long as its not valid
    while (!is_checked(2,chessboard)) {
        if (chessboard[0] == BOARDSIZE) {
            break;
        }
        else if (chessboard[1]>=BOARDSIZE-1) {
            chessboard[0]++;
            chessboard[1]=0;
        }
        else {
            chessboard[1]++;
        }
    }
    nextPos[0] = chessboard[0];
    nextPos[1] = chessboard[1];
}

/* Master for when number of proc. are greater than the board.
 */
int nq_recursion_master_bigboard(int myRank, int mySize) {
    
    if (mySize>((BOARDSIZE-1)*(BOARDSIZE-2))) {printf("Not implemented yet! \n");return 0;}
    printf("  ... Entered master process ...   \n");

    int chessboard[BOARDSIZE], buf[3], activeWorkers, totalSolutions,  pos[2], sender;
    buf[2] = 1;
    MPI_Status status;
    chessboard[0] = 0;
    chessboard[1] = 0;
    
    int* ptrPos;
    pos[0] = 0;
    pos[1] = 0;
    ptrPos = pos;
    // Initial workload for all workers
    for (activeWorkers=0; activeWorkers<mySize-1;activeWorkers++) {
        next_position(chessboard, ptrPos);
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
        next_position(chessboard, ptrPos);
        if (ptrPos[0]<BOARDSIZE){
            buf[0] = ptrPos[0];
            buf[1] = ptrPos[1];
            MPI_Send(&buf, 3, MPI_INT, sender, 0, MPI_COMM_WORLD);
        }
        else {
            buf[2] = 0;
            MPI_Send(&buf, 3, MPI_INT, sender, 0, MPI_COMM_WORLD);
            activeWorkers--;
        }
    }
    printf(" ... Master process terminated ... \n");
    return totalSolutions;
}

/* Worker for when no. processes>boardsize
 */
void nq_recursion_worker_bigboard(int myRank,int mySize) {
    int buf[3], partialSolutions;
    MPI_Status status;
    printf("Worker %d of %d initiated \n", myRank+1, mySize);
    // Receive initial work
    MPI_Recv(&buf, 3, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
    // Receive more work until termination message received
    while (buf[2]==1) {
        partialSolutions=0;
        int chessboard[BOARDSIZE];
        chessboard[0] = buf[0];
        chessboard[1] = buf[1];
        partialSolutions = worker_recursion(2, chessboard);
        buf[0] = partialSolutions;
        MPI_Send(&buf, 3, MPI_INT, 0, 0, MPI_COMM_WORLD);
        MPI_Recv(&buf, 3, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
    }
    return;
}

/* Master mpi function. Called by only one process. Deals out work to worker processes. Right now, N < boardsize, and it deals out N tasks in total, where each task is the board with the first queen on a row.
 */
int nq_recursion_master(int myRank, int mySize) {
    int buf[2], row, activeWorkers, totalSolutions, sender;
    MPI_Status status;
    totalSolutions = 0;
    
    printf("... Entered master process ... \n");
    

    // Send one job to each process in the commgroup. Keep row as tracker of where the queen has been placed.
    for (row=0; row<mySize-1; row++) {
        buf[0] = row;
        buf[1] = 1;
        MPI_Send(&buf, 2, MPI_INT, row+1, 0, MPI_COMM_WORLD);
        printf("Master dealt out job %d of %d \n ", row+1, BOARDSIZE);
    }
    activeWorkers = row;

    // Controls communication to the workers while they are active. When a solution is received: either send more work to, or kill, the process.
    while (activeWorkers>0) {
        MPI_Recv(&buf, 2, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
        sender = status.MPI_SOURCE;
        
        totalSolutions += buf[0];
        printf("Worker no. %d found %d solutions \n ", sender, buf[0]);

        // If received solution was to the final task: initialize a count down, and start killing processes. Otherwise keep sending out work.
        if (row == BOARDSIZE) {
            activeWorkers = mySize-2; // mySize - (thisprocess+masterprocess)
            buf[1] = 0;
            MPI_Send(&buf, 2, MPI_INT, sender, 0, MPI_COMM_WORLD);
            row++;
        }
        else if (row>BOARDSIZE) {
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

/* Worker mpi function. Receives a position on the board and starts worker_recursions on them. Terminates only when they receive a termination message in buf[1].
 */
void nq_recursion_worker(int myRank,int mySize) {
    int buf[2], row, partialSolutions;
    MPI_Status status;
    printf("Worker %d of %d initiated \n", myRank, mySize-1);

    // Receive initial work
    MPI_Recv(&buf, 2, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
    
    // Receive more work until termination message received
    while (buf[1]==1) {
        row = buf[0];
        int chessboard[BOARDSIZE];
        chessboard[0] = row;
        partialSolutions = worker_recursion(1, chessboard);
        buf[0] = partialSolutions;
        MPI_Send(&buf, 2, MPI_INT, 0, 0, MPI_COMM_WORLD);
        MPI_Recv(&buf, 2, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
    }
    
    printf("Process %d of %d terminated \n", myRank, mySize);
    return;
}

/* Main. Initalizes MPI, starts a master and some workers, waits results and finalizes MPI. Also times the whole computation. TO DO -- Change timing to double MPI_Wtime( void ) to use wall time instead of cpu time.
 */
int main(int argc, char *argv[]) {
    int myRank, mySize, solutions;
    
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &myRank);
    MPI_Comm_size(MPI_COMM_WORLD, &mySize);

    if (mySize<BOARDSIZE+1){
        if (myRank==0){
            solutions = nq_recursion_master(myRank, mySize);
            printf("Solutions: %d (seconds.. to be implemented) \n ", solutions);
        }
        else {
            nq_recursion_worker(myRank, mySize);
        }
        
    }
    else {
        if (myRank==0){
            solutions = nq_recursion_master_bigboard(myRank, mySize);
            printf("Solutions: %d (seconds.. to be implemented) \n ", solutions);
        }
        else {
            nq_recursion_worker_bigboard(myRank, mySize);
        }
    }

    MPI_Finalize();
    return 0;
}