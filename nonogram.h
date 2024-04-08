#ifndef NONOGRAM_H_
#define NONOGRAM_H_
typedef struct _NonoGramHints NonoGramHints;


extern NonoGramHints *nonogram_hints_create(
   int **rows;
    int **cols;
    int rows_count;
    int cols_count;
);

extern void nonogram_hints_destroy(NonoGramHints *hints);


extern int nonogram_hints_get_rows_count(NonoGramHints *hints);

extern int nonogram_hints_get_cols_count(NonoGramHints *hints);

extern int nonogram_hints_get_row_value(
  NonoGramHints *hints,
  int row,
  int index
);

extern int nonogram_hints_get_col_value(
  NonoGramHints *hints,
  int col,
  int index
);


extern const char *nonogram_hints_to_string(NonoGramHints *hints);

#endif
