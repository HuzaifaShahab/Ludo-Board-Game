#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Constants
#define MAX_PLAYERS 4
#define TOTAL_TOKENS 4
#define SAFE_POSITION -2

// Player structure
typedef struct {
    char name[50];
    int tokens[TOTAL_TOKENS]; // -1 = Yard, SAFE_POSITION = Safe zone
    int finishedTokens;
} Player;

// Function Declarations
void initializeGame(Player players[], int numPlayers);
void displayBoard(Player players[], int numPlayers);
void playerTurn(Player *player, int playerIndex, Player players[], int numPlayers);
int rollDice();
int allTokensFinished(Player *player);
void displayLeaderboard(Player players[], int numPlayers);

// Main Function
int main() {
    srand(time(0));
    int numPlayers;

    // Get number of players
    printf("Welcome to LUDO!\n");
    do {
        printf("Enter the number of players (2-4): ");
        scanf("%d", &numPlayers);
        if (numPlayers < 2 || numPlayers > 4)
            printf("Invalid input! Please enter a number between 2 and 4.\n");
    } while (numPlayers < 2 || numPlayers > 4);

    Player players[MAX_PLAYERS];
    initializeGame(players, numPlayers);

    int currentPlayer = 0;
    int finishedPlayers = 0;

    // Main Game Loop
    while (finishedPlayers < numPlayers - 1) {
        playerTurn(&players[currentPlayer], currentPlayer, players, numPlayers);

        // Check if the player has finished
        if (allTokensFinished(&players[currentPlayer])) {
            printf("\n%s has finished all tokens!\n", players[currentPlayer].name);
            finishedPlayers++;
        }

        // Move to the next player
        currentPlayer = (currentPlayer + 1) % numPlayers;
    }

    // Display Leaderboard
    displayLeaderboard(players, numPlayers);

    printf("\nGame Over! Thanks for playing.\n");
    return 0;
}

// Function to initialize the game
void initializeGame(Player players[], int numPlayers) {
    for (int i = 0; i < numPlayers; i++) {
        printf("Enter Player %d's name: ", i + 1);
        scanf("%s", players[i].name);
        for (int j = 0; j < TOTAL_TOKENS; j++) {
            players[i].tokens[j] = -1; // All tokens start in the yard
        }
        players[i].finishedTokens = 0;
    }
    printf("\nGame initialized!\n");
}

// Function to display the current board state
void displayBoard(Player players[], int numPlayers) {
    printf("\nCurrent Board:\n");
    for (int i = 0; i < numPlayers; i++) {
        printf("%s: ", players[i].name);
        for (int j = 0; j < TOTAL_TOKENS; j++) {
            if (players[i].tokens[j] == -1)
                printf("Yard ");
            else if (players[i].tokens[j] == SAFE_POSITION)
                printf("Home ");
            else
                printf("Pos-%d ", players[i].tokens[j]);
        }
        printf("\n");
    }
}

// Function for a player's turn
void playerTurn(Player *player, int playerIndex, Player players[], int numPlayers) {
    int diceRoll, consecutiveSixes = 0;

    while (1) {
        printf("\n%s's turn (press Enter to roll the dice)...", player->name);
        getchar();
        getchar();

        diceRoll = rollDice();
        printf("%s rolls: %d\n", player->name, diceRoll);

        // If all tokens are in yard and dice roll is not 6
        int allInYard = 1;
        for (int i = 0; i < TOTAL_TOKENS; i++) {
            if (player->tokens[i] != -1) {
                allInYard = 0;
                break;
            }
        }

        if (allInYard && diceRoll != 6) {
            printf("No tokens can move. Turn skipped.\n");
            return;
        }

        // Allow player to choose a token
        int tokenChoice;
        while (1) {
            displayBoard(players, numPlayers);
            printf("%s, choose a token to move (1-%d): ", player->name, TOTAL_TOKENS);
            scanf("%d", &tokenChoice);

            if (tokenChoice < 1 || tokenChoice > TOTAL_TOKENS) {
                printf("Invalid choice. Try again.\n");
                continue;
            }

            tokenChoice--; // Adjust for 0-based indexing

            // Move logic
            if (player->tokens[tokenChoice] == -1 && diceRoll == 6) {
                player->tokens[tokenChoice] = 1; // Move token out of yard
                printf("Token %d moved out of the yard!\n", tokenChoice + 1);
                break;
            } else if (player->tokens[tokenChoice] >= 1) {
                player->tokens[tokenChoice] += diceRoll;

                // Check for home condition
                if (player->tokens[tokenChoice] >= 50) {
                    player->tokens[tokenChoice] = SAFE_POSITION;
                    player->finishedTokens++;
                    printf("Token %d reached home!\n", tokenChoice + 1);
                }
                break;
            } else {
                printf("Invalid move. Token cannot move. Try again.\n");
            }
        }

        // Handle consecutive sixes
        if (diceRoll == 6) {
            consecutiveSixes++;
            if (consecutiveSixes == 3) {
                printf("Three consecutive 6s! Turn forfeited.\n");
                return;
            }
            printf("You rolled a 6! You get another turn.\n");
        } else {
            break;
        }
    }
}

// Function to roll a dice
int rollDice() {
    return (rand() % 6) + 1;
}

// Function to check if all tokens are home
int allTokensFinished(Player *player) {
    return player->finishedTokens == TOTAL_TOKENS;
}

// Function to display the leaderboard
void displayLeaderboard(Player players[], int numPlayers) {
    printf("\n--- Leaderboard ---\n");
    for (int i = 0; i < numPlayers; i++) {
        printf("%d. %s - Finished Tokens: %d\n", i + 1, players[i].name, players[i].finishedTokens);
    }
}