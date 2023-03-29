/* File to represent the manager of all of the bus processes*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <semaphore.h>
#include <signal.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/types.h>

#define O_EXCL 0x00000800
#define O_CREAT 0x00000200
#define NUM_DIRECTIONS 4
#define INITIAL 1
#define VISITED 2

/*
 * Function: readSequence
 * --------------------
 * Function to read the sequence from the file sequence.txt
 * Dynamically determines how much memory to give to sequence
 *
 *   returns: A '\0' terminated string of the sequence
 */
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

/*
 * Function: writeMatrix
 * --------------------
 * Function to open the file matrix.txt and fill the matrix with zeroes,
 * printing this zero filled matrix to matrix.txt
 *
 *  numBuses: The number of buses(rows) in the matrix
 *  matrix: The matrix to save the values in to
 */

void writeMatrix(int numBuses, int matrix[][NUM_DIRECTIONS])
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
            // Set every matrix value to 0
            matrix[i][j] = 0;
            // Write it to file, checking for success
            status = fprintf(file, "%d ", matrix[i][j]);
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

/*
 * Function: waitSemaphore
 * --------------------
 * Wrapper for sem_wait that handles error checking
 * with useful error messages
 *
 *  sem: The semaphore to call sem_wait on
 */
void waitSemaphore(sem_t *sem)
{
    if (sem_wait(sem) == -1)
    {
        printf("[Error]: Manager could not request editMatrix semaphore\n");
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
 */
void postSemaphore(sem_t *sem)
{
    if (sem_post(sem) == -1)
    {

        printf("[Error]: Manager could not release editMatrix semaphore\n");
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
 *  name: The name of the semaphore in text format
 */
void closeSemaphore(sem_t *sem, char *name)
{
    if (sem_close(sem) == -1)
    {
        printf("[Error]: Manager could not close the %s semaphore\n", name);
        exit(EXIT_FAILURE);
    }
}

/*
 * Function: getRandom
 * --------------------
 * Function to find a pseudorandom number inbetween 0-1,
 * found by dividing C's rand() modulu itself, divided by
 * C's RAND_MAX casted to a double
 *
 *   returns: a random number between 0 and 1
 */
double getRandom()
{
    return (rand() % RAND_MAX) / (double)RAND_MAX;
}

/*
 * Function: readMatrix
 * --------------------
 * Read a numBuses X num_directions matrix into matrix
 *
 *  numBuses: The number of buses(rows) in the matrix
 *  matrix: The matrix to save the values in to
 */
void readMatrix(int numBuses, int matrix[numBuses][NUM_DIRECTIONS])
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
 * Function: createGraph
 * --------------------
 * Creates the adjacency matrix at any given spot by reading
 * in values from matrix.txt, containing current info on what
 * buses control/request what semaphores
 *
 *  numBuses: The number of buses(rows) in the matrix
 *  nodes" The number of nodes that will be in the adjacency graph
 *  matrix: The matrix to save the values in to
 *  adj_matrix: The matrix that gets filled with adjacency values
 *
 */
void createGraph(int numBuses, int nodes, int matrix[numBuses][NUM_DIRECTIONS], int adj_matrix[nodes][nodes])
{

    // Read current values from matrix.txt into curr_matrix
    readMatrix(numBuses, matrix);

    // Iterate over each row and column in adj_matrix and fill with 0's
    for (int i = 0; i < nodes; i++)
    {
        for (int j = 0; j < nodes; j++)
        {
            adj_matrix[i][j] = 0;
        }
    }

    // Create adjacency matrix by iterating over matrix.txt values
    for (int i = 0; i < numBuses; i++)
    {
        for (int j = 0; j < NUM_DIRECTIONS; j++)
        {
            if (matrix[i][j] == 2)
            {
                // If value is a 2, bus is holding this nose
                adj_matrix[j][(i + 4)] = 1;
            }
            else if (matrix[i][j] == 1)
            {
                // If value is 1, but is requesting this node
                adj_matrix[(i + 4)][j] = 1;
            }
        }
    }
}

/*
 * Function: DFS
 * --------------------
 * Performs a depth-first-search on the node to check, and any resulting neighbours
 *
 *  checkNode: The node to visit and perform DFS on
 *  nodes: The number of nodes that will be in the adjacency graph
 *  states: A vector rembering if a given node has already been visited or not
 *  adj_matrix: The matrix that adjacency values get read from
 *
 *  returns: An integer 0, if DFS did not find anything, 1 if it did, or the next DFS recursive call
 */
int DFS(int checkNode, int nodes, int states[nodes], int adj_matrix[nodes][nodes])
{
    // Set state of current node to visited
    states[checkNode] = VISITED;

    for (int i = 0; i < nodes; i++)
    {
        // Check if value is a node
        if (adj_matrix[checkNode][i] == 1)
        {
            // If node has not been seen, check it
            if (states[i] == INITIAL)
            {
                return DFS(i, nodes, states, adj_matrix);
            }
            // Otherwise cycle found, success
            else if (states[i] == VISITED)
            {
                return 1;
            }
        }
    }

    return 0;
}

/*
 * Function: checkDeadlock
 * --------------------
 * A driver function that calls sets all the node
 * states to initial and then calles DFS on all the required nodes
 *
 *  nodes: The number of nodes that will be in the adjacency graph
 *  adj_matrix: The matrix that adjacency values get read from
 *
 *  returns: An integer 0, if DFS did not find anything, 1 if it did
 */
int checkDeadlock(int nodes, int adj_matrix[nodes][nodes])
{

    // Create an matrix to keep track of the states of all the nodes
    int states[nodes];

    // Set the states of all nodes to initial
    for (int n = 0; n < nodes; n++)
    {
        states[n] = INITIAL;
    }

    // Start traversing list of nodes
    for (int n = 0; n < nodes; n++)
    {
        // Call DFS on node if state is still initial
        if (states[n] == INITIAL)
        {
            return DFS(n, nodes, states, adj_matrix);
        }
    }

    return 0;
}

/*
 * Function: findHolding
 * --------------------
 * Uses iteration to first determine what semaphore the current bus is waiting for
 * then using this value to search again for the bus that is holding this value
 *
 *  numBuses: The number of buses(rows) in the matrix
 *  deadlockGrid: A 4 X 5 matrix to hold info on the buses that are deadlocked
 *  bus: The bus to determine who is holding its requested semaphore
 *
 *  returns: holdingBus, the bus that is holding the semaphore bus is requesting
 */
int findHolding(int numBuses, int deadlockGrid[NUM_DIRECTIONS][NUM_DIRECTIONS + 1], int bus)
{
    int waitingFor = 0;
    int holdingBus = 0;

    // Find which semaphore bus is waiting for
    for (int i = 1; i < NUM_DIRECTIONS + 1; i++)
    {
        if (deadlockGrid[bus][i] == 1)
        {
            waitingFor = i;
            break;
        }
    }

    // Find which bus is holding the one this one is waiting for
    for (int i = 0; i < NUM_DIRECTIONS; i++)
    {
        if (deadlockGrid[i][waitingFor] == 2)
        {
            holdingBus = i;
            break;
        }
    }

    return holdingBus;
}

/*
 * Function: createCycle
 * --------------------
 * Read in values from the deadlocked state of matrix.txt,
 * then iterating over these values and extracting those
 * that contain a row that is part of a deadlock, along with its
 * index, and then saving these 5 new values into the matrix deadlockGrid.
 *
 * After doing this, the function then prints out the cycle that is created by these
 * four rows.
 *
 *  numBuses: The number of buses(rows) in the matrix
 *  matrix: The matrix to read the values from
 *  sequence: The characters read in from sequence.txt
 *
 */
void createCycle(int numBuses, int matrix[][NUM_DIRECTIONS], char *sequence)
{

    // Read current values from matrix.txt into curr_matrix
    readMatrix(numBuses, matrix);

    int deadlockGrid[NUM_DIRECTIONS][NUM_DIRECTIONS + 1];
    for (int i = 0, row = 0; i < numBuses; i++)
    {
        for (int j = 0; j < NUM_DIRECTIONS; j++)
        {
            if (matrix[i][j] == 2)
            {
                deadlockGrid[row][0] = i; // store the row index in the first column
                for (int k = 0; k < NUM_DIRECTIONS; k++)
                {
                    deadlockGrid[row][k + 1] = matrix[i][k]; // store the values from matrix in remaining columns
                }
                row++;
                break; // exit inner loop once a deadlock row is found
            }
        }
    }

    // Output the cycle
    printf("---------------------------------------------\n");

    for (int i = 0; i < NUM_DIRECTIONS; i++)
    {
        int holdingBus = findHolding(numBuses, deadlockGrid, i);
        // Args: The current bus, the direction current bus is coming from
        // the bus the current bus is waiting for, the direction the bus
        // the current bus is waiting for is coming from
        printf("Bus <%d> from %c is waiting for Bus<%d> from %c\n", deadlockGrid[i][0] + 1, sequence[deadlockGrid[i][0]], deadlockGrid[holdingBus][0] + 1, sequence[holdingBus]);
    }
    printf("----------------------------------------------\n");
}

int main(int argc, char *argv[])
{

    // Declare variables
    char *sequence;
    double p, r;
    int direction, deadlock = 0;

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

    // Declare variable to contain the number of buses that have not completed
    int remainingBuses = numBuses;

    // Declare matrix
    int matrix[numBuses][NUM_DIRECTIONS];

    // Declare number of nodes in graph
    int nodes = numBuses + NUM_DIRECTIONS;

    // Declare adjacency matrix
    int adj_matrix[nodes][nodes];

    // Write all 0's to matrix.txt
    writeMatrix(numBuses, matrix);

    // Start sending buses
    pid_t pids[numBuses], waitingPid;
    char pid_str[20];
    char direction_str[2];
    char numBuses_str[100];
    // Instantiate first bus ID to 0
    int busID = 0;

    // Fill pids vector with 0's, prevents undefined behaviour
    for (int i = 0; i < numBuses; i++)
    {
        pids[i] = 0;
    }

    // ---------------------SEMAPHORE CREATION---------------------

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

    // Main loop, runs while children still exists and no deadlock has been detected
    while (deadlock != 1 && remainingBuses > 0)
    {

        // If buses till need to be created, enter check
        if (busID < numBuses)
        {
            // Get a random chance of checking for a deadlock
            r = getRandom();

            if (r < p)
            {
                // If r<p, get the semaphore for editing the matrix
                // Then read in values from matrix.txt and check a deadlock
                waitSemaphore(semEditMatrix);
                createGraph(numBuses, nodes, matrix, adj_matrix);
                deadlock = checkDeadlock(nodes, adj_matrix);
                postSemaphore(semEditMatrix);
            }
            else
            {
                // Increment busID for new bus
                busID++;

                // Make child and run the bus
                pid_t pid = fork();
                if (pid < 0)
                {
                    printf("[Error]: Unsuccessful fork to create bus %c at sequence index %d. Program terminating.\n", sequence[busID], busID);
                    exit(EXIT_FAILURE);
                }
                // In child process
                else if (pid == 0)
                {
                    // Convert direction to an integer so bus program can reference direction/semaphores from array
                    switch (sequence[busID - 1])
                    {
                    case 'N':
                        direction = 0;
                        break;
                    case 'W':
                        direction = 1;
                        break;
                    case 'S':
                        direction = 2;
                        break;
                    case 'E':
                        direction = 3;
                        break;
                    default:
                        // Exit program if illegal input detected
                        printf("[Error]: Unexpected input character: %c. Exiting\n", sequence[busID - 1]);
                        exit(EXIT_FAILURE);
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
                // In parent
                else
                {
                    // Save child PID so it can be terminated in deadlock event
                    pids[busID - 1] = pid;
                }
            }
        }
        else
        {
            // Whenever a child process finishes, decrement the # of remaining buses
            if (waitpid(-1, NULL, WNOHANG) != 0)
            {
                remainingBuses--;
            }

            // Break loop if deadlock
            waitSemaphore(semEditMatrix);
            createGraph(numBuses, nodes, matrix, adj_matrix);
            deadlock = checkDeadlock(nodes, adj_matrix);
            postSemaphore(semEditMatrix);
        }
        sleep(1);
    }

    // If deadlock detected print out the cycle to user
    if (deadlock)
    {
        // Send SIGTERM to all children so they always close semaphores
        for (int i = 0; i < numBuses - 1; i++)
        {
            if (pids[i] != 0)
            {
                if (kill(pids[i], SIGTERM) == -1 && errno == ESRCH)
                {
                    // Make sure not to rekill dead processes, creating errors
                    printf("[Warning]: Process %d has already exited, skipping...\n", pids[i]);
                    continue;
                }
            }
        }
        printf("\nSystem Deadlocked!\nThe cycle below was detected:\n\n");
        createCycle(numBuses, matrix, sequence);
    }
    // Otherwise let user know of success
    else if (remainingBuses == 0)
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
    printf("\nAll semaphores successfully closed.\n");

    return 0;
}
