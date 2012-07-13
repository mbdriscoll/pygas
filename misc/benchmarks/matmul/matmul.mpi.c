#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <sys/time.h>
#include <mpi.h>

extern void SGEMM (char*, char*,
        int*, int*, int*,
        float*, float*, int*,
        float*, int*, float*,
        float*, int*); 

void reference_dgemm (int N, float* A, float* B, float* C)
{
    char TRANSA = 'N';
    char TRANSB = 'N';
    int M = N;
    int K = N;
    float ALPHA= 1.;
    float BETA = 1.;
    int LDA = N;
    int LDB = N;
    int LDC = N;
    SGEMM(&TRANSA, &TRANSB, &M, &N, &K, &ALPHA, A, &LDA, B, &LDB, &BETA, C, &LDC);
}  

// Returns a uniformly distributed float between 0.5 and 1.5
float randd(void) {
    return (float) 0.5 + (rand() / (float) RAND_MAX);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: ./mpi <MATRIXSIZE>\n");
        exit(1);
    }
    int n = atoi(argv[1]);

    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int sqrt_P = (int) sqrt((float) size);
    if (sqrt_P * sqrt_P != size) {
        printf("Number of ranks must be square, not %d\n", size);
        exit(2);
    }

    int n_per_rank = n / sqrt_P;
    if (n_per_rank * sqrt_P != n) {
        printf("N must divide sqrt(P) evenly\n");
        exit(3);
    }

    int dims[] = {sqrt_P, sqrt_P};
    int periods[] = {1, 1};
    int row_rem[] = {0, 1};
    int col_rem[] = {1, 0};
    MPI_Comm cart, row, col;
    MPI_Cart_create(MPI_COMM_WORLD, 2, dims, periods, 0, &cart);
    MPI_Cart_sub(cart, row_rem, &row);
    MPI_Cart_sub(cart, col_rem, &col);

    int coords[2];
    MPI_Cart_coords(cart, rank, 2, coords);
    int x = coords[0];
    int y = coords[1];

    size_t A_bytes, B_bytes, C_bytes;
    int nn_per_rank = n_per_rank * n_per_rank;
    A_bytes = B_bytes = C_bytes = nn_per_rank*sizeof(float);
    float *A_ij = (float *) malloc(A_bytes);
    float *B_ij = (float *) malloc(B_bytes);
    float *C_ij = (float *) malloc(C_bytes);

    srand(rank*(unsigned)time());
    for (int i = 0; i < n_per_rank; i++) {
        for (int j = 0; j < n_per_rank; j++) {
            A_ij[i*n_per_rank+j] = randd();
            B_ij[i*n_per_rank+j] = randd();
            C_ij[i*n_per_rank+j] = 0.0;
        }
    }

    int row_shift_dest, row_shift_src, col_shift_dest, col_shift_src;
    MPI_Cart_shift(row, 0, x, &row_shift_src, &row_shift_dest);
    MPI_Cart_shift(col, 0, y, &col_shift_src, &col_shift_dest);

    int row_dest, row_src, col_dest, col_src;
    MPI_Cart_shift(row, 0, 1, &row_src, &row_dest);
    MPI_Cart_shift(col, 0, 1, &col_src, &col_dest);

    // Initial shift
    MPI_Status col_stat, row_stat;
    MPI_Sendrecv_replace(A_ij, nn_per_rank, MPI_FLOAT,
            row_shift_dest, 0, row_shift_src, 0, row, &row_stat);
    MPI_Sendrecv_replace(B_ij, nn_per_rank, MPI_FLOAT,
            col_shift_dest, 0, col_shift_src, 0, col, &col_stat);

    // Time computation and shifting
    struct timeval t_start, t_end;
    gettimeofday(&t_start,(struct timezone *)NULL);
    for (int k = 0; k < sqrt_P; k++) {
        reference_dgemm(n_per_rank, A_ij, B_ij, C_ij);
        MPI_Sendrecv_replace(&A_ij[0], nn_per_rank, MPI_FLOAT,
                row_dest, 0, row_src, 0, row, &row_stat);
        MPI_Sendrecv_replace(&B_ij[0], nn_per_rank, MPI_FLOAT,
                col_dest, 0, col_src, 0, col, &col_stat);
    }
    gettimeofday(&t_end,(struct timezone *)NULL);

    // Print performance
    if (rank == 0) {
        float time = (float) (t_end.tv_sec  - t_start.tv_sec) +
            1e-6 * (float) (t_end.tv_usec - t_start.tv_usec);
        float gflops = 1e-9 * pow(n, 3) / time;
        //printf("C/MPI Cannon ran at %f GFlop/s\n", gflops);
        printf("%d %f\n", n, gflops);
    }

    // Restore data layout
    MPI_Cart_shift(row, 0, -x, &row_shift_src, &row_shift_dest);
    MPI_Cart_shift(col, 0, -y, &col_shift_src, &col_shift_dest);
    MPI_Sendrecv_replace(A_ij, nn_per_rank, MPI_FLOAT,
            row_shift_dest, 0, row_shift_src, 0, row, &row_stat);
    MPI_Sendrecv_replace(B_ij, nn_per_rank, MPI_FLOAT,
            col_shift_dest, 0, col_shift_src, 0, col, &col_stat);

    // Terminate
    MPI_Finalize();
    return 0;
}
