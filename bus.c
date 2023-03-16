#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    // Declare variables
    pid_t pid;
    int curr_direction;
    char *directions[] = {"North", "West", "South", "East"};

    // Get arguments from command line

    // Get bus id
    pid = atoi(argv[1]);

    // Get bus direction
    curr_direction = atoi(argv[2]);

    // Let user know DIRECTION bus has started
    printf("Bus <%d>: %s bus started\n", pid, directions[curr_direction]);

    // Figure out what "RIGHT" is

    // IF DIRECTION = NORTH

    // request north semaphore
    // lock matrix
    // update matrix
    // unlock matrix
    // get north semaphore
    // lock matrix
    // update matrix
    // unlock matrix

    // request west semaphore
    // lock matrix
    // update matrix
    // unlock matrix
    // get west semaphore
    // lock matrix
    // update matrix
    // unlock matrix

    // Request junction semaphore
    // Once it has all 3, lock junction
    // Pass junction(sleep 2 seconds)
    // Release junction lock, north, west semaphores

    // lock matrix
    // update all three values
    // unlock matrix

    return 0;
}
