/*File to represent a bus attempting to cross the junction*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <semaphore.h>
#include <string.h>
#include <signal.h>

#define NUM_DIRECTIONS 4

sem_t *sem_directions[4];
sem_t *semEditMatrix, *semJunction;

/*
 * Function: sigtermHandler
 * --------------------
 * Function to ensure that semaphores can gracefully
 * exit in the event of but process receiving SIGTERM
 */
void sigtermHandler()
{
    for (int i = 0; i < 4; i++)
    {
        sem_close(sem_directions[i]);
    }
    sem_close(semEditMatrix);
    sem_close(semJunction);
    exit(EXIT_SUCCESS);
}

/*
 * Function: readMatrix
 * --------------------
 * Read a numBuses X num_directions matrix into variable matrix
 *
 *  numBuses: The number of buses(rows) in the matrix
 *  matrix: The matrix to save the values in to
 */
void readMatrix(int numBuses, int matrix[][NUM_DIRECTIONS])
{
    FILE *file = fopen("matrix.txt", "r");
    int status;
    if (file == NULL)
    {
        exit(EXIT_FAILURE);
    }

    // Read in values from matrix.txt into matrix
    for (int i = 0; i < numBuses; i++)
    {
        fscanf(file, "%d %d %d %d", &matrix[i][0], &matrix[i][1], &matrix[i][2], &matrix[i][3]);
    }

    fclose(file);
}

/*
 * Function: updateMatrix
 * --------------------
 * Update a value at matrix[rol][col] to newVal.
 *
 *  row: The row to update
 *  col: The column to update
 *  newVal: The new value to input
 *  numBuses: The number of buses(rows) in the matrix
 *  matrix: The matrix to save the values in to
 */
void updateMatrix(int row, int col, int newVal, int numBuses, int matrix[][NUM_DIRECTIONS])
{
    // Read current matrix values into matrix
    readMatrix(numBuses, matrix);

    // Open the file in write mode so file does not get appended to
    FILE *file = fopen("matrix.txt", "w");
    int status;
    if (file == NULL)
    {
        exit(EXIT_FAILURE);
    }

    // Validation to ensure valid coordinate to update gets received
    if (row > numBuses || row < 0)
    {
        printf("[Error]: %d is not a valid row to edit\n", row);
        exit(EXIT_FAILURE);
    }
    else if (col > NUM_DIRECTIONS || col < 0)
    {
        printf("[Error]: %d is not a valid column to edit\n", col);
        exit(EXIT_FAILURE);
    }

    // Update desired value
    matrix[row][col] = newVal;

    // Write new matrix back to matrix.txt
    for (int i = 0; i < numBuses; i++)
    {
        for (int j = 0; j < NUM_DIRECTIONS; j++)
        {
            // Write it to file, checking for success
            status = fprintf(file, "%d ", matrix[i][j]);
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
}

/*
 * Function: waitSemaphore
 * --------------------
 * Wrapper for sem_wait that handles error checking
 * with useful error messages
 *
 *  sem: The semaphore to call sem_wait on
 *  pid: The bus process that is requesting to do this
 *  direction: The direction or name of the semaphore
 */
void waitSemaphore(sem_t *sem, int pid, char *direction)
{
    if (sem_wait(sem) == -1)
    {
        if (direction == NULL || *direction == '\0')
        {
            printf("[Error]: Bus <%d> could not request Junction semaphore\n", pid);
        }
        else if (strcmp(direction, "edit"))
        {
            printf("[Error]: Bus <%d> could not request the editMatrix semaphore\n", pid);
        }
        else if (strcmp(direction, "junction"))
        {
            printf("[Error]: Bus <%d> could not request the Junction semaphore\n", pid);
        }
        else
        {
            printf("[Error]: Bus <%d> could not request %s semaphore\n", pid, direction);
        }
        exit(EXIT_FAILURE);
    }
}

/*
 * Function: postSemaphore
 * --------------------
 * Wrapper for sem_post that handles error checking
 * with useful error messages
 *
 *  sem: The semaphore to call sem_post on
 *  pid: The bus process that is requesting to do this
 *  direction: The direction or name of the semaphore
 */
void postSemaphore(sem_t *sem, int pid, char *direction)
{
    if (sem_post(sem) == -1)
    {
        if (direction == NULL || *direction == '\0')
        {
            printf("[Error]: Bus <%d> could not post Junction semaphore\n", pid);
        }
        else if (strcmp(direction, "edit"))
        {
            printf("[Error]: Bus <%d> could not post the editMatrix semaphore\n", pid);
        }
        else if (strcmp(direction, "junction"))
        {
            printf("[Error]: Bus <%d> could not post the Junction semaphore\n", pid);
        }
        else
        {
            printf("[Error]: Bus <%d> could not post %s semaphore\n", pid, direction);
        }
        exit(EXIT_FAILURE);
    }
}

/*
 * Function: closeSemaphore
 * --------------------
 * Wrapper for sem_close that handles error checking
 * with useful error messages
 *
 *  sem: The semaphore to call sem_close on
 *  pid: The bus process that is requesting to do this
 *  direction: The direction or name of the semaphore
 */
void closeSemaphore(sem_t *sem, int pid, char *direction)
{
    if (sem_close(sem) == -1)
    {
        if (direction == NULL || *direction == '\0')
        {
            printf("[Error]: Bus <%d> could not close Junction semaphore\n", pid);
        }
        else if (strcmp(direction, "edit"))
        {
            printf("[Error]: Bus <%d> could not close the editMatrix semaphore\n", pid);
        }
        else if (strcmp(direction, "junction"))
        {
            printf("[Error]: Bus <%d> could not close the Junction semaphore\n", pid);
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
    // Create signal handler to always close semaphores if
    // the bus process gets killed in event of deadlock
    signal(SIGTERM, sigtermHandler);

    // Declare variables
    pid_t pid;
    int curr_direction, numBuses, right;
    char *directions[] = {"North", "West", "South", "East"};

    // Get bus id
    pid = atoi(argv[7]);

    // Get number of buses to create the matrix with
    numBuses = atoi(argv[8]);

    // Get bus direction
    curr_direction = atoi(argv[9]);

    // Find right semaphore direction
    right = (curr_direction + 1) % NUM_DIRECTIONS;

    // Create matrix to hold current semaphore values
    int matrix[numBuses][NUM_DIRECTIONS];

    // Initialize named semaphores
    semEditMatrix = sem_open(argv[1], 0);
    semJunction = sem_open(argv[2], 0);

    // Handle error checking
    if (semEditMatrix == SEM_FAILED)
    {
        printf("[Error]: Bus <%d> could not create 'semEditMatrix'\n", pid);
        exit(EXIT_FAILURE);
    }

    if (semJunction == SEM_FAILED)
    {
        printf("[Error]: Bus <%d> could not create 'semJunction'\n", pid);
        exit(EXIT_FAILURE);
    }

    // Assign each of the directional semaphores to a spot in the sem_directions matrixay
    for (int i = 3; i < 7; i++)
    {
        sem_directions[i - 3] = sem_open(argv[i], 0);
        // Handle error eching if semaphore fails
        if (sem_directions[i - 3] == SEM_FAILED)
        {
            printf("[Error]: Bus <%d> could not create semaphore '%s'\n", pid, argv[i]);

            exit(EXIT_FAILURE);
        }
    }

    // Let user know curr_direction bus has started
    printf("Bus <%d>: %s bus started\n", pid, directions[curr_direction]);

    // Request semaphore for current direction
    printf("Bus <%d>: Requests for %s lock\n", pid, directions[curr_direction]);

    // Update matrix to reflect request for current direction semaphore
    waitSemaphore(semEditMatrix, pid, "edit");
    updateMatrix((pid - 1), curr_direction, 1, numBuses, matrix);
    postSemaphore(semEditMatrix, pid, "edit");

    waitSemaphore(sem_directions[curr_direction], pid, directions[curr_direction]);
    printf("Bus <%d>: Acquires %s lock\n", pid, directions[curr_direction]);

    // Update matrix to reflect acquisition of current direction semaphore
    waitSemaphore(semEditMatrix, pid, "edit");
    updateMatrix((pid - 1), curr_direction, 2, numBuses, matrix);
    postSemaphore(semEditMatrix, pid, "edit");

    // Request semaphore for right direction
    printf("Bus <%d>: Requests for %s lock\n", pid, directions[right]);

    // Update matrix to reflect request for right direction semaphore
    waitSemaphore(semEditMatrix, pid, "edit");
    updateMatrix((pid - 1), right, 1, numBuses, matrix);
    postSemaphore(semEditMatrix, pid, "edit");

    waitSemaphore(sem_directions[right], pid, directions[right]);
    printf("Bus <%d>: Acquires %s lock\n", pid, directions[right]);

    // Update matrix to reflect acquisition of right direction semaphore
    waitSemaphore(semEditMatrix, pid, "edit");
    updateMatrix((pid - 1), right, 2, numBuses, matrix);
    postSemaphore(semEditMatrix, pid, "edit");

    // Request semaphore for junction
    printf("Bus <%d>: Requests Junction lock\n", pid);
    waitSemaphore(semJunction, pid, "junction");

    // Pass junction
    printf("Bus <%d>: Acquires Junction lock; Passing Junction;\n", pid);
    sleep(2);

    // Release Junction lock
    postSemaphore(semJunction, pid, "junction");
    printf("Bus <%d>: Releases Junction lock\n", pid);

    // Release current direction lock
    postSemaphore(sem_directions[curr_direction], pid, directions[curr_direction]);
    printf("Bus <%d>: Releases %s lock\n", pid, directions[curr_direction]);

    // Update matrix to reflect release of current direction
    waitSemaphore(semEditMatrix, pid, "edit");
    updateMatrix((pid - 1), curr_direction, 0, numBuses, matrix);
    postSemaphore(semEditMatrix, pid, "edit");

    // Release right direction lock
    postSemaphore(sem_directions[right], pid, directions[right]);
    printf("Bus <%d>: Releases %s lock\n", pid, directions[right]);

    // Update matrix to reflect release of right direction
    waitSemaphore(semEditMatrix, pid, "edit");
    updateMatrix((pid - 1), right, 0, numBuses, matrix);
    postSemaphore(semEditMatrix, pid, "edit");

    // Close the semaphores
    closeSemaphore(sem_directions[curr_direction], pid, directions[curr_direction]);
    closeSemaphore(sem_directions[right], pid, directions[right]);
    closeSemaphore(semJunction, pid, "junction");
    closeSemaphore(semEditMatrix, pid, "edit");

    return 0;
}
