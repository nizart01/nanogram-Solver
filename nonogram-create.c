#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "pnmio.h" // Include the libpnmio header file
#include "cJSON.h" // Include the cJSON header file

// Structure to represent Nonogram hints
typedef struct {
    int **rows;
    int **cols;
    int rows_count;
    int cols_count;
} NonoGramHints;

// Function to calculate hints from the image array
NonoGramHints *calculate_hints(const int **image, int rows_count, int cols_count) {
    NonoGramHints *hints = malloc(sizeof(NonoGramHints));
    if (hints == NULL) {
        fprintf(stderr, "Error: Memory allocation failed.\n");
        return NULL;
    }

    // Allocate memory for row hints
    hints->rows = malloc(rows_count * sizeof(int *));
    if (hints->rows == NULL) {
        free(hints);
        fprintf(stderr, "Error: Memory allocation failed.\n");
        return NULL;
    }

    // Allocate memory for column hints
    hints->cols = malloc(cols_count * sizeof(int *));
    if (hints->cols == NULL) {
        free(hints->rows);
        free(hints);
        fprintf(stderr, "Error: Memory allocation failed.\n");
        return NULL;
    }

    // Initialize rows and cols counts
    hints->rows_count = rows_count;
    hints->cols_count = cols_count;

    // Calculate row hints
    for (int i = 0; i < rows_count; i++) {
        hints->rows[i] = malloc(cols_count * sizeof(int));
        if (hints->rows[i] == NULL) {
            free(hints->rows);
            free(hints->cols);
            free(hints);
            fprintf(stderr, "Error: Memory allocation failed.\n");
            return NULL;
        }

        int count = 0;
        for (int j = 0; j < cols_count; j++) {
            if (image[i][j] == 1) {
                count++;
            } else if (count > 0) {
                hints->rows[i][hints->rows[i][0]++] = count;
                count = 0;
            }
        }
        if (count > 0) {
            hints->rows[i][hints->rows[i][0]++] = count;
        }
    }

    // Calculate column hints
    for (int j = 0; j < cols_count; j++) {
        hints->cols[j] = malloc(rows_count * sizeof(int));
        if (hints->cols[j] == NULL) {
            for (int i = 0; i < rows_count; i++) {
                free(hints->rows[i]);
            }
            free(hints->rows);
            free(hints);
            fprintf(stderr, "Error: Memory allocation failed.\n");
            return NULL;
        }

        int count = 0;
        for (int i = 0; i < rows_count; i++) {
            if (image[i][j] == 1) {
                count++;
            } else if (count > 0) {
                hints->cols[j][hints->cols[j][0]++] = count;
                count = 0;
            }
        }
        if (count > 0) {
            hints->cols[j][hints->cols[j][0]++] = count;
        }
    }

    return hints;
}

// Function to generate a JSON file containing the hints
void generate_hints_json(const char *hints_file, const NonoGramHints *hints) {
    // Create a cJSON object to represent the hints
    cJSON *json_hints = cJSON_CreateObject();
    if (json_hints == NULL) {
        fprintf(stderr, "Error: Failed to create cJSON object for hints.\n");
        return;
    }

    // Add row hints to the JSON
    cJSON *rows_array = cJSON_AddArrayToObject(json_hints, "rows");
    for (int i = 0; i < hints->rows_count; i++) {
        cJSON *row_array = cJSON_CreateIntArray(hints->rows[i], hints->cols_count);
        cJSON_AddItemToArray(rows_array, row_array);
    }

    // Add column hints to the JSON
    cJSON *cols_array = cJSON_AddArrayToObject(json_hints, "cols");
    for (int i = 0; i < hints->cols_count; i++) {
        cJSON *col_array = cJSON_CreateIntArray(hints->cols[i], hints->rows_count);
        cJSON_AddItemToArray(cols_array, col_array);
    }

    // Write the JSON to a file
    FILE *file = fopen(hints_file, "w");
    if (file != NULL) {
        char *json_str = cJSON_Print(json_hints);
        if (json_str != NULL) {
            fprintf(file, "%s\n", json_str);
            free(json_str);
        } else {
            fprintf(stderr, "Error: Failed to convert cJSON to string.\n");
        }
        fclose(file);
    } else {
        fprintf(stderr, "Error: Failed to open JSON hints file for writing.\n");
    }

    // Free the memory used by cJSON
    cJSON_Delete(json_hints);
}

// Function to generate a PBM file adding some blackened cells if necessary
void generate_board_pbm(const char *board_file, const int **image, int rows_count, int cols_count) {
    FILE *file = fopen(board_file, "w");
    if (file == NULL) {
        fprintf(stderr, "Error: Failed to open PBM board file for writing.\n");
        return;
    }

    // Write PBM header
    fprintf(file, "P1\n%d %d\n", cols_count, rows_count);

    // Write image data to the PBM file
    for (int i = 0; i < rows_count; i++) {
        for (int j = 0; j < cols_count; j++) {
            fprintf(file, "%d ", image[i][j]);
        }
        fprintf(file, "\n");
    }

    fclose(file);
}

// Function to generate an SVG file representing the puzzle
void generate_svg(const char *output_file, const int **image, int rows_count, int cols_count) {
    FILE *file;

    // Open the output file for writing or use stdout if output_file is NULL
    if (output_file != NULL) {
        file = fopen(output_file, "w");
        if (file == NULL) {
            fprintf(stderr, "Error: Failed to open output file for writing.\n");
            return;
        }
    } else {
        file = stdout;
    }

    // Write the SVG header
    if (output_file != NULL) {
        fprintf(file, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
        fprintf(file, "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\" \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\n");
    }
    fprintf(file, "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"%d\" height=\"%d\" version=\"1.1\">\n", cols_count * 20, rows_count * 20);

    // Loop through the image array to generate SVG rectangles
    for (int i = 0; i < rows_count; i++) {
        for (int j = 0; j < cols_count; j++) {
            if (image[i][j] == 1) {
                fprintf(file, "<rect x=\"%d\" y=\"%d\" width=\"20\" height=\"20\" fill=\"black\" stroke=\"black\"/>\n", j * 20, i * 20);
            } else {
                fprintf(file, "<rect x=\"%d\" y=\"%d\" width=\"20\" height=\"20\" fill=\"white\" stroke=\"black\"/>\n", j * 20, i * 20);
            }
        }
    }

    // Write the SVG footer
    fprintf(file, "</svg>\n");

    // Close the output file if it's not stdout
    if (output_file != NULL) {
        fclose(file);
    }
}

int main(int argc, char *argv[]) {
    // Check the number of arguments
    if (argc < 2) {
        fprintf(stderr, "Error: Not enough arguments.\n");
        // Print usage message
        return 1;
    }

    // Get the input PBM image file name
    const char *image_file = argv[1];

    // Read the PBM image file to get the data
    int **image = NULL;
    int rows_count = 0, cols_count = 0;
    image = parse_pbm_board_libpnmio(image_file, &rows_count, &cols_count);
    if (image == NULL) {
        fprintf(stderr, "Error: Failed to parse PBM image.\n");
        return 1;
    }

   //  Calculate hints from the image data
    //NonoGramHints *hints = calculate_hints((const int **)image, rows_count, cols_count);

     //Optionally, generate a JSON file containing the hints
    //if (/* condition to generate the JSON file */) {
     //   generate_hints_json(/* arguments */);
    //}

    // Optionally, generate a PBM file adding some blackened cells
    //if (/* condition to generate the PBM file */) {
       // generate_board_pbm(/* arguments */);
   // }

    // Generate an SVG file representing the puzzle
    //generate_svg(/* arguments */);

    // Free the memory used by the image data and hints
    // ...

    return 0;
}
