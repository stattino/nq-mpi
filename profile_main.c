#include <stdio.h>
#include <stdlib.h>
static int solutions=0;

typedef int bool;
#define true 1
#define false 0

double second() {
    struct timeval tp;
    gettimeofday(&tp, NULL);
    return ((double)tp.tv_sec + (double)tp.tv_usec * 1.e-6);
}

static bool threatens(const int row_1,const int row_2,const int col_1,const int col_2) {
    return (row_1==row_2||abs(row_1-row_2)==abs(col_1-col_2)) ? true:false;
}

static bool is_checked(const int counter, int board[]) {
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

static void nq_recursion(const int counter, int board[], int boardSize) {
    if (counter >= boardSize) {
        if (is_checked(counter,board)) {
            solutions++;
        }
        return;
    }
    else if (!is_checked(counter,board)) {
        return;
    }
    else{
        for(int row=0; row<boardSize; row++){
            board[counter] = row;
            nq_recursion(counter+1, board, boardSize);
        }
    }
}

int main(int argc, char *argv[]) {
    int boardSize=13;
    int chessboard[boardSize];
    nq_recursion(0, chessboard, boardSize);
    printf("Solutions: %d in %4.4f seconds \n", solutions, tStart);
}



