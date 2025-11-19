#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>

// --- Grid and Cell Definitions ---
#define ROWS 4
#define COLS 3

// Cell types
enum CellType { BLANK, R1, R2, FAKE };

// Struct to hold cell information
struct Cell {
    enum CellType type;
    char display_char;
};

// --- Tunable Costs ---
#define R2_REWARD_FRONT 100
#define R2_REWARD_DIAGONAL 50
const int R1_PENALTIES[] = {-5, -10, -15, -20}; // Penalty for R1 in row 0, 1, 2, 3
#define FAKE_PENALTY -10000
// --------------------

// --- ANSI Color Codes for Display ---
#define COLOR_RESET   "\x1b[0m"
#define COLOR_RED     "\x1b[41m" // Red background for FAKE
#define COLOR_ORANGE  "\x1b[48;5;208m" // Orange background for R1
#define COLOR_PURPLE  "\x1b[45m" // Purple background for R2
#define COLOR_BLANK   "\x1b[100m" // Grey background for BLANK
#define PATH_LINE     "\x1b[43m" // Yellow background for path
#define PATH_R2       "\x1b[44m" // Blue background for R2 on path

// --- Function Prototypes ---
void randomize_grid(struct Cell grid[ROWS][COLS]);
void print_grid(struct Cell grid[ROWS][COLS], int path[ROWS][2], int show_path);
int get_cell_score(enum CellType cell_type, int row, int move_type);
int find_optimal_path(struct Cell grid[ROWS][COLS], int path[ROWS][2]);
void shuffle(struct Cell arr[], int n);

// --- Main Program Loop ---
int main() {
    srand(time(NULL)); // Seed the random number generator

    struct Cell grid[ROWS][COLS];
    int path[ROWS][2] = {{-1}}; // Stores coordinates [r, c] of the best path
    
    randomize_grid(grid);

    int choice = 0;
    while (choice != 3) {
        printf("\n--- Meihua Forest C --- (Score: %d)\n", find_optimal_path(grid, path));
        print_grid(grid, path, 0); // Print grid without path initially
        
        printf("\nMenu:\n");
        printf("1. Show Optimal Path\n");
        printf("2. Randomize Grid\n");
        printf("3. Exit\n");
        printf("Enter your choice: ");
        
        if (scanf("%d", &choice) != 1) {
            // Clear input buffer on invalid input
            while (getchar() != '\n');
            choice = 0;
        }

        switch (choice) {
            case 1:
                // Recalculate and show path
                find_optimal_path(grid, path);
                printf("\n--- Grid with Optimal Path ---\n");
                print_grid(grid, path, 1);
                printf("\nPress Enter to continue...");
                while (getchar() != '\n'); // Clear buffer
                getchar(); // Wait for Enter
                break;
            case 2:
                randomize_grid(grid);
                break;
            case 3:
                printf("Exiting.\n");
                break;
            default:
                printf("Invalid choice. Please try again.\n");
                break;
        }
    }

    return 0;
}

// --- Function Implementations ---

/**
 * Shuffles an array of Cells.
 */
void shuffle(struct Cell arr[], int n) {
    for (int i = n - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        struct Cell temp = arr[i];
        arr[i] = arr[j];
        arr[j] = temp;
    }
}

/**
 * Fills the grid with a random but valid layout.
 */
void randomize_grid(struct Cell grid[ROWS][COLS]) {
    struct Cell all_items[] = {
        {R1, '1'}, {R1, '1'}, {R1, '1'},
        {R2, '2'}, {R2, '2'}, {R2, '2'}, {R2, '2'},
        {FAKE, 'F'},
        {BLANK, ' '}, {BLANK, ' '}, {BLANK, ' '}, {BLANK, ' '}
    };
    int total_items = sizeof(all_items) / sizeof(all_items[0]);

    struct Cell* grid_flat = &grid[0][0];
    for(int i=0; i<ROWS*COLS; i++) {
        grid_flat[i].type = -1; // Mark as empty
    }

    // Define special indices
    const int edge_indices[] = {0, 1, 2, 3, 5, 6, 8, 9, 10, 11};
    int num_edge_indices = sizeof(edge_indices) / sizeof(edge_indices[0]);
    int fourth_row_indices[] = {9, 10, 11};

    // Place R1 items on edges
    int temp_edge_indices[num_edge_indices];
    for(int i=0; i<num_edge_indices; i++) temp_edge_indices[i] = edge_indices[i];
    int available_edges = num_edge_indices;

    for (int i = 0; i < 3; i++) { // 3 R1 items
        int rand_idx = rand() % available_edges;
        int grid_pos = temp_edge_indices[rand_idx];
        grid_flat[grid_pos] = (struct Cell){R1, '1'};
        // Remove used index
        temp_edge_indices[rand_idx] = temp_edge_indices[available_edges - 1];
        available_edges--;
    }

    // Place Fake item (not in the last row)
    int valid_fake_pos[ROWS * COLS];
    int count = 0;
    for (int i = 0; i < ROWS * COLS; i++) {
        int is_fourth_row = 0;
        for(int j=0; j<3; j++) if(i == fourth_row_indices[j]) is_fourth_row = 1;
        
        if (grid_flat[i].type == -1 && !is_fourth_row) {
            valid_fake_pos[count++] = i;
        }
    }
    int fake_pos = valid_fake_pos[rand() % count];
    grid_flat[fake_pos] = (struct Cell){FAKE, 'F'};

    // Place remaining items
    struct Cell remaining_items[ROWS * COLS];
    count = 0;
    for (int i = 0; i < total_items; i++) {
        enum CellType t = all_items[i].type;
        if (t != R1 && t != FAKE) {
            remaining_items[count++] = all_items[i];
        }
    }
    shuffle(remaining_items, count);
    
    int current_rem = 0;
    for (int i = 0; i < ROWS * COLS; i++) {
        if (grid_flat[i].type == -1) {
            grid_flat[i] = remaining_items[current_rem++];
        }
    }
}

/**
 * Prints the grid to the console with optional path highlighting.
 */
void print_grid(struct Cell grid[ROWS][COLS], int path[ROWS][2], int show_path) {
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            int is_path = 0;
            if (show_path) {
                for (int i = 0; i < ROWS; i++) {
                    if (path[i][0] == r && path[i][1] == c) {
                        is_path = 1;
                        break;
                    }
                }
            }

            char* color = COLOR_BLANK;
            if (is_path) {
                color = (grid[r][c].type == R2) ? PATH_R2 : PATH_LINE;
            } else {
                switch (grid[r][c].type) {
                    case R1: color = COLOR_ORANGE; break;
                    case R2: color = COLOR_PURPLE; break;
                    case FAKE: color = COLOR_RED; break;
                    case BLANK: color = COLOR_BLANK; break;
                }
            }
            printf("%s %c %s", color, grid[r][c].display_char, COLOR_RESET);
        }
        printf("\n");
    }
}

/**
 * Calculates the score for moving to a specific cell.
 */
int get_cell_score(enum CellType cell_type, int row, int move_type) {
    switch (cell_type) {
        case FAKE: return FAKE_PENALTY;
        case R1: return R1_PENALTIES[row];
        case R2: return (move_type == 0) ? R2_REWARD_FRONT : R2_REWARD_DIAGONAL;
        default: return 0;
    }
}

/**
 * Finds the optimal path using dynamic programming and returns the score.
 */
int find_optimal_path(struct Cell grid[ROWS][COLS], int path[ROWS][2]) {
    int dp[ROWS][COLS];
    int from[ROWS][COLS];

    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            dp[r][c] = INT_MIN;
            from[r][c] = -1;
        }
    }

    for (int c = 0; c < COLS; c++) {
        dp[ROWS - 1][c] = get_cell_score(grid[ROWS - 1][c].type, ROWS - 1, 0);
    }

    for (int r = ROWS - 2; r >= 0; r--) {
        for (int c = 0; c < COLS; c++) {
            for (int prev_c = c - 1; prev_c <= c + 1; prev_c++) {
                if (prev_c >= 0 && prev_c < COLS) {
                    if (dp[r + 1][prev_c] > FAKE_PENALTY) {
                        int move_type = (prev_c == c) ? 0 : 1;
                        int current_score = get_cell_score(grid[r][c].type, r, move_type);
                        int new_score = dp[r + 1][prev_c] + current_score;
                        if (new_score > dp[r][c]) {
                            dp[r][c] = new_score;
                            from[r][c] = prev_c;
                        }
                    }
                }
            }
        }
    }

    int max_score = INT_MIN;
    int last_col = -1;
    for (int c = 0; c < COLS; c++) {
        if (dp[0][c] > max_score) {
            max_score = dp[0][c];
            last_col = c;
        }
    }

    if (last_col != -1) {
        int current_col = last_col;
        for (int r = 0; r < ROWS; r++) {
            path[r][0] = r;
            path[r][1] = current_col;
            if (r < ROWS - 1) {
                current_col = from[r][current_col];
            }
        }
    } else {
         for (int r = 0; r < ROWS; r++) path[r][0] = -1; // Invalidate path
    }
    
    return max_score < FAKE_PENALTY ? 0 : max_score;
}
