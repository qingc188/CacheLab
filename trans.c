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
    if ((M == 32 && N == 32) || (M == 64 && N == 64)) {
        int t0, t1, t2, t3, t4, t5, t6, t7;

        for (int i = 0; i < N; i += 8) {
            for (int j = 0; j < M; j += 8) {
                for (int r = i; r < i + (M == 32 ? 8 : 4); ++r) {
                    /* Read the whole row before stores can evict it. */
                    t0 = A[r][j];     t1 = A[r][j + 1];
                    t2 = A[r][j + 2]; t3 = A[r][j + 3];
                    t4 = A[r][j + 4]; t5 = A[r][j + 5];
                    t6 = A[r][j + 6]; t7 = A[r][j + 7];
                    B[j][r] = t0;     B[j + 1][r] = t1;
                    B[j + 2][r] = t2; B[j + 3][r] = t3;
                    if (M == 32) {
                        B[j + 4][r] = t4; B[j + 5][r] = t5;
                        B[j + 6][r] = t6; B[j + 7][r] = t7;
                    } else {
                        /* Stage A's upper-right quadrant in B's upper-right. */
                        B[j][r + 4] = t4;     B[j + 1][r + 4] = t5;
                        B[j + 2][r + 4] = t6; B[j + 3][r + 4] = t7;
                    }
                }
                if (M == 32)
                    continue;

                /* Replace the staged values with A's lower-left quadrant. */
                for (int c = j; c < j + 4; ++c) {
                    t0 = A[i + 4][c]; t1 = A[i + 5][c];
                    t2 = A[i + 6][c]; t3 = A[i + 7][c];
                    t4 = B[c][i + 4]; t5 = B[c][i + 5];
                    t6 = B[c][i + 6]; t7 = B[c][i + 7];
                    B[c][i + 4] = t0; B[c][i + 5] = t1;
                    B[c][i + 6] = t2; B[c][i + 7] = t3;
                    B[c + 4][i] = t4;     B[c + 4][i + 1] = t5;
                    B[c + 4][i + 2] = t6; B[c + 4][i + 3] = t7;
                }
                for (int r = i + 4; r < i + 8; ++r) {
                    t0 = A[r][j + 4]; t1 = A[r][j + 5];
                    t2 = A[r][j + 6]; t3 = A[r][j + 7];
                    B[j + 4][r] = t0; B[j + 5][r] = t1;
                    B[j + 6][r] = t2; B[j + 7][r] = t3;
                }
            }
        }
    } else {
        /* Rectangular tiles keep an eight-element span along rows of A. */
        for (int i = 0; i < N; i += 20)
            for (int j = 0; j < M; j += 8)
                for (int r = i; r < i + 20 && r < N; ++r)
                    for (int c = j; c < j + 8 && c < M; ++c)
                        B[c][r] = A[r][c];
    }
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

