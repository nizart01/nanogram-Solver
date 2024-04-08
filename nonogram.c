
#include "./nonogram.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "./nonogram.inc"

static NonoGramHints *_nonogram_hints_new(int rows_count, int cols_count) {
  if (!rows_count || !cols_count) {
    return NULL;
  }
  NonoGramHints *hints = malloc(sizeof(NonoGramHints));
  if (!hints) {
    return NULL;
  }
  hints->rows = malloc(rows_count * sizeof(int *));
  if (!hints->rows) {
    free(hints);
    return NULL;
  }
  hints->cols = malloc(cols_count * sizeof(int *));
  if (!hints->cols) {

    free(hints->rows);
    free(hints);
    return NULL;
  }
  for (int row = 0; row < rows_count; row++) {
    hints->rows[row] = calloc(cols_count, sizeof(int));
    if (!hints->rows[row]) {
      for (int i = 0; i < row; i++) {
        free(hints->rows[i]);
      }
      free(hints->rows);
      free(hints->cols);
      free(hints);
      return NULL;
    }
  }
  for (int col = 0; col < cols_count; col++) {
    hints->cols[col] = calloc(rows_count, sizeof(int));
    if (!hints->cols[col]) {
      for (int i = 0; i < col; i++) {
        free(hints->cols[i]);
      }
      for (int row = 0; row < rows_count; row++) {
        free(hints->rows[row]);
      }
      free(hints->rows);
      free(hints->cols);
      free(hints);
      return NULL;
    }
  }
  return hints;
}

static NonoGramHints *_nonogram_hints_fill(
  NonoGramHints *hints,
  int **board,
  int rows_count,
  int cols_count
) {
  hints->rows_count = rows_count;
  hints->cols_count = cols_count;
  for (int row = 0; row < rows_count; row++) {
    int count = 0;
    int index = 0;
    for (int col = 0; col < cols_count; col++) {
      if (board[row][col] == 1) {
        count++;
      } else {
        if (count > 0) {
          hints->rows[row][index++] = count;
          count = 0;
        }
      }
    }
    hints->rows[row][index] = count;
  }
  for (int col = 0; col < cols_count; col++) {
    int count = 0;
    int index = 0;
    for (int row = 0; row < rows_count; row++) {
      if (board[row][col] == 1) {
        count++;
      } else {
        if (count > 0) {
          hints->cols[col][index++] = count;
          count = 0;
        }
      }
    }
    hints->cols[col][index++] = count;
  }
  return hints;
}

NonoGramHints *nonogram_hints_create(
  int **board,
  int rows_count,
  int cols_count
) {
  NonoGramHints *hints = _nonogram_hints_new(rows_count, cols_count);
  if (hints) {
    _nonogram_hints_fill(hints, board, rows_count, cols_count);
  }
  return hints;
}

void nonogram_hints_destroy(NonoGramHints *hints) {
  for (int row = 0; row < hints->rows_count; row++) {
    free(hints->rows[row]);
  }
  for (int col = 0; col < hints->cols_count; col++) {
    free(hints->cols[col]);
  }
  free(hints->rows);
  free(hints->cols);
  free(hints);
}

int nonogram_hints_get_rows_count(NonoGramHints *hints) {
  return hints->rows_count;
}

int nonogram_hints_get_cols_count(NonoGramHints *hints) {
  return hints->cols_count;
}

int nonogram_hints_get_row_value(NonoGramHints *hints, int row, int index) {
  assert(row < hints->rows_count);
  assert(index < hints->cols_count);
  return hints->rows[row][index];
}

int nonogram_hints_get_col_value(NonoGramHints *hints, int col, int index) {
  assert(col < hints->cols_count);
  assert(index < hints->rows_count);
  return hints->cols[col][index];
}
static bool _add_string(
  char **pstring,
  unsigned int *plength,
  const char *add
) {
  if (add) {
    unsigned int inc_length = strlen(add);
    char *string = realloc(*pstring, *plength + inc_length + 1);
    if (string) {
      *pstring = string;
      strncpy(*pstring + *plength, add, inc_length + 1);
      *plength += inc_length;
      return true;
    } else {
      return false;
    }
  } else {
    return false;
  }
}
#define _ADD_STRING(pstring, plength, add) \
  if (!_add_string(pstring, plength, add)) \
    return NULL;

const char *nonogram_hints_to_string(NonoGramHints *hints) {
  static char *string = NULL;
  char buffer[128];

  if (!hints) {
    free(string);
    string = NULL;
    return NULL;
  }

  int length = 0;
  _ADD_STRING(&string, &length, "{");

  _ADD_STRING(&string, &length, "\"rows\":[");

  for (int row = 0; row < hints->rows_count; row++) {
    _ADD_STRING(&string, &length, row > 0 ? "," : "");
    _ADD_STRING(&string, &length, "[");
    for (int index = 0; index < hints->cols_count; index++) {
      if (!hints->rows[row][index]) {
        break;
      }
      _ADD_STRING(&string, &length, index > 0 ? "," : "");
      snprintf(buffer, sizeof buffer, "%d", hints->rows[row][index]);
      _ADD_STRING(&string, &length, buffer);
    }
    _ADD_STRING(&string, &length, "]");
  }
  _ADD_STRING(&string, &length, "]");
  _ADD_STRING(&string, &length, ",\"cols\":[");

  for (int col = 0; col < hints->cols_count; col++) {
    _ADD_STRING(&string, &length, col > 0 ? "," : "");
    _ADD_STRING(&string, &length, "[");
    for (int index = 0; index < hints->rows_count; index++) {
      if (!hints->cols[col][index]) {
        break;
      }
      _ADD_STRING(&string, &length, index > 0 ? "," : "");
      snprintf(buffer, sizeof buffer, "%d", hints->cols[col][index]);
      _ADD_STRING(&string, &length, buffer);
    }
    _ADD_STRING(&string, &length, "]");
  }
  _ADD_STRING(&string, &length, "]");

  _ADD_STRING(&string, &length, "}");

  return string;
}
