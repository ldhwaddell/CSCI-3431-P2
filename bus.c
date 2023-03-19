#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <semaphore.h>

#define NUM_DIRECTIONS 4

void readMatrix(int numBuses, int arr[][NUM_DIRECTIONS])
{
    FILE *file = fopen("matrix.txt", "r");
    int status;
    if (file == NULL)
    {
        exit(EXIT_FAILURE);
    }

    // Read in values from matrix.txt into arr
    for (int i = 0; i < numBuses; i++)
    {
        fscanf(file, "%d %d %d %d", &arr[i][0], &arr[i][1], &arr[i][2], &arr[i][3]);
    }

    fclose(file);
}

void updateMatrix(int row, int col, int newVal, int numBuses, int arr[][NUM_DIRECTIONS])
{
    // Read current matrix values into arr
    readMatrix(numBuses, arr);

    // Open the file in write mode so file does not get appended to
    FILE *file = fopen("matrix.txt", "w");
    int status;
    if (file == NULL)
    {
        exit(EXIT_FAILURE);
    }

    // Update desired value
    arr[row][col] = newVal;

    // Write new matrix back to matrix.txt
    for (int i = 0; i < numBuses; i++)
    {
        for (int j = 0; j < NUM_DIRECTIONS; j++)
        {
            // Write it to file, checking for success
            status = fprintf(file, "%d ", arr[i][j]);
            if (status < 0)
            {
                printf("[Error]: Could not write to matrix.txt\n");
                exit(EXIT_FAILURE);
            }
        }
        // Write newline
        status = fprintf(file, "\n");
        if (status < 0)
        {
            printf("[Error]: Could not write to matrix.txt\n");
            exit(EXIT_FAILURE);
        }
    }

    fclose(file);
    // printf("Setting (%d, %d) to %d\n", row, col, newVal);
    // sleep(1);
}

void waitSemaphore(sem_t *sem, int pid, char *direction)
{
    if (sem_wait(sem) == -1)
    {
        if (direction == NULL || *direction == '\0')
        {
            printf("[Error]: Bus <%d> could not request Junction semaphore\n", pid);
        }
        else
        {
            printf("[Error]: Bus <%d> could not request %s semaphore\n", pid, direction);
        }
        exit(EXIT_FAILURE);
    }
}

void postSemaphore(sem_t *sem, int pid, char *direction)
{
    if (sem_post(sem) == -1)
    {
        if (direction == NULL || *direction == '\0')
        {
            printf("[Error]: Bus <%d> could not post Junction semaphore\n", pid);
        }
        else
        {
            printf("[Error]: Bus <%d> could not post %s semaphore\n", pid, direction);
        }
        exit(EXIT_FAILURE);
    }
}

void closeSemaphore(sem_t *sem, int pid, char *direction)
{
    if (sem_post(sem) == -1)
    {
        if (direction == NULL || *direction == '\0')
        {
            printf("[Error]: Bus <%d> could not close Junction semaphore\n", pid);
        }
        else
        {
            printf("[Error]: Bus <%d> could not close %s semaphore\n", pid, direction);
        }
        exit(EXIT_FAILURE);
    }
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

    // Create matrix to hold current semaphore values
    int matrix[numBuses][NUM_DIRECTIONS];

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

    // Update matrix to reflect request
    // waitSemaphore(semEditMatrix, pid, NULL);
    // updateMatrix((pid - 1), curr_direction, 1, numBuses, matrix);
    // postSemaphore(semEditMatrix, pid, NULL);

    waitSemaphore(sem_directions[curr_direction], pid, directions[curr_direction]);
    printf("Bus <%d>: Acquires %s lock\n", pid, directions[curr_direction]);

    // Update matrix to reflect acquisition
    // waitSemaphore(semEditMatrix, pid, NULL);
    // updateMatrix((pid - 1), curr_direction, 2, numBuses, matrix);
    // postSemaphore(semEditMatrix, pid, NULL);

    // Request semaphore for right direction
    printf("Bus <%d>: Requests for %s lock\n", pid, directions[right]);

    // Update matrix to reflect request
    // waitSemaphore(semEditMatrix, pid, NULL);
    // updateMatrix((pid - 1), right, 1, numBuses, matrix);
    // postSemaphore(semEditMatrix, pid, NULL);

    waitSemaphore(sem_directions[right], pid, directions[right]);
    printf("Bus <%d>: Acquires %s lock\n", pid, directions[right]);

    // Update matrix to reflect acquisition
    // waitSemaphore(semEditMatrix, pid, NULL);
    // updateMatrix((pid - 1), right, 2, numBuses, matrix);
    // postSemaphore(semEditMatrix, pid, NULL);

    // Request semaphore for junction
    printf("Bus <%d>: Requests Junction lock\n", pid);
    waitSemaphore(semJunction, pid, NULL);
    printf("Bus <%d>: Acquires Junction lock; Passing Junction;\n", pid);
    sleep(2);

    // Release Junction lock
    printf("Bus <%d>: Releases Junction lock\n", pid);
    postSemaphore(semJunction, pid, NULL);

    // Release current direction lock
    printf("Bus <%d>: Releases %s lock\n", pid, directions[curr_direction]);
    postSemaphore(sem_directions[curr_direction], pid, directions[curr_direction]);

    // Update matrix to reflect release of current direction
    // waitSemaphore(semEditMatrix, pid, NULL);
    // updateMatrix((pid - 1), curr_direction, 0, numBuses, matrix);
    // postSemaphore(semEditMatrix, pid, NULL);

    // Release right direction lock
    printf("Bus <%d>: Releases %s lock\n", pid, directions[right]);
    postSemaphore(sem_directions[right], pid, directions[right]);

    // Update matrix to reflect release of right direction
    // waitSemaphore(semEditMatrix, pid, NULL);
    // updateMatrix((pid - 1), right, 0, numBuses, matrix);
    // postSemaphore(semEditMatrix, pid, NULL);

    // Close the semaphores
    closeSemaphore(sem_directions[curr_direction], pid, directions[curr_direction]);
    closeSemaphore(sem_directions[right], pid, directions[right]);
    closeSemaphore(semJunction, pid, NULL);
    closeSemaphore(semEditMatrix, pid, NULL);

    return 0;
}
