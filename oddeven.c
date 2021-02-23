/*
    An implementation of parallel odd-even sort
    The algorithm refers to
        "An Introduction to Parallel Programming" by Peter Pacheco
    Author: Zhixuan Wu
    Date: 2/23/2021
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <mpi.h>
#include<time.h>

int compute_partner(int phase, int my_rank, int n_p) {
    int partner;
    if (phase%2 == 0) {
        if (my_rank % 2 != 0)
            partner = my_rank-1;
        else
            partner = my_rank+1;
    } 
    else {
        if (my_rank % 2 != 0)
            partner = my_rank+1;
        else
            partner = my_rank-1;
    } 
    if (partner==-1 || partner==n_p) {
        partner = MPI_PROC_NULL;
    }
    return partner;
}

void mergelow(int my_list[], int recv_list[], int temp_list[], int local_N) {
    int m_i, r_i, t_i;
    m_i = r_i = t_i = 0;
    while (t_i < local_N) {
        if (my_list[m_i] < recv_list[r_i]) {
            temp_list[t_i] = my_list[m_i];
            t_i++; m_i++;
        } else {
            temp_list[t_i] = recv_list[r_i];
            t_i++; r_i++;
        }
    }
    for (m_i = 0; m_i < local_N; m_i++)
        my_list[m_i] = temp_list[m_i];
}

void mergehigh(int my_list[], int recv_list[], int temp_list[], int local_N) {
    int m_i, r_i, t_i;
    m_i = r_i = local_N-1;
    t_i = local_N-1;
    while (t_i >= 0) {
        if (my_list[m_i] > recv_list[r_i]) {
            temp_list[t_i] = my_list[m_i];
            t_i--; m_i--;
        } else  {
            temp_list[t_i] = recv_list[r_i];
            t_i--; r_i--;
        }
    }
    for (m_i = 0; m_i < local_N; m_i++)
        my_list[m_i] = temp_list[m_i];
}

void printArray(int arr[], int size) { 
    int i; 
    for (i=0; i < size; i++) 
        printf("%d ", arr[i]); 
    printf("\n"); 
} 

int main(int argc, char* argv[]) {

    int *local_list;
    int *temp_B;
    int *temp_C;
    int my_rank, n_p, i, phase, partner;

    MPI_Init(&argc, &argv);

    /*GET n_p and my_rank*/
    MPI_Comm_size(MPI_COMM_WORLD, &n_p);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    local_list = (int*)malloc(5*sizeof(int));
    //srand(my_rank);
    for (i = 0; i < 5; i++) {
        //local_list[i] = rand()%20;
        local_list[i] = i;
    }

    temp_B = malloc(5*sizeof(int));
    temp_C = malloc(5*sizeof(int));
    MPI_Status status;
    for (phase = 0; phase < n_p; phase++ ) {
        partner = compute_partner(phase, my_rank, n_p);
        if (partner < 0 || partner >= n_p) continue;

        MPI_Sendrecv(local_list, 5, MPI_INT, partner, 0,
                temp_B, 5, MPI_INT, partner, 0, MPI_COMM_WORLD,
                &status);
        if (1) {
            if (my_rank < partner) {
                mergelow(local_list, temp_B, temp_C, 5);
            } else {
                mergehigh(local_list, temp_B, temp_C, 5);
            }
        }
    }

    printf("%d>(%d) Sorted Array: ", my_rank, partner);
    printArray(local_list, 5);

    free(temp_C);
    free(temp_B);
    free(local_list);


    MPI_Finalize();
}