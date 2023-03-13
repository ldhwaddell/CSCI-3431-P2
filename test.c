#include <stdio.h>
#include <stdlib.h>

int main() {
    int m = 3; // number of rows
    int n = 4; // number of columns
    int matrix[m][n]; // declare matrix
    int i, j;

    // initialize matrix with random values
    for (i = 0; i < m; i++) {
        for (j = 0; j < n; j++) {
            matrix[i][j] = rand() % 10;
        }
    }

    // print matrix to console
    printf("Initial matrix:\n");
    for (i = 0; i < m; i++) {
        for (j = 0; j < n; j++) {
            printf("%d ", matrix[i][j]);
        }
        printf("\n");
    }

    // write matrix to file
    FILE *fp = fopen("matrix.txt", "w");
    if (fp == NULL) {
        printf("Error opening file.\n");
        return 1;
    }
    fprintf(fp, "%d %d\n", m, n);
    for (i = 0; i < m; i++) {
        for (j = 0; j < n; j++) {
            fprintf(fp, "%d ", matrix[i][j]);
        }
        fprintf(fp, "\n");
    }
    fclose(fp);

    // update matrix in file
    int x = 2; // new value
    int y = 0; // row index
    int z = 0; // column index
    fp = fopen("matrix.txt", "r+");
    if (fp == NULL) {
        printf("Error opening file.\n");
        return 1;
    }
    fscanf(fp, "%d %d\n", &m, &n);
    fseek(fp, (y-1)*n*2 + (z-1)*2, SEEK_CUR);
    fprintf(fp, "%d", x);
    fclose(fp);

    // print updated matrix to console
    printf("Updated matrix:\n");
    fp = fopen("matrix.txt", "r");
    if (fp == NULL) {
        printf("Error opening file.\n");
        return 1;
    }
    fscanf(fp, "%d %d\n", &m, &n);
    for (i = 0; i < m; i++) {
        for (j = 0; j < n; j++) {
            fscanf(fp, "%d", &matrix[i][j]);
            printf("%d ", matrix[i][j]);
        }
        printf("\n");
    }
    fclose(fp);

    return 0;
}
