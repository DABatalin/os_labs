#include "stdio.h"
#include "stdlib.h" 	// pid_t declaration is here
#include "unistd.h" 	// fork() and getpid() declarations are here
#include "pthread.h"
#include <sys/time.h>     // for clock_t, clock(), CLOCKS_PER_SEC
#include <string.h>

pthread_mutex_t player1_mutex;
pthread_mutex_t player2_mutex;

typedef struct
{
	int experiment_num;
	int throws_number;
	int tour_number;
	int player1_start_points;
	int player2_start_points;
    int player1_wins;
	int player2_wins;
} game_parameters;


void* experiment(void *param) {
	game_parameters *game = (game_parameters*) param;

	int player1_wins = 0;
	int player2_wins = 0;
	// printf("experiment_num: %d", game->experiment_num);
	for (int j = 0; j < game->experiment_num; j++) {
		int player1_experiment_points = game->player1_start_points;
		int player2_experiment_points = game->player2_start_points;

		for (int i = game->tour_number; i < game->throws_number; i++) {
			player1_experiment_points += rand() % 12 + 1;
			player2_experiment_points += rand() % 12 + 1;
		}
		if (player1_experiment_points != player2_experiment_points) {
			if (player1_experiment_points > player2_experiment_points) {
				player1_wins += 1;
			}
			else {
				player2_wins += 1;
			}
		}
	}
	// printf("PLayer1 wins: %d\n", player1_wins);
	// printf("PLayer2 wins: %d\n\n", player2_wins);
	pthread_mutex_lock(&player1_mutex);
	game->player1_wins += player1_wins;
	pthread_mutex_unlock(&player1_mutex);

	pthread_mutex_lock(&player2_mutex);
	game->player2_wins += player2_wins;
	pthread_mutex_unlock(&player2_mutex);
	
	pthread_exit(0);
}


int main(int argc, char **argv) {
	srand(time(NULL));
	int threads_number = 1;
	if ((argc > 3) || (argc == 2)) {
		perror("Wrong number of parameters");
		printf("You wrote %d parametres, but it must be only 2 or 0", argc);
		return 2;
    }

	// for (int i = 0; i < argc; i++) {
	// 	printf("%d param: %s\n", i, argv[i]);
	// }

	if (argc == 3) {
		if (strcmp(argv[1], "-t") == 0) {
			threads_number = atoi(argv[2]);
		}
		else {
			perror("No such parameter exists");
			return 3;
		}
	}
	
	int experiment_number = 1;
	game_parameters game = {0};


	pthread_mutex_init(&player1_mutex, NULL);
	pthread_mutex_init(&player2_mutex, NULL);

	printf("Enter the number of throws (K-number): ");
	scanf("%d", &game.throws_number);
	printf("\nEnter the tour number: ");
	scanf("%d", &game.tour_number);
	printf("\nEnter the amount of points of the first player: ");
	scanf("%d", &game.player1_start_points);
	printf("\nEnter the amount of points of the second player: ");
	scanf("%d", &game.player2_start_points);
	printf("\nEnter the number of experiments: ");
	scanf("%d", &experiment_number);

	if (experiment_number < threads_number) {
		perror("The number of threads is more than number of experiments");
		return 4;
	}
	game.experiment_num = experiment_number / threads_number;
	pthread_t tid[threads_number];


    struct timeval start_time, end_time;
    long seconds, microseconds;
    gettimeofday(&start_time, NULL);

    for (int i=0; i<threads_number; i++)
    {
        pthread_create(&tid[i], NULL, experiment, &game);
    }

	for (int i=0; i<threads_number; i++)
    {
        pthread_join(tid[i], NULL);
    }

	gettimeofday(&end_time, NULL);
    seconds = end_time.tv_sec - start_time.tv_sec;
    microseconds = end_time.tv_usec - start_time.tv_usec;
    double elapsed_time = seconds + microseconds / 1e6;

	double player1_chance = (double)game.player1_wins / (double)(game.player1_wins + game.player2_wins);
	double player2_chance = (double)game.player2_wins / (double)(game.player1_wins + game.player2_wins);

	printf("\nFirst player chance to win: %f\n", player1_chance);
	printf("Second player chance to win: %f\n", player2_chance);
    printf("The elapsed time is %f seconds\n", elapsed_time);

	return 0;
}