/*
 *
 *
 * \Author : Mohammad Derhami
 *
 *
 * \ OS Project1
 *
 *
 *
 * Child Process Worker
 * Processes a segment of numbers and calculates local max and average
 * Stores results in shared memory for parent process
 */
#include "../include/child.hpp"


int main(int argc, char *argv[])
{
    /* Validate command line arguments */
    if (argc != 6)
    {
        std::cerr << "Child: Invalid number of arguments" << std::endl;
        return 1;
    }

    /* Parse arguments */
    int start_index = atoi(argv[1]);
    int end_index = atoi(argv[2]);
    int shmid_numbers = atoi(argv[3]);
    int shmid_results = atoi(argv[4]);
    int child_index = atoi(argv[5]);

    /* Attach to shared memory segments */
    int *numbers = (int *)shmat(shmid_numbers, NULL, 0);
    double *results = (double *)shmat(shmid_results, NULL, 0);

    if (numbers == (int *)-1 || results == (double *)-1)
    {
        std::cerr << "Child " << child_index << ": Failed to attach shared memory" << std::endl;
        return 1;
    }

    /* Calculate local max and average for assigned segment */
    int local_max = INT_MIN;
    double sum = 0.0;
    int count = end_index - start_index + 1;

    /* Process assigned numbers */
    for (int i = start_index; i <= end_index; i++)
    {
        if (numbers[i] > local_max)
        {
            local_max = numbers[i];
        }
        sum += numbers[i];
    }

    double local_avg = sum / count;

    /* Store results in shared memory */
    results[child_index * 2] = local_max;      /* local max */
    results[child_index * 2 + 1] = local_avg;  /* local average */

    /* Detach from shared memory */
    shmdt(numbers);
    shmdt(results);

    return 0;
}
