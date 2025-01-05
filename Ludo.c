#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

#define MAX_TOKENS 4
#define BOARD_SIZE 52
#define SAFE_ZONE_COUNT 4
#define NUM_PLAYERS 2

typedef struct {
    int player_id;
    char name[50];
    int tokens[MAX_TOKENS];
    int tokens_in_home;
    int consecutive_sixes;
} Player;

int board[BOARD_SIZE];
int safe_zones[SAFE_ZONE_COUNT] = {0, 13, 26, 39};
int current_turn = 0;
int winner_declared = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

Player players[NUM_PLAYERS];

// Function to check if a position is a safe zone
int is_safe_zone(int position) {
    for (int i = 0; i < SAFE_ZONE_COUNT; i++) {
        if (safe_zones[i] == position) return 1;
    }
    return 0;
}

// Function to check if a token can move
int can_token_move(Player *player, int token_index, int roll) {
    int current_pos = player->tokens[token_index];
    int next_pos = current_pos + roll;

    // Check if token is already in home
    if (current_pos == -2) return 0;

    // Check for exact roll to move into home
    if (next_pos >= BOARD_SIZE) {
        if (next_pos == BOARD_SIZE) return 1; // Exact roll for home
        return 0;
    }

    // Check if the position is occupied by the player's other tokens
    for (int i = 0; i < MAX_TOKENS; i++) {
        if (i != token_index && player->tokens[i] == next_pos) return 0;
    }

    return 1;
}

// Function to move a token
void move_token(Player *player, int token_index, int roll) {
    int current_pos = player->tokens[token_index];
    int next_pos = current_pos + roll;

    // Move to home if it reaches the exact position
    if (next_pos >= BOARD_SIZE) {
        player->tokens[token_index] = -2; // -2 means in home
        player->tokens_in_home++;
        if (player->tokens_in_home == MAX_TOKENS) {
            winner_declared = 1;
            printf("\n*** %s wins the game! ***\n", player->name);
        }
        return;
    }

    // Check for collision
    if (!is_safe_zone(next_pos)) {
        for (int i = 0; i < NUM_PLAYERS; i++) {
            if (i != player->player_id) {
                for (int j = 0; j < MAX_TOKENS; j++) {
                    if (players[i].tokens[j] == next_pos) {
                        printf("%s's token at position %d is sent back to the yard by %s!\n",
                               players[i].name, next_pos, player->name);
                        players[i].tokens[j] = -1; // Send token back to yard
                    }
                }
            }
        }
    }

    // Move the token
    player->tokens[token_index] = next_pos;
}

// Function to roll the dice
int roll_dice() {
    return rand() % 6 + 1;
}

// Function to display the current board state
void display_board() {
    printf("\nCurrent Board State:\n");
    for (int i = 0; i < NUM_PLAYERS; i++) {
        printf("%s's tokens: ", players[i].name);
        for (int j = 0; j < MAX_TOKENS; j++) {
            if (players[i].tokens[j] == -1) {
                printf("[Yard] ");
            } else if (players[i].tokens[j] == -2) {
                printf("[Home] ");
            } else {
                printf("[%d] ", players[i].tokens[j]);
            }
        }
        printf("\n");
    }
    printf("\n");
}

// Function for each player's turn
void *player_turn(void *arg) {
    Player *player = (Player *)arg;

    while (!winner_declared) {
        pthread_mutex_lock(&mutex);

        while (current_turn != player->player_id && !winner_declared) {
            pthread_cond_wait(&cond, &mutex);
        }

        if (winner_declared) {
            pthread_mutex_unlock(&mutex);
            break;
        }

        display_board(); // Display board state before the turn

        printf("\n%s's turn:\n", player->name);

        int roll = roll_dice();
        printf("%s rolled: %d\n", player->name, roll);

        int moved = 0;

        if (roll == 6) {
            player->consecutive_sixes++;
            if (player->consecutive_sixes == 3) {
                printf("%s rolled three consecutive sixes. Turn skipped.\n", player->name);
                player->consecutive_sixes = 0;
                current_turn = (current_turn + 1) % NUM_PLAYERS;
                pthread_cond_broadcast(&cond);
                pthread_mutex_unlock(&mutex);
                continue;
            }
        } else {
            player->consecutive_sixes = 0;
        }

        // Check if any token can move
        for (int i = 0; i < MAX_TOKENS; i++) {
            if (player->tokens[i] == -1 && roll == 6) {
                player->tokens[i] = 0; // Enter token onto the board
                printf("%s moved token %d from yard to start position.\n", player->name, i + 1);
                moved = 1;
                break;
            } else if (can_token_move(player, i, roll)) {
                move_token(player, i, roll);
                printf("%s moved token %d to position %d.\n", player->name, i + 1, player->tokens[i]);
                moved = 1;
                break;
            }
        }

        if (!moved) {
            printf("%s has no valid moves. Turn skipped.\n", player->name);
        }

        if (roll != 6 || player->consecutive_sixes == 3) {
            current_turn = (current_turn + 1) % NUM_PLAYERS;
        }

        display_board(); // Display board state after the turn

        pthread_cond_broadcast(&cond);
        pthread_mutex_unlock(&mutex);

        usleep(300000); // Slightly faster pacing
    }

    pthread_exit(NULL);
}

// Initialize the game
void initialize_game() {
    srand(time(NULL));
    for (int i = 0; i < NUM_PLAYERS; i++) {
        players[i].player_id = i;
        players[i].tokens_in_home = 0;
        players[i].consecutive_sixes = 0;
        for (int j = 0; j < MAX_TOKENS; j++) {
            players[i].tokens[j] = -1; // -1 means in the yard
        }
    }
}

// Main function
int main() {
    pthread_t threads[NUM_PLAYERS];

    initialize_game();

    for (int i = 0; i < NUM_PLAYERS; i++) {
        printf("Enter Player %d's name: ", i + 1);
        fgets(players[i].name, 50, stdin);
        players[i].name[strcspn(players[i].name, "\n")] = '\0'; // Remove newline character
    }

    printf("\nPress Enter to start the game...\n");
    getchar();

    for (int i = 0; i < NUM_PLAYERS; i++) {
        pthread_create(&threads[i], NULL, player_turn, (void *)&players[i]);
    }

    for (int i = 0; i < NUM_PLAYERS; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("\nGame Over! Thank you for playing Ludo!\n");

    return 0;
}
