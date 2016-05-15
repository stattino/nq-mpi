#include <stdio.h>
#include <stdlib.h>
// #include <mpi.h>
static const int BOARDSIZE=7;
static int solutions=0;


static bool threatens(const int row_1,const int row_2,const int col_1,const int col_2) {
    //  Feels pretty done
    //  Checks if the queens are on the same row, or diagonal
    if ( row_1==row_2||abs(row_1-row_2)==abs(col_1-col_2) ){
        return true;
    }
    else{
        return false;
    }
}

static bool is_checked(const int counter, int board[]) {
    // parallelize this
    // Redundant checking ATM.. to be fixed
    int row_1, row_2;
    // first loop over all the columns
    for (int col_1=0; col_1<counter; col_1++){
        row_1 = board[col_1];
        // second loop over following columns. no need checking the same cols/rows
        for (int col_2=col_1+1; col_2<counter; col_2++) {
            row_2 = board[col_2];
            if (threatens(row_1, row_2, col_1, col_2)) {return false;}
        }
    }
    return true;
}

static void nq_recursion(const int counter, int board[]) {
    // row = number in the board vector
    // col = index in board vector
    // counter decides which column we are in. switch name mbe...
    
    // End condition, counter is boardsize and no threats: add a solution
    if (counter >= BOARDSIZE) {
        if (is_checked(counter,board)) {
            solutions++;
        }
        return;
    }
    
    // If position violates no-threat rule, "backtrack"
    else if (!is_checked(counter,board)) {
        return;
    }
    
    // Recursion over all the new row-positions for this col
    // This is to be parallelized
    else{
        for(int row=0; row<BOARDSIZE; row++){
            board[counter] = row;
            nq_recursion(counter+1, board);
        }
    }
}


int main(int argc, char *argv[]) {
    // std::vector<int> chessboard(BOARDSIZE);
    int chessboard[BOARDSIZE];
    nq_recursion(0, chessboard);
    printf("Solutions: %i", solutions);
    
    /*
     int myrank, mysize;
     MPI_Status status;
     
     MPI_Init(&argc, &argv);
     MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
     MPI_Comm_size(MPI_COMM_WORLD, &mysize);
     
     MPI_Comm_spawn()
     
     MPI_Finalize();*/
}



