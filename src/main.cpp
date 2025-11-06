#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <ctime>
#include <cmath>
#include <climits>


int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        std::cerr << "Usage: " << argv[0] << " <number_of_processes> <filename>" << std::endl;
        return 1;
    }

    int num_processes = atoi(argv[1]);
    std::string filename = argv[2];

    if (num_processes <= 0)
    {
        std::cerr << "Error: Number of processes must be positive" << std::endl;
        return 1;
    }

    std::ifstream file(filename);
    if (!file.is_open())
    {
        std::cerr << "Error: Cannot open file " << filename << std::endl;
        return 1;
    }

    int total_numbers;
    file >> total_numbers;

    if (total_numbers <= 0)
    {
        std::cerr << "Error: Total numbers must be positive" << std::endl;
        return 1;
    }

    std::vector<int> numbers(total_numbers);
    for (int i = 0; i < total_numbers; i++)
    {
        file >> numbers[i];
    }
    file.close();

    key_t shared_numbers_key = 1234;
    int shmid_numbers = shmget(shared_numbers_key, total_numbers * sizeof(int), IPC_CREAT | 0666);
    if (shmid_numbers == -1)
    {
        perror("shmget numbers failed");
        return 1;
    }

    int *shared_numbers = (int *)shmat(shmid_numbers, NULL, 0);
    if (shared_numbers == (int *)-1)
    {
	    std::perror("shmat numbers failed");
        return 1;
    }

    memcpy(shared_numbers, numbers.data(), total_numbers * sizeof(int));

    key_t shared_results_key = 2345;
    int shmid_results = shmget(shared_results_key, num_processes * 2 * sizeof(double), IPC_CREAT | 0666);
    if (shmid_results == -1)
    {
	    std::perror("shmget results failed");
        shmdt(shared_numbers);
        return 1;
    }

    double *shared_results = (double *)shmat(shmid_results, NULL, 0);
    if (shared_results == (double *)-1)
    {
	std::perror("shmat results failed");
        shmdt(shared_numbers);
        return 1;
    }

    for (int i = 0; i < num_processes; i++)
    {
        shared_results[i * 2] = 0.0;     
        shared_results[i * 2 + 1] = 0.0; 
    }

    int base_size = total_numbers / num_processes;
    int remainder = total_numbers % num_processes;

    clock_t start = clock();

    for (int i = 0; i < num_processes; i++)
    {
        pid_t pid = fork();

        if (pid == 0) 
        {
            int start_index = i * base_size;
            int end_index = (i + 1) * base_size - 1;
            
            if (i == num_processes - 1)
            {
                end_index += remainder;
            }

	    std::string s_start = std::to_string(start_index);
            std::string s_end = std::to_string(end_index);
	    std::string s_shmid_numbers = std::to_string(shmid_numbers);
	    std::string s_shmid_results = std::to_string(shmid_results);
	    std::string child_index = std::to_string(i);

            execl("./child", "child", s_start.c_str(), s_end.c_str(), 
                  s_shmid_numbers.c_str(), s_shmid_results.c_str(), 
                  child_index.c_str(), NULL);

            std::cerr << "exec failed for child process " << i << std::endl;
            exit(1);
        }
        else if (pid < 0)
        {
            std::cerr << "fork failed for process " << i << std::endl;
        }
    }

    for (int i = 0; i < num_processes; i++)
    {
        wait(NULL);
    }

    double global_max = -INFINITY;
    double total_sum_avg = 0.0;

    for (int i = 0; i < num_processes; i++)
    {
        double process_max = shared_results[i * 2];
        double process_avg = shared_results[i * 2 + 1];

        if (process_max > global_max)
        {
            global_max = process_max;
        }
        total_sum_avg += process_avg;
    }

    double global_avg = total_sum_avg / num_processes;
    clock_t end = clock();
    double execution_time = (double)(end - start) / CLOCKS_PER_SEC;

    std::cout << "\n=== Parallel Processing Results ===" << std::endl;
    std::cout << "Number of Processes: " << num_processes << std::endl;
    std::cout << "Total Numbers Processed: " << total_numbers << std::endl;
    std::cout << "Global Average: " << global_avg << std::endl;
    std::cout << "Global Maximum: " << global_max << std::endl;
    std::cout << "Execution Time: " << execution_time << " seconds\n\n" << std::endl;

    shmdt(shared_numbers);
    shmdt(shared_results);
    shmctl(shmid_numbers, IPC_RMID, NULL);
    shmctl(shmid_results, IPC_RMID, NULL);

    return 0;
}
