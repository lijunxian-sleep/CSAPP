/* 
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */ 
#include <stdio.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

void transpose_1(int M, int N, int A[N][M], int B[M][N]){
    for(int i=0;i<32;i+=8){
        for(int j=0;j<32;j+=8){
            if(i != j){
                for(int p=0;p<8;p++){
                    for(int q=0;q<8;q++){
                        B[i+p][j+q] = A[j+q][i+p];
                    }
                }
            }
        }
    }
    
    for(int i=0; i<32; i+=8){
        for(int p=0;p<4;p++){
            for(int q=4;q<8;q++){
                B[i+q][i+p] = A[i+p][i+q];
            }
            for(int q=0; q<4; q++){
                B[i+p+4][i+q+4] = A[i+p][i+q];
            }
        }
        for(int p=0;p<4;p++){
            for(int q=0;q<4;q++){
                B[i+p][i+q] = B[i+4+q][i+4+p];
            }
        }
        B[i+0][i+4] = A[i+4][i+0]; B[i+0][i+5] = A[i+5][i+0];
        B[i+1][i+4] = A[i+4][i+1]; B[i+1][i+5] = A[i+5][i+1];
        B[i+2][i+4] = A[i+4][i+2]; B[i+2][i+5] = A[i+5][i+2];
        B[i+3][i+4] = A[i+4][i+3]; B[i+3][i+5] = A[i+5][i+3];
        B[i+6][i+4] = A[i+4][i+6]; B[i+6][i+5] = A[i+5][i+6];
        B[i+7][i+4] = A[i+4][i+7]; B[i+7][i+5] = A[i+5][i+7];
        int a = A[i+4][i+4], b = A[i+5][i+4], c = A[i+4][i+5], d = A[i+5][i+5];
        for(int p=6;p<8;p++){
            for(int q=0;q<6;q++){
                B[i+q][i+p] = A[i+p][i+q];
            }
        }
        B[i+4][i+4] = a; B[i+5][i+4] = c; B[i+4][i+5] = b; B[i+5][i+5] = d;
        a = A[i+6][i+6]; b = A[i+7][i+6]; c = A[i+6][i+7]; d = A[i+7][i+7];
        B[i+6][i+6] = a; B[i+7][i+6] = c; B[i+6][i+7] = b; B[i+7][i+7] = d;
    }
}

void transpose_2(int M, int N, int A[N][M], int B[M][N]){
    for(int i=0;i<64;i+=8){
        for(int j=0;j<64;j+=8){
            if(i == j)  continue;
            //非对角线上4行和下4行冲突。
            for(int p=0;p<4;p++){
                for(int q=0;q<4;q++){
                    B[i+p][j+q] = A[j+q][i+p]; 
                }
            }
            int x1=A[j+0][i+4], x2=A[j+0][i+5], x3=A[j+0][i+6], x4=A[j+0][i+7];
            int x5=A[j+1][i+4], x6=A[j+1][i+5], x7=A[j+1][i+6], x8=A[j+1][i+7];
            for(int p=4;p<8;p++){
                B[i+2][j+p] = A[j+2][i+p];
                B[i+3][j+p] = A[j+3][i+p];
            }
            //A前4行全取出,开始取后4行
            for(int q=4;q<8;q++){
                B[i][j+q] = A[j+q][i];
                B[i+1][j+q] = A[j+q][i+1];
            }
            //B前2行全放入，开始取5，6行。
            B[i+4][j+0] = x1, B[i+4][j+1] = x5;
            B[i+5][j+0] = x2, B[i+5][j+1] = x6;
            B[i+4][j+2] = B[i+2][j+4], B[i+4][j+3] = B[i+3][j+4];
            B[i+5][j+2] = B[i+2][j+5], B[i+5][j+3] = B[i+3][j+5];
            //B3，4行全放入，开始取后4行。
            x1=B[i+2][j+6], x2=B[i+2][j+7];
            x5=B[i+3][j+6], x6=B[i+3][j+7];
            for(int q=4;q<8;q++){
                B[i+2][j+q] = A[j+q][i+2];
                B[i+3][j+q] = A[j+q][i+3];
            }
            B[i+6][j+0] = x3, B[i+6][j+1] = x7;
            B[i+7][j+0] = x4, B[i+7][j+1] = x8;
            B[i+6][j+2] = x1, B[i+6][j+3] = x5;
            B[i+7][j+2] = x2, B[i+7][j+3] = x6;
            for(int p=4;p<8;p++){
                for(int q=4;q<8;q++){
                    B[i+p][j+q] = A[j+q][i+p];
                }
            }
            
        }
    }
    int x1,x2,x3,x4,x5,x6,x7,x8;
    for(int i=0;i<64;i+=8){
        //A1 B4
        B[i+6][i+0] = A[i+0][i+6], B[i+6][i+1] = A[i+1][i+6], B[i+7][i+0] = A[i+0][i+7], B[i+7][i+1] = A[i+1][i+7];     //1.4->B4
        B[i+6][i+6] = A[i+0][i+4], B[i+6][i+7] = A[i+0][i+5], B[i+7][i+6] = A[i+1][i+4], B[i+7][i+7] = A[i+1][i+5];     //1.3->B4
        x1=A[i+0][i+0], x2=A[i+0][i+1], x3=A[i+1][i+0], x4=A[i+1][i+1];                                                 //1.1->x1
        x5=A[i+0][i+2], x6=A[i+0][i+3], x7=A[i+1][i+2], x8=A[i+1][i+3];                                                 //1.2->x5
        //A3 B4
        B[i+6][i+4] = A[i+4][i+6], B[i+7][i+4] = A[i+4][i+7], B[i+6][i+5] = A[i+5][i+6], B[i+7][i+5] = A[i+5][i+7];     //3.4->B4
        //A3 B2
        B[i+2][i+0] = x5, B[i+2][i+1] = x7, B[i+3][i+0] = x6, B[i+3][i+1] = x8;                                         //1.2->B2
        B[i+2][i+4] = A[i+4][i+2], B[i+2][i+5] = A[i+5][i+2], B[i+3][i+4] = A[i+4][i+3], B[i+3][i+5] = A[i+5][i+3];     //3.2->B2
        B[i+2][i+6] = A[i+4][i+4], B[i+2][i+7] = A[i+4][i+5], B[i+3][i+6] = A[i+5][i+4], B[i+3][i+7] = A[i+5][i+5];     //3.3->B2
        x5=A[i+4][i+0], x6=A[i+4][i+1], x7=A[i+5][i+0], x8=A[i+5][i+1];                                                 //3.1->x5
        //B1 B2
        B[i+0][i+0] = x1, B[i+0][i+1] = x3, B[i+1][i+0] = x2, B[i+1][i+1] = x4;                                         //1.1->B1
        B[i+0][i+4] = x5, B[i+0][i+5] = x7, B[i+1][i+4] = x6, B[i+1][i+5] = x8;                                         //3.1->B1
        x1=B[i+2][i+6], x2=B[i+2][i+7], x3=B[i+3][i+6], x4=B[i+3][i+7];                                                 //3.3->x1
        //A2 B1
        B[i+0][i+2] = A[i+2][i+0], B[i+0][i+3] = A[i+3][i+0], B[i+1][i+2] = A[i+2][i+1], B[i+1][i+3] = A[i+3][i+1];     //2.1->B1
        //A2 B3
        B[i+4][i+2] = A[i+2][i+4], B[i+4][i+3] = A[i+3][i+4], B[i+5][i+2] = A[i+2][i+5], B[i+5][i+3] = A[i+3][i+5];     //2.3->B3
        B[i+4][i+4] = x1, B[i+4][i+5] = x3, B[i+5][i+4] = x2, B[i+5][i+5] = x4;                                         //3.3->B3
        x1=A[i+2][i+2], x2=A[i+2][i+3], x3=A[i+3][i+2], x4=A[i+3][i+3];                                                 //2.2->x1
        x5=A[i+2][i+6], x6=A[i+2][i+7], x7=A[i+3][i+6], x8=A[i+3][i+7];                                                 //2.4->x5
        //B3 B4
        B[i+6][i+2] = x5, B[i+6][i+3] = x7, B[i+7][i+2] = x6, B[i+7][i+3] = x8;                                         //2.4->B4
        B[i+4][i+0] = B[i+6][i+6], B[i+4][i+1] = B[i+7][i+6], B[i+5][i+0] = B[i+6][i+7], B[i+5][i+1] = B[i+7][i+7];     //1.3->B3
        //A4 B3
        B[i+4][i+6] = A[i+6][i+4], B[i+4][i+7] = A[i+7][i+4], B[i+5][i+6] = A[i+6][i+5], B[i+5][i+7] = A[i+7][i+5];     //4.3->B3
        //A4 B1
        B[i+0][i+6] = A[i+6][i+0], B[i+0][i+7] = A[i+7][i+0], B[i+1][i+6] = A[i+6][i+1], B[i+1][i+7] = A[i+7][i+1];     //4.1->B1
        x5=A[i+6][i+2], x6=A[i+6][i+3], x7=A[i+7][i+2], x8=A[i+7][i+3];                                                 //4.2->x5
        //B2 B1
        B[i+2][i+2] = x1, B[i+2][i+3] = x3, B[i+3][i+2] = x2, B[i+3][i+3] = x4;                                         //2.2->B2
        B[i+2][i+6] = x5, B[i+2][i+7] = x7, B[i+3][i+6] = x6, B[i+3][i+7] = x8;                                         //4.2->B2
        //A4 B1
        x5=A[i+6][i+6], x6=A[i+6][i+7], x7=A[i+7][i+6], x8=A[i+7][i+7];                                                 //4.4->x5
        //B4 B1
        B[i+6][i+6] = x5, B[i+6][i+7] = x7, B[i+7][i+6] = x6, B[i+7][i+7] = x8;                                         //4.4->B2
    }
}   

void transpose_3(int M, int N, int A[N][M], int B[M][N]){
    int e = 18;

    for(int i=0;i<M;i+=e){
        for(int j=0;j<N;j+=e){
            for(int p=i; p<M && p<i+e; p++){
                for(int q=j; q<N && q <j+e; q++){
                    B[p][q] = A[q][p];
                }
            }
        }
    }
}

void trans(int M, int N, int A[N][M], int B[M][N]);
/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    if(M == 32) transpose_1(M, N, A, B);   
    if(M == 64) transpose_2(M, N, A, B);   
    if(M == 61) transpose_3(M, N, A, B);
}

/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */ 

/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }    

}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc); 

    /* Register any additional transpose functions */
    registerTransFunction(trans, trans_desc); 

}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}

