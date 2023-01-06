/* Conversion of files between different charsets and surfaces.
   Copyright © 1990-2022 Free Software Foundation, Inc.
   Contributed by François Pinard <pinard@iro.umontreal.ca>, 1988.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License
   as published by the Free Software Foundation; either version 3 of the
   License, or (at your option) any later version.

   This library is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the Recode Library; see the file `COPYING.LIB'.
   If not, see <https://www.gnu.org/licenses/>.
*/

/* Published (opaque) typedefs.  */
typedef struct recode_outer *                   RECODE_OUTER;
typedef struct recode_request *                 RECODE_REQUEST;
typedef struct recode_task *                    RECODE_TASK;
typedef const struct recode_request *           RECODE_CONST_REQUEST;
typedef const struct recode_symbol *            RECODE_CONST_SYMBOL;

/* Description of list formats.  */

enum recode_list_format
{
  RECODE_NO_FORMAT,             /* format not decided yet */
  RECODE_DECIMAL_FORMAT,        /* concise tabular list using decimal */
  RECODE_OCTAL_FORMAT,          /* concise tabular list using octal */
  RECODE_HEXADECIMAL_FORMAT,    /* concise tabular list using hexadecimal */
  RECODE_FULL_FORMAT            /* full list, one character per line */
};

/* Description of programming languages.  */

enum recode_programming_language
{
  RECODE_NO_LANGUAGE,           /* language not decided yet */
  RECODE_LANGUAGE_C,            /* C (or C++) */
  RECODE_LANGUAGE_PERL          /* Perl */
};

/* Function prototypes.  */

#ifdef __cplusplus
extern "C" {
#endif

/*--------------------------------.
| Recode library at OUTER level.  |
`--------------------------------*/

#define RECODE_AUTO_ABORT_FLAG 1
#define RECODE_NO_ICONV_FLAG 2
#define RECODE_STRICT_MAPPING_FLAG 4
#define RECODE_FORCE_FLAG 8

RECODE_OUTER recode_new_outer (unsigned);
bool recode_delete_outer (RECODE_OUTER);

bool list_all_symbols (RECODE_OUTER, RECODE_CONST_SYMBOL);
bool list_concise_charset (RECODE_OUTER, RECODE_CONST_SYMBOL,
                           const enum recode_list_format);
bool list_full_charset (RECODE_OUTER, RECODE_CONST_SYMBOL);

/*----------------------------------.
| Recode library at REQUEST level.  |
`----------------------------------*/

RECODE_REQUEST recode_new_request (RECODE_OUTER);
bool recode_delete_request (RECODE_REQUEST);
bool recode_scan_request (RECODE_REQUEST, const char *);

bool recode_format_table (RECODE_REQUEST,
                          enum recode_programming_language,
                          const char *);

char *recode_string (RECODE_CONST_REQUEST, const char *);

bool recode_string_to_buffer (RECODE_CONST_REQUEST,
                              const char *,
                              char **, size_t *, size_t *);
bool recode_string_to_file (RECODE_CONST_REQUEST,
                            const char *,
                            FILE *);
bool recode_buffer_to_buffer (RECODE_CONST_REQUEST,
                              const char *, size_t,
                              char **, size_t *, size_t *);
bool recode_buffer_to_file (RECODE_CONST_REQUEST,
                            const char *, size_t,
                            FILE *);
bool recode_file_to_buffer (RECODE_CONST_REQUEST,
                            FILE *,
                            char **, size_t *, size_t *);
bool recode_file_to_file (RECODE_CONST_REQUEST,
                          FILE *,
                          FILE *);

/*-------------------------------.
| Recode library at TASK level.  |
`-------------------------------*/

RECODE_TASK recode_new_task (RECODE_CONST_REQUEST);
bool recode_delete_task (RECODE_TASK);
bool recode_perform_task (RECODE_TASK);
/* FILE *recode_filter_open (RECODE_TASK, FILE *); */
/* bool recode_filter_close (RECODE_TASK); */

#ifdef __cplusplus
}
#endif
