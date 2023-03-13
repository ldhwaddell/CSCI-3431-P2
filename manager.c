#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

int writeMatrix(char *fileName, int rows, int arr[][4])
{
    FILE *file = fopen(fileName, "w");
    int status;
    if (file == NULL)
    {
        return 1;
    }
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            // Set arr[i][j] to 0
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

int updateMatrix(char *fileName, int newValue, int row, int rows, int col, int arr[][4])
{
    FILE *file = fopen(fileName, "r+");
    if (file == NULL)
    {
        return 1;
    }

    // Update matrix value
    arr[row][col] = newValue;

    // Seek to location to update in matrix.txt
    fseek(file, (row) * rows * 2 + (col) * 2, SEEK_CUR);
    fprintf(file, "%d", newValue);
    fclose(file);

    return 0;
}

int main()
{
    // Declare variables
    char *sequence;
    int matrix_write_status;

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

    printf("%s\n", sequence);

    // Declare matrix
    int matrix[strlen(sequence)][4];

    matrix_write_status = writeMatrix("matrix.txt", strlen(sequence), matrix);

    switch (matrix_write_status)
    {
    case 1:
        printf("[Error]: Could not open matrix.txt\n");
        exit(1);
    case 2:
        printf("[Error]: Could not write to file matrix.txt\n");
        exit(1);
    }


    int test = updateMatrix("matrix.txt", 9, 3, strlen(sequence), 0, matrix);

    return 0;
}