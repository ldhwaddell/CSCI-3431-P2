#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <semaphore.h>

#define NUM_DIRECTIONS 4

int writeF(pid)
{
    FILE *file = fopen("matrix.txt", "w");
    int status;
    if (file == NULL)
    {
        return 1;
    }
    for (int i = 0; i < 5; i++)
    {
        fprintf(file, "%d  %d\n", pid, i);
    }

    fclose(file);

    return 0;
}

int readMatrix(int numBuses, int matrix[][NUM_DIRECTIONS])
{
    FILE *file = fopen("matrix.txt", "r");
    int status;
    if (file == NULL)
    {
        return 1;
    }

    for (int i = 0; i < numBuses; i++)
    {
        fscanf(file, "%d %d %d %d", &matrix[i][0], &matrix[i][1], &matrix[i][2], &matrix[i][3]);
    }

    fclose(file);

    return 0;
}

int main(int argc, char *argv[])
{

    // Declare variables
    pid_t pid;
    int curr_direction, numBuses;
    char *directions[] = {"North", "West", "South", "East"};
    sem_t *sem_directions[4];
    sem_t *semEditMatrix, *semJunction;

    // Get bus id
    pid = atoi(argv[7]);

    // Get number of buses to create the matrix with
    numBuses = atoi(argv[8]);

    // Get bus direction
    curr_direction = atoi(argv[9]);

    // Find right semaphore direction
    int right = (curr_direction + 1) % NUM_DIRECTIONS;

    // Initialize the named semaphores
    semEditMatrix = sem_open(argv[1], 0);
    semJunction = sem_open(argv[2], 0);

    if (semEditMatrix == SEM_FAILED)
    {
        printf("[Error]: Could not create create 'semEditMatrix'\n");
        exit(EXIT_FAILURE);
    }

    if (semJunction == SEM_FAILED)
    {
        printf("[Error]: Could not create create 'semJunction'\n");
        exit(EXIT_FAILURE);
    }

    // Assign each of the directional semaphores to a spot in the sem_directions array
    for (int i = 3; i < 7; i++)
    {
        sem_directions[i - 3] = sem_open(argv[i], 0);
        if (sem_directions[i - 3] == SEM_FAILED)
        {
            printf("[Error]: Could not create semaphore: %s\n", argv[i]);
            exit(EXIT_FAILURE);
        }
    }

    // Let user know curr_direction bus has started
    printf("Bus <%d>: %s bus started\n", pid, directions[curr_direction]);

    // Request semaphore for current direction
    printf("Bus <%d>: Requests for %s lock\n", pid, directions[curr_direction]);
    if (sem_wait(sem_directions[curr_direction]) == -1)
    {
        printf("[Error]: Bus <%d> could not request %s semaphore\n", pid, directions[curr_direction]);
        exit(EXIT_FAILURE);
    }

    printf("Bus <%d>: Acquires %s lock\n", pid, directions[curr_direction]);

    // Request semaphore for right direction
    printf("Bus <%d>: Requests for %s lock\n", pid, directions[right]);
    if (sem_wait(sem_directions[right]) == -1)
    {
        printf("[Error]: Bus <%d> could not request %s semaphore\n", pid, directions[right]);
        exit(EXIT_FAILURE);
    }

    printf("Bus <%d>: Acquires %s lock\n", pid, directions[right]);

    // Request semaphore for junction
    printf("Bus <%d>: Requests Junction lock\n", pid);
    if (sem_wait(semJunction) == -1)
    {
        printf("[Error]: Bus <%d> could not request Junction semaphore\n", pid);
        exit(EXIT_FAILURE);
    }

    printf("Bus <%d>: Acquires Junction lock; Passing Junction;\n", pid);
    sleep(2);

    // Release Junction lock
    if (sem_post(semJunction) == -1)
    {
        printf("[Error]: Bus <%d> could not post Junction semaphore\n", pid);
        exit(EXIT_FAILURE);
    }

    printf("Bus <%d>: Releases Junction lock\n", pid);

    // Release current direction lock
    if (sem_post(sem_directions[curr_direction]) == -1)
    {
        printf("[Error]: Bus <%d> could not post %s semaphore\n", pid, directions[curr_direction]);
        exit(EXIT_FAILURE);
    }

    printf("Bus <%d>: Releases %s lock\n", pid, directions[curr_direction]);

    // Release right direction lock
    if (sem_post(sem_directions[right]) == -1)
    {
        printf("[Error]: Bus <%d> could not post %s semaphore\n", pid, directions[right]);
        exit(EXIT_FAILURE);
    }

    printf("Bus <%d>: Releases %s lock\n", pid, directions[right]);

    // Close the semaphore
    if (sem_close(sem_directions[curr_direction]) == -1)
    {
        printf("[Error]: Bus <%d> could not post %s semaphore\n", pid, directions[curr_direction]);
        perror("sem_close");
        exit(EXIT_FAILURE);
    }

    return 0;
}
