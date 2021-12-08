#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <time.h>

// struct to be sent to threads
struct thread_d {
    char type;
    char* order;
    char** matrix;
    int size;
};

bool isFull(char ** matrix, int N){
// postcondition: makes sure that the matrix is full.
    for (int i = 0; i < N; i++){
        for(int j = 0; j < N; j++){
            if (matrix[i][j] != 'x' && matrix[i][j] != 'o'){
                return false;
            }
        }
    }
    return true;
}

bool isWin(char ** matrix, int N){
// postcondition: makes sure there is a winner

    for (int row = 0; row < N; row++){
        for (int col = 0; col < N; col++){

            // checks columns
            if (col > 0 && col + 1 < N && matrix[row][col] != ' '){
                if (matrix[row][col-1] == matrix[row][col] && matrix[row][col] == matrix[row][col+1]) {
                    return true;
                }
            }
            
            // checks rows
            if (row > 0 && row + 1 < N && matrix[row][col] != ' '){
                if (matrix[row-1][col] == matrix[row][col] && matrix[row][col] == matrix[row+1][col]) {
                    return true;
                }
            }

            // checks diagonals
            if (row > 0 && col > 0 && row + 1 < N && col + 1 < N && matrix[row][col] != ' '){
                if (matrix[row-1][col-1] == matrix[row][col] && matrix[row][col] == matrix[row+1][col+1]) {
                    return true;
                }

                if (matrix[row-1][col+1] == matrix[row][col] && matrix[row][col] == matrix[row+1][col-1]) {
                    return true;
                }
            }
        }
    }

    return false;
}

// mutex for threads
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
bool isOver = false;

void * thread_func(void * arg) {
// postcondition: selects a position and places the player

    // sets variables
    struct thread_d * args = (struct thread_d*) arg;
    char * order = args->order;
    
    while(!isOver){

        pthread_mutex_lock(&lock);  // locks mutex

        // makes sure the player's turn comes
        if (((args->type == 'x' && * order == 'x') | (args->type == 'o' && *order == 'o')) & !isOver){

            // sets matrix varaibles;
            char** matrix = args->matrix;
            int row, col;


            // takes a random place and places the player
            do{
                row = rand()%args->size;
                col = rand()%args->size;
            }while(matrix[row][col] == 'x' || matrix[row][col] == 'o');
            
            matrix[row][col] = args->type;
            

            // prints choise of player
            printf("Player %c played on: (%d,%d)\n", args->type, row, col);

            // makes sure the game isn't over
            if (isFull(matrix, args->size) || isWin(matrix, args->size)){
                isOver = true;
            }

            // sets the next player
            *order = args->type == 'x' ? 'o' : 'x';
        }

        pthread_mutex_unlock(&lock); // unlocks mutex
    }
}

int main(int argc, char* argv[]) {

    // defines threads and matrix dimension
    pthread_t thread1, thread2;
    int N = atoi(argv[1]);
    
    // initializes matrix
    char** matrix = (char**)malloc(N * sizeof(char*));
    for (int i = 0; i < N; i++){
        matrix[i] = (char*)malloc(N * sizeof(char));
        for(int j = 0; j < N; j++){
            matrix[i][j] = ' ';
        }
    }
    
    // defines thread's varaibles
    char ch = 'x';
    struct thread_d t1 = {'x', &ch, matrix, N};
    struct thread_d t2 = {'o', &ch, matrix, N};

    printf("Board Size: %ix%i\n", N,N); // prints matrix dimension
    srand(time(NULL));  // initialize random class

    // runs threads and starts the game
    pthread_create(&thread1, NULL, thread_func, &t1);
    pthread_create(&thread2, NULL, thread_func, &t2);

    // waits for threads until the game is over
    pthread_join(thread1,NULL);
    pthread_join(thread2,NULL);
    printf("Game end\n");

    // determines that there is a winner
    if(isWin(matrix, N)){
        // determines the winner based on who made the last move
        char winner = ch == 'x' ? 'O' : 'X';
        printf("Winner is %c\n", winner);
    } else {
        printf("It is a tie\n");
    }

    // prints matrix
    for (int i = 0; i < N; i++){
        for(int j = 0; j < N; j++){
            printf("[%c] ", matrix[i][j]);
        }
        printf("\n");
    }

    // deallocates matrix
    for (int i = 0; i < N; i++)
        free(matrix[i]);
 
    free(matrix);
    return 0;
}
