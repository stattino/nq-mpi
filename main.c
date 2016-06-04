#include <stdio.h>
#include <stdlib.h>
#include <time.h>
//static const int BOARDSIZE=14;
static int solutions=0;
typedef int bool;
#define true 1
#define false 0

static bool threatens(const int row_1,const int row_2,const int col_1,const int col_2) {
    //  Checks if the queens are on the same row, or diagonal
    return (row_1==row_2||abs(row_1-row_2)==abs(col_1-col_2)) ? true:false;
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

static void nq_recursion(const int counter, int board[], int boardSize) {
    // row = number in the board vector
    // col = index in board vector
    // counter = which column is "active"
    // End condition, counter is boardsize and no threats: add a solution
    if (counter >= boardSize) {
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
        for(int row=0; row<boardSize; row++){
            board[counter] = row;
            nq_recursion(counter+1, board, boardSize);
        }
    }
}

int main(int argc, char *argv[]) {
    int boardSize;
    (argc<2) ? boardSize=10 : sscanf(argv[1],"%d",&boardSize);
    //printf("Input: %d", N);
    
    int chessboard[boardSize];
    time_t tStart;
    tStart = time(NULL);
    nq_recursion(0, chessboard, boardSize);
    tStart = time(NULL) - tStart;
    printf("Solutions: %d in %f seconds", solutions, ((double)(tStart)));
}



