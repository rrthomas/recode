/* Conversion of files between different charsets and surfaces.
   Copyright © 1990, 93, 94, 96-99, 2000, 2001 Free Software Foundation, Inc.
   Contributed by François Pinard <pinard@iro.umontreal.ca>, 1988.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License
   as published by the Free Software Foundation; either version 3 of the
   License, or (at your option) any later version.

   This library is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the Recode Library; see the file `COPYING.LIB'.
   If not, see <https://www.gnu.org/licenses/>.
*/

/* Extended definitions, for externally written steps.  */

#include "recode.h"

/* Internal typedefs, to supplement those in "recode.h".  */

typedef struct recode_symbol *			RECODE_SYMBOL;
typedef struct recode_option_list *		RECODE_OPTION_LIST;
typedef struct recode_single *			RECODE_SINGLE;
typedef struct recode_step *			RECODE_STEP;
typedef struct recode_alias *			RECODE_ALIAS;
typedef struct recode_subtask *			RECODE_SUBTASK;

typedef const struct recode_option_list *	RECODE_CONST_OPTION_LIST;
typedef const struct recode_outer *		RECODE_CONST_OUTER;
typedef const struct recode_step *		RECODE_CONST_STEP;
typedef const struct recode_alias *		RECODE_CONST_ALIAS;
typedef const struct recode_task *		RECODE_CONST_TASK;

/*--------------------------------------------------------------------------.
| Return from SUBTASK, with `false' if the failure level has been reached.  |
`--------------------------------------------------------------------------*/

#define SUBTASK_RETURN(Subtask) \
  return (Subtask)->task->error_so_far < (Subtask)->task->fail_level

/*-------------------------------------------------------------------------.
| Maintain maximum of ERROR and current error in SUBTASK.  If the abort    |
| level has been reached, then return immediately as with SUBTASK_RETURN.  |
`-------------------------------------------------------------------------*/

#define RETURN_IF_NOGO(Error, Subtask) \
  do {								\
    if (recode_if_nogo (Error, Subtask))			\
       SUBTASK_RETURN (Subtask);				\
  } while (false)

/* Various structure declarations.  */

/*-----------------------------------------.
| Outer variables for the recode library.  |
`-----------------------------------------*/

/* Error codes, in increasing severity.  */

enum recode_error
  {
    RECODE_NO_ERROR,		/* no error so far */
    RECODE_NOT_CANONICAL,	/* input is not exact, but equivalent */
    RECODE_AMBIGUOUS_OUTPUT,	/* output will be misleading */
    RECODE_UNTRANSLATABLE,	/* input is getting lost, while valid */
    RECODE_INVALID_INPUT,	/* input is getting lost, but was invalid */
    RECODE_SYSTEM_ERROR,	/* system returned input/output failure */
    RECODE_USER_ERROR,		/* library is being misused */
    RECODE_INTERNAL_ERROR,	/* programming botch in the library */
    RECODE_MAXIMUM_ERROR	/* impossible value (should be kept last) */
  };

/* Structure for relating alias names to charsets and surfaces.  */

struct recode_alias
  {
    const char *name;		/* charset, surface or alias name */
    RECODE_SYMBOL symbol;	/* associated symbol */
    /* If a charset, list of surfaces usually applied by default.  */
    struct recode_surface_list *implied_surfaces;
  };

/* The sole purpose of qualities is for later attributing step costs.  */

enum recode_size
  {
    RECODE_1,			/* roughly one byte per character */
    RECODE_2,			/* roughly two bytes per character */
    RECODE_4,			/* roughly four bytes per character */
    RECODE_N			/* variable number of bytes per character */
  };

struct recode_quality
  {
    enum recode_size in_size : 3; /* rough byte size of input characters */
    enum recode_size out_size : 3; /* rough byte size of output characters */
    bool reversible : 1;	/* transformation is known to be reversible */
    bool slower : 1;		/* transformation is slower than average */
    bool faster : 1;		/* transformation is faster than average */
  };

/* Main variables of the initialised library.  */

struct recode_outer
  {
    /* This flag asks the library to diagnose and abort itself if errors.  */
    bool auto_abort;

    /* If the external `iconv' library should be initialized and used.  */
    bool use_iconv;

    /* charset.c */
    /* --------- */

    /* Known pairs (for restricting listing).  */
    struct recode_known_pair *pair_restriction;
    unsigned pair_restrictions;

    /* Opaque Hash_table pointer giving access to the single table holding all
       names and aliases for charsets, surfaces and fallback methods.  */
    void *alias_table;

    /* Unique symbols are linked into a list and counted.  */
    RECODE_SYMBOL symbol_list;
    unsigned number_of_symbols;

    /* Arrays of strings ready for argmatch.  */
    char const **argmatch_charset_array;
    char const **argmatch_surface_array;
    const char **realname_charset_array;
    const char **realname_surface_array;

    /* recode.c */
    /* -------- */

    /* Known single steps.  */
    struct recode_single *single_list;
    unsigned number_of_singles;

    /* Identity recoding table.  */
    const unsigned char *one_to_same;

    /* Preset charsets and surfaces.  */
    RECODE_SYMBOL data_symbol;/* special charset defining surfaces */
    RECODE_SYMBOL ucs2_charset; /* UCS-2 */
    RECODE_SYMBOL iconv_pivot; /* `iconv' internal UCS */
    RECODE_SYMBOL crlf_surface; /* for IBM PC machines */
    RECODE_SYMBOL cr_surface;	/* for Macintosh machines */

    /* Preset qualities, to make step initialisation simpler.  */
    struct recode_quality quality_byte_reversible;
    struct recode_quality quality_byte_to_byte;
    struct recode_quality quality_byte_to_ucs2;
    struct recode_quality quality_byte_to_variable;
    struct recode_quality quality_ucs2_to_byte;
    struct recode_quality quality_ucs2_to_variable;
    struct recode_quality quality_variable_to_byte;
    struct recode_quality quality_variable_to_ucs2;
    struct recode_quality quality_variable_to_variable;
  };

/*--------------------------.
| Description of a symbol.  |
`--------------------------*/

enum recode_symbol_type
  {
    RECODE_NO_SYMBOL_TYPE,	/* missing value */
    RECODE_CHARSET,		/* visible in the space of charsets */
    RECODE_DATA_SURFACE		/* this is a mere data surface */
  };

enum recode_data_type
  {
    RECODE_NO_CHARSET_DATA,	/* the charset_table field is unused */
    RECODE_STRIP_DATA,		/* pool pointer and array of strips */
    RECODE_EXPLODE_DATA		/* explode variable length data */
  };

struct recode_symbol
  {
    /* Chaining of all known symbols (charsets and surfaces).  */
    RECODE_SYMBOL next;

    /* Unique ordinal for this symbol, counted from zero.  */
    unsigned ordinal;

    /* Main name.  */
    const char *name;

    /* Name to use for the iconv library, or NULL.  */
    const char *iconv_name;

    /* Type of table.  */
    enum recode_data_type data_type;

    /* Recoding table.  */
    void *data;

    /* Step for data..CHARSET transformation, if any, or NULL.  */
    struct recode_single *resurfacer;

    /* Step for CHARSET..data transformation, if any, or NULL.  */
    struct recode_single *unsurfacer;

    /* Non zero if this is an acceptable charset (not only a surface).  */
    enum recode_symbol_type type : 3;

    /* Non zero if this one should be ignored.  */
    bool ignore : 1;
  };

struct recode_surface_list
  {
    RECODE_CONST_SYMBOL surface;
    struct recode_surface_list *next;
  };

/*-------------------------------------------.
| Description of a single step of recoding.  |
`-------------------------------------------*/

extern bool recode_interrupted;

typedef bool (*Recode_init) (RECODE_STEP, RECODE_CONST_REQUEST,
                             RECODE_CONST_OPTION_LIST,
                             RECODE_CONST_OPTION_LIST);
typedef bool (*Recode_term) (RECODE_STEP);
typedef bool (*Recode_transform) (RECODE_SUBTASK);
typedef bool (*Recode_fallback) (RECODE_SUBTASK, unsigned);

/* The `single' structure holds data needed to decide of sequences, and is
   invariant over actual requests.  The `step' structure holds data needed for
   task execution, it may take care of fallback and option variance.  */

struct recode_single
  {
    /* Chaining of all known single steps.  */
    struct recode_single *next;

    /* Charset before conversion.  */
    RECODE_SYMBOL before;

    /* Charset after conversion.  */
    RECODE_SYMBOL after;

    /* Cost for this single step only.  */
    short conversion_cost;

    /* Initial value for step_table.  */
    void *initial_step_table;

    /* Recoding quality.  */
    struct recode_quality quality;

    /* Initialisation handler, to be called before step optimisation.  */
    Recode_init init_routine;

    /* Transformation handler, for doing the actual recoding work.  */
    Recode_transform transform_routine;

    /* Default fallback for the step.  Merely to implement `-s' option.  */
    Recode_fallback fallback_routine;
  };

enum recode_step_type
  {
    RECODE_NO_STEP_TABLE,	/* the step_table field is unused */
    RECODE_BYTE_TO_BYTE,	/* array of 256 bytes */
    RECODE_BYTE_TO_STRING,	/* array of 256 strings */
    RECODE_UCS2_TO_BYTE,	/* hash from ucs2 to byte */
    RECODE_UCS2_TO_STRING,	/* hash from ucs2 to string */
    RECODE_STRING_TO_UCS2,	/* hash from ucs2 to string, reversed */
    RECODE_COMBINE_EXPLODE,	/* raw data for combining or exploding */
    RECODE_COMBINE_STEP,	/* special hash for combining */
    RECODE_EXPLODE_STEP		/* special hash for exploding */
  };

struct recode_step
  {
    /* Charset before conversion.  */
    RECODE_SYMBOL before;

    /* Charset after conversion.  */
    RECODE_SYMBOL after;

    /* Recoding quality.  */
    struct recode_quality quality;

    /* Type of table.  */
    enum recode_step_type step_type;

    /* Recoding table.  */
    void *step_table;
    void (*step_table_term_routine)(void *);

    /* Step specific variables.  */
    void *local;

    /* Transformation handler, for doing the actual recoding work.  */
    Recode_transform transform_routine;

    /* Fallback for the step.  */
    Recode_fallback fallback_routine;

    /* Cleanup handler, to be called after the recoding is done.  */
    Recode_term term_routine;
  };

struct recode_option_list
  {
    const char *option;
    RECODE_OPTION_LIST next;
  };

/*------------------------------------------------------------------------.
| A recoding request holds, among other things, a selected path among the |
| available recoding steps, it so represents a kind of recoding plan.     |
`------------------------------------------------------------------------*/

struct recode_request
  {
    /* A request is always associated with a recoding system.  */
    RECODE_OUTER outer;

    /* By setting the following flag, the program will echo to stderr the
       sequence of elementary recoding steps needed to achieve the requested
       recoding.  */
    bool verbose_flag : 1;

    /* In `texte' charset, some countries use double quotes to mark diaeresis,
       while some other prefer colons.  This field contains the diaeresis
       character for `texte' charset.  Nominally set to a double quote, it can
       be forced to a colon.  Those are the only two acceptable values.  */
    char diaeresis_char;

    /* If producing a recoding table in source form, there will be no actual
       recoding done, and consequently, the optimisation of step sequence can
       be attempted more aggressively.  If the step sequence cannot be reduced
       to a single step, table production will fail.  */
    bool make_header_flag : 1;

    /* For `latex/bibtex' charsets, it is often convenient to recode the diacritics
       only, while letting other LaTeX code using backslashes unrecoded.  In
       the other charset, one can edit text as well as LaTeX directives.  */
    bool diacritics_only : 1;

    /* For `ibmpc' charset, characters 176 to 223 are use to draw boxes.  If
       this field is set, while getting out of `ibmpc', ASCII characters are
       selected so to approximate these boxes.  */
    bool ascii_graphics : 1;

    /* Array stating the sequence of conversions.  */
    RECODE_STEP sequence_array;
    size_t sequence_allocated;
    short sequence_length;

    /* Internal variables used while scanning request text.  */
    char *work_string;		/* buffer space for generated work strings */
    size_t work_string_length;	/* length of work_string */
    size_t work_string_allocated; /* allocated length of work_string */
    const char *scan_cursor;	/* next character to be seen */
    char *scanned_string;	/* buffer space to scan strings */
  };

/*--------------------------------------------------------------------.
| A recoding text is either an external file or an in memory buffer.  |
`--------------------------------------------------------------------*/

/* While the recoding is going on, FILE being non-NULL has precedence over
   BUFFER.  Moreover, if NAME is not NULL at start of recoding, this is
   interpreted as a request for the library to open the named file, either
   in read or write mode, and also to close it afterwards.  Standard input
   or output is denoted by NAME being an empty string.

   If FILE is NULL in input mode, the in-memory read-only text extends from
   BUFFER to LIMIT.  There is no clue about if the buffer has been allocated
   bigger.  When CURSOR reaches LIMIT, there is no more data to get.  If
   FILE is NULL in output mode, the in-memory text extends from BUFFER to
   CURSOR, but the buffer has been allocated until LIMIT.  When CURSOR
   reaches LIMIT, the buffer should be reallocated bigger, as needed.  */

struct recode_read_only_text
  {
    const char *name;
    FILE *file;
    const char *buffer;
    const char *cursor;
    const char *limit;
  };

struct recode_read_write_text
  {
    const char *name;
    FILE *file;
    char *buffer;
    char *cursor;
    char *limit;
  };

/* Tells how to swap the incoming pair of bytes, while reading UCS-2.  */

enum recode_swap_input
  {
    RECODE_SWAP_UNDECIDED,	/* the text has not been read yet */
    RECODE_SWAP_NO,		/* no need to swap pair of bytes */
    RECODE_SWAP_YES		/* should swap incoming pair of bytes */
  };

/*--------------------------------------------------------------------------.
| A recoding subtask associates a particular recoding step to a given input |
| text, for producing a corresponding output text.  It also holds error     |
| related statistics for the execution of that step.                        |
`--------------------------------------------------------------------------*/

struct recode_subtask
  {
    /* Task for which this subtask is an element.  */
    RECODE_TASK task;

    /* Step being executed by this subtask.  */
    RECODE_CONST_STEP step;

    /* Current input and output.  */
    struct recode_read_only_text input;
    struct recode_read_write_text output;

    /* Line count and character count in last line, both zero-based.  */
    unsigned newline_count;
    unsigned character_count;
  };

#define GOT_CHARACTER(Subtask) \
  ((Subtask)->character_count++)

#define GOT_NEWLINE(Subtask) \
  ((Subtask)->newline_count++, (Subtask)->character_count = 0)

/*--------------------------------------------------------------------------.
| A recoding task associates a sequence of steps to a given input text, for |
| producing a corresponding output text.  It holds an array of subtasks.    |
`--------------------------------------------------------------------------*/

struct recode_task
  {
    /* Associated request.  */
    RECODE_CONST_REQUEST request;

    /* Initial input and final output.  */
    struct recode_read_only_text input;
    struct recode_read_write_text output;

    /* Produce a byte order mark on UCS-2 output, insist for it on input.  */
    bool byte_order_mark : 1;

    /* The input UCS-2 stream might have bytes swapped (status variable).  */
    enum recode_swap_input swap_input : 3;

    /* Error processing.  */
    /* -----------------  */

    /* At this level, there will be failure.  */
    enum recode_error fail_level : 5;

    /* At this level, task should be interrupted.  */
    enum recode_error abort_level : 5;

    /* Maximum error level met so far (status variable).  */
    enum recode_error error_so_far : 5;

    /* Step being executed when error_so_far was last set.  */
    RECODE_CONST_STEP error_at_step;
  };

/* Specialities for some function arguments.  */

/* For restricting charset lists.  */

struct recode_known_pair
  {
    unsigned char left;		/* first character in pair */
    unsigned char right;	/* second character in pair */
  };

/*----------------------.
| Various definitions.  |
`----------------------*/

typedef unsigned short recode_ucs2;

/* Double tables are generated as arrays of indices into a pool of strips,
   each strip holds STRIP_SIZE UCS-2 characters.  Some memory is saved by
   not allowing duplicate strips in the pool.  A smaller strip size yields
   more duplicates and so, a smaller pool, but then, tables get longer
   because more strip indices are needed for each table.  It is difficult to
   predict the optimal strip size.  Tests made on 1997-09-22 showed that a
   strip size of 4 needs 27808 bytes total, 8 needs 22656, 16 needs 23584
   and 32 needs 25568, so we decided to stick to a strip size of 8.  Change
   $STRIP_SIZE in `doc/tables.pl' if you change the value here.  */

/* "Are we speaking slips, strips or bars?" (of gold-pressed latinum :-) */
#define STRIP_SIZE 8

/* An struct strip_data is a pointer to a pool of strips, and an array
   of 256/STRIP_SIZE offsets for the start of strips into the pool, each strip
   describes STRIP_SIZE UCS-2 characters.  A missing character in a strip is
   indicated by all 16 bits set.  */
struct strip_data
  {
    const recode_ucs2 *pool;
    const short offset[256 / STRIP_SIZE];
  };

struct ucs2_to_string
  {
    recode_ucs2 code;		/* UCS-2 value */
    unsigned short flags;	/* various flags */
    const char *string;		/* corresponding string */
  };

/* Per module declarations.  */

#ifdef __cplusplus
extern "C" {
#endif

/* recode.c.  */

#define ALLOC_SIZE(Variable, Size, Type) \
  (Variable = (Type *) recode_malloc (outer, (Size)))

#define ALLOC(Variable, Count, Type) \
  ALLOC_SIZE (Variable, (Count) * sizeof (Type), Type)

#define REALLOC(Variable, Count, Type) \
  (Variable = (Type *) recode_realloc (outer, Variable,		\
				       (Count) * sizeof(Type)), \
   Variable)

void recode_error (RECODE_OUTER, const char *, ...);
void recode_perror (RECODE_OUTER, const char *, ...);
void *recode_malloc (RECODE_OUTER, size_t);
void *recode_realloc (RECODE_OUTER, void *, size_t);

unsigned char *invert_table (RECODE_OUTER, const unsigned char *);
bool complete_pairs (RECODE_OUTER, RECODE_STEP,
                     const struct recode_known_pair *, unsigned,
                     bool, bool);
bool transform_byte_to_ucs2 (RECODE_SUBTASK);
bool init_ucs2_to_byte (RECODE_STEP, RECODE_CONST_REQUEST,
                        RECODE_CONST_OPTION_LIST,
                        RECODE_CONST_OPTION_LIST);
bool transform_ucs2_to_byte (RECODE_SUBTASK);

/* charname.c and fr-charname.c.  */

const char *ucs2_to_charname (int);
const char *ucs2_to_french_charname (int);

/* charset.c.  */

enum alias_find_type
{
  SYMBOL_CREATE_CHARSET,	/* charset as given, create as needed */
  SYMBOL_CREATE_DATA_SURFACE,	/* data surface as given, create as needed */
  ALIAS_FIND_AS_CHARSET,	/* disambiguate only as a charset */
  ALIAS_FIND_AS_SURFACE,	/* disambiguate only as a surface */
  ALIAS_FIND_AS_EITHER		/* disambiguate as a charset or a surface */
};

int code_to_ucs2 (RECODE_CONST_SYMBOL, unsigned);
bool prepare_for_aliases (RECODE_OUTER);
RECODE_ALIAS declare_alias (RECODE_OUTER,
                            const char *, const char *);
bool declare_implied_surface (RECODE_OUTER, RECODE_ALIAS,
                              RECODE_CONST_SYMBOL);
bool make_argmatch_arrays (RECODE_OUTER);
void delete_alias (RECODE_ALIAS alias);
RECODE_ALIAS find_alias (RECODE_OUTER, const char *,
                         enum alias_find_type);
bool find_and_report_subsets (RECODE_OUTER);
bool decode_known_pairs (RECODE_OUTER, const char *);

/* combine.c.  */

#define DONE NOT_A_CHARACTER
#define ELSE BYTE_ORDER_MARK_SWAPPED

bool init_explode (RECODE_STEP, RECODE_CONST_REQUEST,
                   RECODE_CONST_OPTION_LIST,
                   RECODE_CONST_OPTION_LIST);
bool explode_byte_byte (RECODE_SUBTASK);
bool explode_ucs2_byte (RECODE_SUBTASK);
bool explode_byte_ucs2 (RECODE_SUBTASK);
bool explode_ucs2_ucs2 (RECODE_SUBTASK);

bool init_combine (RECODE_STEP, RECODE_CONST_REQUEST,
                   RECODE_CONST_OPTION_LIST,
                   RECODE_CONST_OPTION_LIST);
bool combine_byte_byte (RECODE_SUBTASK);
bool combine_ucs2_byte (RECODE_SUBTASK);
bool combine_byte_ucs2 (RECODE_SUBTASK);
bool combine_ucs2_ucs2 (RECODE_SUBTASK);

/* freeze.c.  */

void recode_freeze_tables (RECODE_OUTER);

/* iconv.c.  */

bool transform_with_iconv (RECODE_SUBTASK);
bool module_iconv (struct recode_outer *);
void delmodule_iconv (struct recode_outer *);

/* names.c.  */

bool should_prefer_french (void);

/* mixed.c.  */

bool transform_c_source (RECODE_TASK);
bool transform_po_source (RECODE_TASK);

/* outer.c.  */

bool reversibility (RECODE_SUBTASK, unsigned);
RECODE_SINGLE declare_single
  (RECODE_OUTER, const char *, const char *,
   struct recode_quality,
   bool (*) (RECODE_STEP, RECODE_CONST_REQUEST,
             RECODE_CONST_OPTION_LIST, RECODE_CONST_OPTION_LIST),
   bool (*) (RECODE_SUBTASK));
bool declare_iconv (RECODE_OUTER, const char *, const char *);
bool declare_explode_data (RECODE_OUTER, const unsigned short *,
                           const char *, const char *);
bool declare_strip_data (RECODE_OUTER, struct strip_data *,
                         const char *);

/* pool.c.  */

extern const recode_ucs2 ucs2_data_pool[];

/* request.c.  */

char *edit_sequence (RECODE_REQUEST, bool);

/* rfc1345.c.  */

const char *ucs2_to_rfc1345 (recode_ucs2);

/* task.c.  */

int get_byte (RECODE_SUBTASK);
size_t get_bytes (RECODE_SUBTASK, char *, size_t);
void put_byte (char, RECODE_SUBTASK);
void put_bytes (const char *, size_t, RECODE_SUBTASK);
bool recode_if_nogo (enum recode_error, RECODE_SUBTASK);
bool transform_byte_to_byte (RECODE_SUBTASK);
bool transform_byte_to_variable (RECODE_SUBTASK);

/* ucs.c.  */

/* Replacement character for when correctly formed character has no
   equivalent.  It is not used for ill-formed characters, however.  */
#define REPLACEMENT_CHARACTER 0xFFFD

/* Device for detecting if bytes are swapped.  This value should appear first
   in UCS-2 files.  */
#define BYTE_ORDER_MARK 0xFEFF
#define BYTE_ORDER_MARK_SWAPPED 0xFFFE

/* Never an UCS-2 character.  */
#define NOT_A_CHARACTER 0xFFFF

bool get_ucs2 (unsigned *, RECODE_SUBTASK);
bool get_ucs4 (unsigned *, RECODE_SUBTASK);
bool put_ucs2 (unsigned, RECODE_SUBTASK);
bool put_ucs4 (unsigned, RECODE_SUBTASK);

#ifdef __cplusplus
}
#endif

/* Global macros specifically for Recode.  */

/* Giving a name to the ASCII character assigned to position 0.  */
#define NUL '\0'

#ifdef FLEX_SCANNER

# define PUT_NON_DIACRITIC_BYTE(Byte, Subtask) \
    if (request->diacritics_only)				\
      ECHO;							\
    else							\
      put_byte ((Byte), (Subtask))

/* ECHO may not have a (Subtask) argument, because some ECHO without argument
   is generated by Flex -- yet Vern tells me it won't happen if I inhibit
   the rule about default copying.  Happily enough, within Flex, Subtask is
   `subtask' quite systematically, so it may be used as a constant, here.  */
# define ECHO \
    do {							\
      const char *cursor = librecode_yytext;			\
      int counter = librecode_yyleng;				\
      for (; counter > 0; cursor++, counter--)			\
	put_byte (*cursor, subtask);				\
    } while (false)

#endif /* FLEX_SCANNER */
