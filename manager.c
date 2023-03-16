#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

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
    fseek(file, 0, SEEK_END);
    long f_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    sequence = malloc(f_size);

    while ((c = fgetc(file)) != EOF)
    {
        sequence[n++] = (char)c;
    }

    // Add termination character to end of char sequences
    sequence[n] = '\0';

    return sequence;
}

int writeMatrix(int rows, int arr[][NUM_DIRECTIONS])
{
    FILE *file = fopen("matrix.txt", "w");
    int status;
    if (file == NULL)
    {
        return 1;
    }
    for (int i = 0; i < rows; i++)
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

int updateMatrix(int x, int y, int newValue, int rows, int arr[][NUM_DIRECTIONS])
{
    FILE *file = fopen("matrix.txt", "r+");
    int status;
    if (file == NULL)
    {
        return 1;
    }

    // Validation to ensure valid coordinate to update gets entered
    if (x > rows || x < 0)
    {
        return 3;
    }
    else if (y > NUM_DIRECTIONS || y < 0)
    {
        return 4;
    }

    // Update matrix value
    arr[x][y] = newValue;

    for (int i = 0; i < rows; i++)
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

    // Initialize a seed for random number generation
    srand(time(0));

    // Get input for p from command line
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

    printf("Sequence found: %s\n", sequence);

    // Declare number of rows
    int rows = strlen(sequence);

    // Declare matrix
    int matrix[rows][NUM_DIRECTIONS];

    switch (writeMatrix(rows, matrix))
    {
    case 1:
        printf("[Error]: Could not open matrix.txt\n");
        exit(1);
    case 2:
        printf("[Error]: Could not write to file matrix.txt\n");
        exit(1);
    }

    // Start sending buses

    for (int i = 0; i < rows; i++)
    {
        r = getRandom();

        if (r < p)
        {
            checkDeadlock();
        }
        else
        {
            // Make child and run the bus
        }
    }

    int test = updateMatrix(1, 3, 9, rows, matrix);

    return 0;
}