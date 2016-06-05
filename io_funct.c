#include "io_funct.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void write_qualitative(int nProc, int boardSize, double elapsedTime, double timeVec[], int depth){
    char filename[6+4+2+2+2+1] = {'0'};
    char prefix[] = "qual_n";
    char suffix[] = ".txt";
    sprintf(filename, "%s%2d%s%2d%s%d%s", prefix, nProc, "b", boardSize, "d", depth, suffix);
    
    FILE *outFile;
    outFile = fopen(filename, "w");
    if (outFile == NULL) {
        fprintf(stderr, "Can't open output file %s!\n", filename);
        exit(1);
    }

    fprintf(outFile, "%d, %d, %4.4f \n", nProc, boardSize, elapsedTime);
    
    for (int i=1; i<nProc; i++) {
        fprintf(outFile, "%d, %4.4f, %1.0f \n", i, timeVec[2*i], timeVec[2*i+1]);
    }
    fclose(outFile);
}

void write_quantitative(int nProc, int boardSize, double elapsedTime) {
    FILE *outFile;
    outFile = fopen("quant.txt", "a");
    
    if (outFile == NULL) {
        fprintf(stderr, "Can't open output file %s!\n", "quant.txt");
        exit(1);
    }
    fprintf(outFile, "%d, %d, %4.4f \n", nProc, boardSize, elapsedTime);
    
    fclose(outFile);
}

