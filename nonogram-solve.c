#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "cJSON.h"
#include "pnmio.h"

// Structure to represent Nonogram hints
typedef struct {
    int **rows;
    int **cols;
    int rows_count;
    int cols_count;
} NonoGramHints;

// Function to parse JSON hints file
NonoGramHints *parse_json_hints(const char *hints_file) {
    FILE *file = fopen(hints_file, "r");
    if (file == NULL) {
        fprintf(stderr, "Error: Failed to open JSON hints file.\n");
        return NULL;
    }

    // Read JSON file content
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    char *json_content = (char *)malloc(file_size + 1);
    if (json_content == NULL) {
        fprintf(stderr, "Error: Memory allocation failed.\n");
        fclose(file);
        return NULL;
    }
    fread(json_content, 1, file_size, file);
    json_content[file_size] = '\0';

    // Parse JSON content
    cJSON *json = cJSON_Parse(json_content);
    free(json_content);
    fclose(file);

    if (json == NULL) {
        fprintf(stderr, "Error: Failed to parse JSON hints.\n");
        return NULL;
    }

    // Extract hints for rows
    cJSON *rows_array = cJSON_GetObjectItem(json, "rows");
    if (rows_array == NULL || !cJSON_IsArray(rows_array)) {
        cJSON_Delete(json);
        fprintf(stderr, "Error: Invalid JSON hints format (rows).\n");
        return NULL;
    }
    int rows_count = cJSON_GetArraySize(rows_array);
    int **rows_hints = (int **)malloc(rows_count * sizeof(int *));
    if (rows_hints == NULL) {
        cJSON_Delete(json);
        fprintf(stderr, "Error: Memory allocation failed.\n");
        return NULL;
    }
    for (int i = 0; i < rows_count; i++) {
        cJSON *row = cJSON_GetArrayItem(rows_array, i);
        if (row == NULL || !cJSON_IsArray(row)) {
            cJSON_Delete(json);
            fprintf(stderr, "Error: Invalid JSON hints format (rows).\n");
            return NULL;
        }
        int hints_count = cJSON_GetArraySize(row);
        rows_hints[i] = (int *)malloc((hints_count + 1) * sizeof(int)); // Extra space for the terminating zero
        if (rows_hints[i] == NULL) {
            cJSON_Delete(json);
            fprintf(stderr, "Error: Memory allocation failed.\n");
            return NULL;
        }
        for (int j = 0; j < hints_count; j++) {
            cJSON *hint = cJSON_GetArrayItem(row, j);
            if (hint == NULL || !cJSON_IsNumber(hint)) {
                cJSON_Delete(json);
                fprintf(stderr, "Error: Invalid JSON hints format (rows).\n");
                return NULL;
            }
            rows_hints[i][j] = hint->valueint;
        }
        rows_hints[i][hints_count] = 0; // Terminate with zero
    }

    // Extract hints for columns
    cJSON *cols_array = cJSON_GetObjectItem(json, "cols");
    if (cols_array == NULL || !cJSON_IsArray(cols_array)) {
        cJSON_Delete(json);
        fprintf(stderr, "Error: Invalid JSON hints format (cols).\n");
        return NULL;
    }
    int cols_count = cJSON_GetArraySize(cols_array);
    int **cols_hints = (int **)malloc(cols_count * sizeof(int *));
    if (cols_hints == NULL) {
        cJSON_Delete(json);
        fprintf(stderr, "Error: Memory allocation failed.\n");
        return NULL;
    }
    for (int i = 0; i < cols_count; i++) {
        cJSON *col = cJSON_GetArrayItem(cols_array, i);
        if (col == NULL || !cJSON_IsArray(col)) {
            cJSON_Delete(json);
            fprintf(stderr, "Error: Invalid JSON hints format (cols).\n");
            return NULL;
        }
        int hints_count = cJSON_GetArraySize(col);
        cols_hints[i] = (int *)malloc((hints_count + 1) * sizeof(int)); // Extra space for the terminating zero
        if (cols_hints[i] == NULL) {
            cJSON_Delete(json);
            fprintf(stderr, "Error: Memory allocation failed.\n");
            return NULL;
        }
        for (int j = 0; j < hints_count; j++) {
            cJSON *hint = cJSON_GetArrayItem(col, j);
            if (hint == NULL || !cJSON_IsNumber(hint)) {
                cJSON_Delete(json);
                fprintf(stderr, "Error: Invalid JSON hints format (cols).\n");
                return NULL;
            }
            cols_hints[i][j] = hint->valueint;
        }
        cols_hints[i][hints_count] = 0; // Terminate with zero
    }

    // Create and populate NonoGramHints structure
    NonoGramHints *hints = (NonoGramHints *)malloc(sizeof(NonoGramHints));
    if (hints == NULL) {
        cJSON_Delete(json);
        fprintf(stderr, "Error: Memory allocation failed.\n");
        return NULL;
    }
    hints->rows = rows_hints;
    hints->cols = cols_hints;
    hints->rows_count = rows_count;
    hints->cols_count = cols_count;

    cJSON_Delete(json);
    return hints;
}

// Function to parse the initial board state from the PBM file using libpnmio
int **parse_pbm_board_libpnmio(const char *board_file, int *rows_count, int *cols_count) {
    FILE *file = fopen(board_file, "rb"); // Ouvrir le fichier en mode binaire
    if (file == NULL) {
        fprintf(stderr, "Error: Failed to open PBM board file.\n");
        return NULL;
    }

    int img_type;
    int is_ascii;
    int xdim, ydim;
    int **board = NULL;

    // Lire l'en-tête PBM pour obtenir les dimensions et le type d'image
    img_type = get_pnm_type(file);
    if (img_type == PBM_BINARY || img_type == PBM_ASCII) {
        if (read_pbm_header(file, &xdim, &ydim, &is_ascii) == FALSE) {
            fprintf(stderr, "Error: Invalid PBM file format.\n");
            fclose(file);
            return NULL;
        }
    } else {
        fprintf(stderr, "Error: Unsupported PBM image type.\n");
        fclose(file);
        return NULL;
    }

    // Allouer de la mémoire pour le tableau
    board = (int **)malloc(ydim * sizeof(int *));
    if (board == NULL) {
        fprintf(stderr, "Error: Memory allocation failed.\n");
        fclose(file);
        return NULL;
    }
    for (int i = 0; i < ydim; i++) {
        board[i] = (int *)malloc(xdim * sizeof(int));
        if (board[i] == NULL) {
            fprintf(stderr, "Error: Memory allocation failed.\n");
            fclose(file);
            for (int j = 0; j < i; j++) {
                free(board[j]);
            }
            free(board);
            return NULL;
        }
    }

    // Lire les données de l'image PBM dans le tableau
    if (img_type == PBM_BINARY) {
        read_pbm_data(file, (int *)board, is_ascii);
    } else if (img_type == PBM_ASCII) {
        read_pbm_data(file, (int *)board, TRUE); // Toujours ASCII dans libpnmio
    }

    fclose(file);
    *rows_count = ydim;
    *cols_count = xdim;
    return board;
}
// Function to check if the puzzle is solvable using simplistic reasoning
bool is_puzzle_solvable(const NonoGramHints *hints, const int **initial_board) {
    // Check rows
    for (int i = 0; i < hints->rows_count; i++) {
        int count = 0;
        int hint_index = 0;
        for (int j = 0; j < hints->cols_count; j++) {
            if (initial_board[i][j] == 1) {
                count++;
            } else if (count > 0) {
                if (hints->rows[i][hint_index] != count) {
                    return false;
                }
                hint_index++;
                count = 0;
            }
        }
        if (count > 0 && hints->rows[i][hint_index] != count) {
            return false;
        }
    }

    // Check columns
    for (int j = 0; j < hints->cols_count; j++) {
        int count = 0;
        int hint_index = 0;
        for (int i = 0; i < hints->rows_count; i++) {
            if (initial_board[i][j] == 1) {
                count++;
            } else if (count > 0) {
                if (hints->cols[j][hint_index] != count) {
                    return false;
                }
                hint_index++;
                count = 0;
            }
        }
        if (count > 0 && hints->cols[j][hint_index] != count) {
            return false;
        }
    }

    return true;
}

// Function to solve the nonogram puzzle
void solve_nonogram(const char *hints_file, const char *board_file, const char *output_file) {
 // Parse JSON hints file
    NonoGramHints *hints = parse_json_hints(hints_file);
    if (hints == NULL) {
        fprintf(stderr, "Error: Failed to parse JSON hints.\n");
        return;
    }

    // Parse initial board state from PBM file if provided
    int **initial_board = NULL;
    int rows_count = 0, cols_count = 0; // Ajout des variables pour stocker les dimensions
    if (board_file != NULL) {
        initial_board = parse_pbm_board_libpnmio(board_file, &rows_count, &cols_count);
        if (initial_board == NULL) {
            fprintf(stderr, "Error: Failed to parse initial board state.\n");
            return;
        }
    }

    // Check if the puzzle is solvable using simplistic reasoning
      if (!is_puzzle_solvable(hints, (const int **)initial_board)) {
        fprintf(stderr, "Unsolvable puzzle.\n");
        if (initial_board != NULL) {
            for (int i = 0; i < rows_count; i++) {
                free(initial_board[i]);
            }
            free(initial_board);
        }
        for (int i = 0; i < hints->rows_count; i++) {
            free(hints->rows[i]);
        }
        for (int j = 0; j < hints->cols_count; j++) {
            free(hints->cols[j]);
        }
        free(hints->rows);
        free(hints->cols);
        free(hints);
        return;
    }
    // Implement algorithm to solve the nonogram puzzle based on hints and initial board state
    // This could involve various logical deductions and backtracking
    // For now, let's assume a simple approach of filling all cells as black if the hint is greater than 0

    // Create the solved board
    int **solved_board = (int **)malloc(hints->rows_count * sizeof(int *));
    for (int i = 0; i < hints->rows_count; i++) {
        solved_board[i] = (int *)malloc(hints->cols_count * sizeof(int));
        memset(solved_board[i], 0, hints->cols_count * sizeof(int));
    }

    // Fill in the cells based on row hints
    for (int i = 0; i < hints->rows_count; i++) {
        int index = 0;
        for (int j = 0; j < hints->cols_count; j++) {
            if (hints->rows[i][index] > 0) {
                solved_board[i][j] = 1;
                hints->rows[i][index]--;
            } else {
                index++;
                if (index >= hints->rows[i][0]) {
                    break;
                }
            }
        }
    }

    // Fill in the cells based on column hints
    for (int j = 0; j < hints->cols_count; j++) {
        int index = 0;
        for (int i = 0; i < hints->rows_count; i++) {
            if (hints->cols[j][index] > 0) {
                solved_board[i][j] = 1;
                hints->cols[j][index]--;
            } else {
                index++;
                if (index >= hints->cols[j][0]) {
                    break;
                }
            }
        }
    }

    // Output the solution to the specified file or standard output
    FILE *output = stdout;
    if (output_file != NULL) {
        output = fopen(output_file, "w");
        if (output == NULL) {
            fprintf(stderr, "Error: Failed to open output file for writing.\n");
            return;
        }
    }

    // Write PBM header
    fprintf(output, "P1\n%d %d\n", hints->cols_count, hints->rows_count);

    // Write solved board to output
    for (int i = 0; i < hints->rows_count; i++) {
        for (int j = 0; j < hints->cols_count; j++) {
            fprintf(output, "%d ", solved_board[i][j]);
        }
        fprintf(output, "\n");
    }

    // Close output file if opened
    if (output_file != NULL) {
        fclose(output);
    }

    // Free memory
    for (int i = 0; i < hints->rows_count; i++) {
        free(solved_board[i]);
    }
    free(solved_board);

    if (initial_board != NULL) {
        for (int i = 0; i < hints->rows_count; i++) {
            free(initial_board[i]);
        }
        free(initial_board);
    }

    for (int i = 0; i < hints->rows_count; i++) {
        free(hints->rows[i]);
    }
    free(hints->rows);
    for (int j = 0; j < hints->cols_count; j++) {
        free(hints->cols[j]);
    }
    free(hints->cols);
    free(hints);
}

void print_usage(const char *program_name) {
    printf("Usage: %s <hints_file> [--board <board_file>] [--output <output_file>]\n", program_name);
}

int main(int argc, char *argv[]) {
    // Check the number of arguments
    if (argc < 2) {
        fprintf(stderr, "Error: Not enough arguments.\n");
        print_usage(argv[0]);
        return 1;
    }

    // Parse command-line arguments
    const char *hints_file = argv[1];
    const char *board_file = NULL;
    const char *output_file = NULL;

    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "--board") == 0) {
            if (i + 1 < argc) {
                board_file = argv[i + 1];
                i++;
            } else {
                fprintf(stderr, "Error: Missing argument for --board.\n");
                print_usage(argv[0]);
                return 1;
            }
        } else if (strcmp(argv[i], "--output") == 0) {
            if (i + 1 < argc) {
                output_file = argv[i + 1];
                i++;
            } else {
                fprintf(stderr, "Error: Missing argument for --output.\n");
                print_usage(argv[0]);
                return 1;
            }
        } else {
            fprintf(stderr, "Error: Unknown argument: %s\n", argv[i]);
            print_usage(argv[0]);
            return 1;
        }
    }

    // Solve the nonogram puzzle
    solve_nonogram(hints_file, board_file, output_file);

    return 0;
}
