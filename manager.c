#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <semaphore.h>

#define NUM_DIRECTIONS 4
#define INITIAL 1
#define VISITED 2
#define FINISHED 3

char *readSequence()
{
    FILE *file = fopen("sequence.txt", "r");
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

void writeMatrix(int numBuses, int arr[][NUM_DIRECTIONS])
{
    FILE *file = fopen("matrix.txt", "w");
    int status;
    if (file == NULL)
    {
        printf("[Error]: Could not open matrix.txt\n");
        exit(EXIT_FAILURE);
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
                printf("[Error]: Could not write to file matrix.txt\n");
                exit(EXIT_FAILURE);
            }
        }
        // Write newline
        status = fprintf(file, "\n");
        if (status < 0)
        {
            printf("[Error]: Could not write to file matrix.txt\n");
            exit(EXIT_FAILURE);
        }
    }
    fclose(file);
}

void waitSemaphore(sem_t *sem)
{
    if (sem_wait(sem) == -1)
    {
        printf("[Error]: Manager could not request editMatrix semaphore\n");
        exit(EXIT_FAILURE);
    }
}

void postSemaphore(sem_t *sem)
{
    if (sem_post(sem) == -1)
    {

        printf("[Error]: Manager could not release editMatrix semaphore\n");
        exit(EXIT_FAILURE);
    }
}

// Function to "randomly" generate a double in range 0-1
double getRandom()
{
    return (rand() % RAND_MAX) / (double)RAND_MAX;
}

/*
 * Function: readMatrix
 * --------------------
 * Read a numBuses X num_directions matrix into arr
 *
 *  numBuses: The number of buses(rows) in the matrix
 *  arr: The array to save the values in to
 */
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

void createGraph(int numBuses, int matrix[][NUM_DIRECTIONS], int adj_matrix[][numBuses + 4])
{

    // Read current values from matrix.txt into curr_matrix
    readMatrix(numBuses, matrix);

    // Iterate over each row and column in adj_matrix and fill with 0's
    for (int i = 0; i < numBuses + 4; i++)
    {
        for (int j = 0; j < numBuses+4; j++)
        {
            adj_matrix[i][j] = 0;
        }
    }

    // Create adjaceny matrix by iterating over matrix.txt values
    for (int i = 0; i < numBuses; i++)
    {
        for (int j = 0; j < NUM_DIRECTIONS; j++)
        {
            if (matrix[i][j] == 2)
            {
                // Add an edge in
                printf("Value at (%d, %d) is 2, adding to adj: (%d, %d)\n", i, j, j, (i + 4) % numBuses+4);
                adj_matrix[j][(i + 4)] = 1;
            }
            else if (matrix[i][j] == 1)
            {
                printf("Value at (%d, %d) is 1, adding to adj: (%d, %d)\n", i, j, (i + 4) % numBuses+4, j);
                adj_matrix[(i + 4)][j] = 1;
            }
        }
    }

    for (int i = 0; i < numBuses + 4; i++)
    {
        for (int j = 0; j < numBuses+4; j++)
        {
            printf("%d ", adj_matrix[i][j]);
        }
        printf("\n");
    }
}

int checkDeadlock(int numBuses, int adj_matrix[][numBuses + 4])
{
    int vertices = numBuses + 4;
    int states[vertices];

    for (int v = 0; v < vertices; v++)
    {
        states[v] = INITIAL;
    }

    // start the DFS at vertex 0

    // Need a way of keeping track of everything that has been seen

    // Everytime it sees a node, add it to list of ones that have been seen.

    // If cycledetected, then print out that list in order?

    return 0;
}

void closeSemaphore(sem_t *sem, char *name)
{
    if (sem_close(sem) == -1)
    {
        printf("[Error]: Manager could not close the %s semaphore\n", name);
        exit(EXIT_FAILURE);
    }
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
        exit(EXIT_FAILURE);
    }

    p = atof(argv[1]);
    if (p < 0.2 || p > 0.7)
    {
        printf("[Error]: p must be in range [0.2, 0.7]\n");
        exit(EXIT_FAILURE);
    }

    // Try to read input from sequence.txt file
    printf("Reading input from file sequence.txt\n");
    sequence = readSequence();
    if (sequence == NULL)
    {
        printf("[Error]: Could not open file.\n");
        exit(EXIT_FAILURE);
    }
    else if (*sequence == '\0')
    {
        printf("[Error]: Sequence.txt apprears to be empty.\n");
        exit(EXIT_FAILURE);
    }

    printf("Sequence found: %s\nStarting buses:\n\n", sequence);

    // Declare number of buses to create
    int numBuses = strlen(sequence);

    // Declare matrix
    int matrix[numBuses][NUM_DIRECTIONS];

    // Declare adjacency matrix
    int adj_matrix[numBuses + 4][numBuses + 4];

    // Write all 0's to matrix
    writeMatrix(numBuses, matrix);

    // createGraph(numBuses, matrix, adj_matrix);

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
                waitSemaphore(semEditMatrix);
                printf("Manager waiting for semEditMatrix\n");
                createGraph(numBuses, matrix, adj_matrix);
                deadlock = checkDeadlock(numBuses, adj_matrix);
                printf("Manager releasing semEditMatrix\n");
                postSemaphore(semEditMatrix);
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
                    exit(EXIT_FAILURE);
                }
                else if (pid == 0)
                {

                    // Convert direction to an integer so bus program can reference direction/semaphores from array
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

                    // Convert pid, direction to a char to send as command line argument
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
            waitSemaphore(semEditMatrix);
            printf("Manager waiting for semEditMatrix\n");
            createGraph(numBuses, matrix, adj_matrix);
            deadlock = checkDeadlock(numBuses, adj_matrix);
            printf("Manager releasing semEditMatrix\n");
            postSemaphore(semEditMatrix);
            printf("here\n");
            // allBusesPassed = 1;
            deadlock = 1;
            // sleep(1);
        }
        sleep(1);
    }

    // Ensure all children are finished
    for (int i = 0; i < numBuses; i++)
    {
        wait(NULL);
    }

    // If deadlock detected print out the cycle to user
    if (deadlock)
    {
        printf("\nSystem Deadlocked!\nThe cycle below was detected:\n\n");
    }
    else if (allBusesPassed)
    {
        printf("\nSuccess! All buses passed the junction without a deadlock occurring.\n");
        printf("This working sequence was: %s\n", sequence);
    }

    // Close and unlink all semaphores
    closeSemaphore(semEditMatrix, "editMatrix");
    closeSemaphore(semJunction, "junction");
    closeSemaphore(semNorth, "north");
    closeSemaphore(semWest, "west");
    closeSemaphore(semSouth, "south");
    closeSemaphore(semEast, "east");
    for (int i = 1; i < 7; i++)
    {
        sem_unlink(args[i]);
    }

    return 0;
}
