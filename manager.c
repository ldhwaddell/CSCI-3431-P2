#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <semaphore.h>

#define NUM_DIRECTIONS 4

char *readSequence(char *fileName)
{
    FILE *file = fopen(fileName, "r");
    char *sequence;
    size_t n = 0;
    int c;

    if (file == NULL)
    {
        return NULL;
    }
    // Detemine how much memory needs to be allocated to hold the sequence
    fseek(file, 0, SEEK_END);
    long f_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    sequence = malloc(f_size);

    // Read characters while EOF has not been reached
    while ((c = fgetc(file)) != EOF)
    {
        sequence[n++] = (char)c;
    }

    // Add termination character to end of sequence
    sequence[n] = '\0';

    return sequence;
}

int writeMatrix(int numBuses, int arr[][NUM_DIRECTIONS])
{
    FILE *file = fopen("matrix.txt", "w");
    int status;
    if (file == NULL)
    {
        return 1;
    }
    for (int i = 0; i < numBuses; i++)
    {
        for (int j = 0; j < NUM_DIRECTIONS; j++)
        {
            // Set every array value to 0
            arr[i][j] = 0;
            // Write it to file, checking for success
            status = fprintf(file, "%d ", arr[i][j]);
            if (status < 0)
            {
                return 2;
            }
        }
        // Write newline
        status = fprintf(file, "\n");
        if (status < 0)
        {
            return 2;
        }
    }
    fclose(file);

    return 0;
}

int updateMatrix(int x, int y, int newValue, int numBuses, int arr[][NUM_DIRECTIONS])
{
    FILE *file = fopen("matrix.txt", "r+");
    int status;
    if (file == NULL)
    {
        return 1;
    }

    // Validation to ensure valid coordinate to update gets entered
    if (x > numBuses || x < 0)
    {
        return 3;
    }
    else if (y > NUM_DIRECTIONS || y < 0)
    {
        return 4;
    }

    // Update matrix value
    arr[x][y] = newValue;

    for (int i = 0; i < numBuses; i++)
    {
        for (int j = 0; j < NUM_DIRECTIONS; j++)
        {
            // Write new matrix to file, checking for success
            status = fprintf(file, "%d ", arr[i][j]);
            if (status < 0)
            {
                return 2;
            }
        }
        // Write newline
        status = fprintf(file, "\n");
        if (status < 0)
        {
            return 2;
        }
    }
    fclose(file);

    return 0;
}

// Function to "randomly" generate a double in range 0-1
double getRandom()
{
    return (rand() % RAND_MAX) / (double)RAND_MAX;
}

int checkDeadlock()
{

    return 0;
}

int main(int argc, char *argv[])
{
    // Declare variables
    char *sequence;
    int matrix_write_status;
    double p, r;
    int direction, deadlock = 0, allBusesPassed = 0;

    // Initialize a seed for random number generation
    srand(getpid());

    // Get and validate input for p from command line
    if (argc != 2)
    {
        printf("[Error]: Must enter a probability\n");
        exit(1);
    }

    p = atof(argv[1]);
    if (p < 0.2 || p > 0.7)
    {
        printf("[Error]: p must be in range [0.2, 0.7]\n");
        exit(1);
    }

    // Try to read input from sequence.txt file
    printf("Reading input from file sequence.txt\n");
    sequence = readSequence("sequence.txt");
    if (sequence == NULL)
    {
        printf("[Error]: Could not open file.\n");
        exit(1);
    }
    else if (*sequence == '\0')
    {
        printf("[Error]: Sequence.txt apprears to be empty.\n");
        exit(1);
    }

    printf("Sequence found: %s\nStarting buses:\n\n", sequence);

    // Declare number of buses to create
    int numBuses = strlen(sequence);

    // Declare matrix
    int matrix[numBuses][NUM_DIRECTIONS];

    // Write all 0's to matrix
    switch (writeMatrix(numBuses, matrix))
    {
    case 1:
        printf("[Error]: Could not open matrix.txt\n");
        exit(1);
    case 2:
        printf("[Error]: Could not write to file matrix.txt\n");
        exit(1);
    }

    // Start sending buses
    pid_t pid;
    char pid_str[20];
    char direction_str[2];
    char numBuses_str[100];
    int busID = 0;

    // ---------------------SEMAPHORE CREATION---------------------

    // Semaphore for controlling writing to matrix

    // 0_CREATE: only create semaphore if one doesnt already exist
    // 0644: read and write permission to user who created, read permission to group
    // 1: initial semaphore value
    sem_t *semEditMatrix = sem_open("/semEditMatrix", O_CREAT | O_EXCL, 0644, 1);
    sem_t *semJunction = sem_open("/semJunction", O_CREAT | O_EXCL, 0644, 1);
    sem_t *semNorth = sem_open("/semNorth", O_CREAT | O_EXCL, 0644, 1);
    sem_t *semWest = sem_open("/semWest", O_CREAT | O_EXCL, 0644, 1);
    sem_t *semSouth = sem_open("/semSouth", O_CREAT | O_EXCL, 0644, 1);
    sem_t *semEast = sem_open("/semEast", O_CREAT | O_EXCL, 0644, 1);

    char *args[11];
    args[0] = "./bus";
    args[1] = "/semEditMatrix";
    args[2] = "/semJunction";
    args[3] = "/semNorth";
    args[4] = "/semWest";
    args[5] = "/semSouth";
    args[6] = "/semEast";

    // ------------------------------------------------------------

    while (deadlock != 1 && allBusesPassed != 1)
    {
        if (busID < numBuses)
        {
            r = getRandom();

            if (r < p)
            {
                deadlock = checkDeadlock();
                printf("locking\n");
                // deadlock = 1;
            }
            else
            {
                // Increment busID as a new bus is now being made
                busID++;

                // Make child and run the bus
                pid = fork();
                if (pid < 0)
                {
                    printf("[Error]: Unsuccessful fork to create bus %c at sequence index %d. Program terminating.\n", sequence[busID], busID);
                    exit(1);
                }
                else if (pid == 0)
                {

                    // Convert direction to an integer to bus program can reference direction/semaphores from array
                    if (sequence[busID - 1] == 'N')
                    {
                        direction = 0;
                    }
                    else if (sequence[busID - 1] == 'W')
                    {
                        direction = 1;
                    }
                    else if (sequence[busID - 1] == 'S')
                    {
                        direction = 2;
                    }
                    else if (sequence[busID - 1] == 'E')
                    {
                        direction = 3;
                    }

                    // Create command line args to send to bus program
                    // Convert pid, direction to a char to send as command line argument
                    // snprintf(pid_str, sizeof(pid_str), "%d", getpid());
                    snprintf(pid_str, sizeof(pid_str), "%d", busID);
                    snprintf(numBuses_str, sizeof(numBuses_str), "%d", numBuses);
                    sprintf(direction_str, "%d", direction);

                    args[7] = pid_str;
                    args[8] = numBuses_str;
                    args[9] = direction_str;
                    args[10] = NULL;

                    execvp(args[0], args);
                    break;
                }
            }
        }
        else
        {
            // Break loop if deadlock
            deadlock = checkDeadlock();
            printf("here\n");
            // allBusesPassed = 1;
            for (int i = 0; i < numBuses; i++)
            {
                wait(NULL);
            }

            // int test = updateMatrix(1, 3, 9, rows, matrix);

            // Close and unlink all semaphores

            // ADD ERR CHECK
            sem_close(semEditMatrix);
            sem_close(semJunction);
            sem_close(semNorth);
            sem_close(semWest);
            sem_close(semSouth);
            sem_close(semEast);
            for (int i = 1; i < 7; i++)
            {
                sem_unlink(args[i]);
            }
        }
        sleep(1);
    }

    // If deadlock detected print out the cycle to user
    if (deadlock)
    {
        printf("\nSystem Deadlocked!\nThe cycle below was detected:\n\n");
        printf("pisss");
    }
    else if (allBusesPassed)
    {
        printf("\nSuccess! All buses passed the junction without a deadlock occurring.\n");
        printf("This working sequence was: %s\n", sequence);
    }

    for (int i = 0; i < numBuses; i++)
    {
        wait(NULL);
    }

    // int test = updateMatrix(1, 3, 9, rows, matrix);

    // Close and unlink all semaphores

    // ADD ERR CHECK
    sem_close(semEditMatrix);
    sem_close(semJunction);
    sem_close(semNorth);
    sem_close(semWest);
    sem_close(semSouth);
    sem_close(semEast);
    for (int i = 1; i < 7; i++)
    {
        sem_unlink(args[i]);
    }

    return 0;
}