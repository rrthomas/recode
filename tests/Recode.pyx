# -*- coding: utf-8 -*-
# Python interface to the Recode C library, for testing.
# Copyright © 1996-2008 Free Software Foundation, Inc.
# François Pinard <pinard@iro.umontreal.ca>, 1988.

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3, or (at your option)
# any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program; if not, see <https://www.gnu.org/licenses/>.

from libcpp cimport bool
from libc.stdlib cimport free
from libc.stdio cimport FILE

cdef extern from "common.h":

    ## Forwarded.

    cdef struct recode_outer
    ctypedef recode_outer *RECODE_OUTER
    ctypedef recode_outer *RECODE_CONST_OUTER

    cdef struct recode_single
    ctypedef recode_single *RECODE_SINGLE

    ## Symbols.

    enum recode_symbol_type:
        RECODE_NO_SYMBOL_TYPE
        RECODE_CHARSET
        RECODE_DATA_SURFACE

    enum recode_data_type:
        RECODE_NO_CHARSET_DATA
        RECODE_STRIP_DATA
        RECODE_EXPLODE_DATA

    struct recode_symbol:
        recode_symbol *next
        unsigned ordinal
        char *name
        char *iconv_name
        recode_data_type data_type
        void *data
        RECODE_SINGLE resurfacer
        RECODE_SINGLE unsurfacer
        recode_symbol_type type
        bool ignore
    ctypedef recode_symbol *RECODE_SYMBOL
    ctypedef recode_symbol *RECODE_CONST_SYMBOL

    struct recode_surface_list:
        RECODE_CONST_SYMBOL surface
        recode_surface_list *next

    struct recode_alias:
        char *name
        RECODE_SYMBOL symbol
        recode_surface_list *implied_surfaces
    ctypedef recode_alias *RECODE_ALIAS
    ctypedef recode_alias *RECODE_CONST_ALIAS

    ## Steps.

    enum recode_size:
        RECODE_1
        RECODE_2
        RECODE_4
        RECODE_N

    struct recode_quality:
        recode_size in_size
        recode_size out_size
        bool reversible
        bool slower
        bool faster

    struct recode_option_list:
        char *option
        recode_option_list *next
    ctypedef recode_option_list *RECODE_OPTION_LIST
    ctypedef recode_option_list *RECODE_CONST_OPTION_LIST

    ctypedef bool (*Recode_init)(RECODE_STEP, RECODE_CONST_REQUEST,
                                 RECODE_CONST_OPTION_LIST,
                                 RECODE_CONST_OPTION_LIST)
    ctypedef bool (*Recode_term)(RECODE_STEP, RECODE_CONST_REQUEST)
    ctypedef bool (*Recode_transform)(RECODE_SUBTASK)
    ctypedef bool (*Recode_fallback)(RECODE_SUBTASK, unsigned)

    struct recode_single:
        recode_single *next
        RECODE_SYMBOL before
        RECODE_SYMBOL after
        short conversion_cost
        void *initial_step_table
        recode_quality quality
        Recode_init init_routine
        Recode_transform transform_routine
        Recode_fallback fallback_routine

    enum recode_step_type:
        RECODE_NO_STEP_TABLE
        RECODE_BYTE_TO_BYTE
        RECODE_BYTE_TO_STRING
        RECODE_UCS2_TO_BYTE
        RECODE_UCS2_TO_STRING
        RECODE_STRING_TO_UCS2
        RECODE_COMBINE_EXPLODE
        RECODE_COMBINE_STEP
        RECODE_EXPLODE_STEP

    struct recode_step:
        RECODE_SYMBOL before
        RECODE_SYMBOL after
        recode_quality quality
        recode_step_type step_type
        void *step_table
        void *local
        Recode_transform transform_routine
        Recode_fallback fallback_routine
        Recode_term term_routine
    ctypedef recode_step *RECODE_STEP
    ctypedef recode_step *RECODE_CONST_STEP

    ## Requests.

    struct recode_request:
        RECODE_OUTER outer
        bool verbose_flag
        char diaeresis_char
        bool make_header_flag
        bool diacritics_only
        bool ascii_graphics
        RECODE_STEP sequence_array
        size_t sequence_allocated
        short sequence_length
    ctypedef recode_request *RECODE_REQUEST
    ctypedef recode_request *RECODE_CONST_REQUEST

    ## Tasks.

    struct recode_read_only_text:
        char *name
        FILE *file
        char *buffer
        char *cursor
        char *limit

    struct recode_read_write_text:
        char *name
        FILE *file
        char *buffer
        char *cursor
        char *limit

    enum recode_swap_input:
        RECODE_SWAP_UNDECIDED
        RECODE_SWAP_NO
        RECODE_SWAP_YES

    enum recode_error_ 'recode_error':
        RECODE_NO_ERROR
        RECODE_NOT_CANONICAL
        RECODE_AMBIGUOUS_OUTPUT
        RECODE_UNTRANSLATABLE
        RECODE_INVALID_INPUT
        RECODE_SYSTEM_ERROR
        RECODE_USER_ERROR
        RECODE_INTERNAL_ERROR
        RECODE_MAXIMUM_ERROR

    struct recode_task:
        RECODE_CONST_REQUEST request
        recode_read_only_text input
        recode_read_write_text output
        bool byte_order_mark
        recode_swap_input swap_input
        recode_error_ fail_level
        recode_error_ abort_level
        recode_error_ error_so_far
        RECODE_CONST_STEP error_at_step
    ctypedef recode_task *RECODE_TASK
    ctypedef recode_task *RECODE_CONST_TASK

    struct recode_subtask:
        RECODE_TASK task
        RECODE_CONST_STEP step
        recode_read_only_text input
        recode_read_write_text output
        unsigned newline_count
        unsigned character_count
    ctypedef recode_subtask *RECODE_SUBTASK

    int get_byte(RECODE_SUBTASK)
    void put_byte(int, RECODE_SUBTASK)
    void SET_SUBTASK_ERROR(recode_error_, RECODE_SUBTASK)
    bool SUBTASK_RETURN(RECODE_SUBTASK)
    void RETURN_IF_NOGO(recode_error_, RECODE_SUBTASK)
    void GOT_CHARACTER(RECODE_SUBTASK)
    void GOT_NEWLINE(RECODE_SUBTASK)

    ## Outers.

    struct recode_known_pair:
        unsigned char left
        unsigned char right

    struct recode_outer:
        bool auto_abort
        bool use_iconv
        # charset.c:
        recode_known_pair *pair_restriction
        unsigned pair_restrictions
        void *alias_table
        RECODE_SYMBOL symbol_list
        unsigned number_of_symbols
        char **argmatch_charset_array
        char **argmatch_surface_array
        char **realname_charset_array
        char **realname_surface_array
        # recode.c:
        recode_single *single_list
        unsigned number_of_singles
        unsigned char *one_to_same
        RECODE_SYMBOL data_symbol
        RECODE_SYMBOL ucs2_charset
        RECODE_SYMBOL iconv_pivot
        RECODE_SYMBOL crlf_surface
        RECODE_SYMBOL cr_surface
        recode_quality quality_byte_reversible
        recode_quality quality_byte_to_byte
        recode_quality quality_byte_to_ucs2
        recode_quality quality_byte_to_variable
        recode_quality quality_ucs2_to_byte
        recode_quality quality_ucs2_to_variable
        recode_quality quality_variable_to_byte
        recode_quality quality_variable_to_ucs2
        recode_quality quality_variable_to_variable

    ## Miscellaneous.

    enum recode_list_format:
        RECODE_NO_FORMAT
        RECODE_DECIMAL_FORMAT
        RECODE_OCTAL_FORMAT
        RECODE_HEXADECIMAL_FORMAT
        RECODE_FULL_FORMAT

    enum recode_programming_language:
        RECODE_NO_LANGUAGE
        RECODE_LANGUAGE_C
        RECODE_LANGUAGE_PERL

    enum:
        NUL_ 'NUL'
        STRIP_SIZE_ 'STRIP_SIZE'

    ctypedef unsigned short recode_ucs2

    struct strip_data:
        recode_ucs2 *pool
        short offset[256 // STRIP_SIZE_]

    struct ucs2_to_byte:
        recode_ucs2 code
        unsigned char byte

    struct ucs2_to_string:
        recode_ucs2 code
        unsigned short flags
        char *string

    ## Per module declarations.

    # recode.c

    void recode_error(RECODE_OUTER, char *, ...)
    void recode_perror(RECODE_OUTER, char *, ...)
    void *recode_malloc(RECODE_OUTER, size_t)
    void *recode_realloc(RECODE_OUTER, void *, size_t)

    unsigned char *invert_table(RECODE_OUTER, unsigned char *)
    bool complete_pairs(RECODE_OUTER, RECODE_STEP,
                        recode_known_pair *, unsigned, bool, bool)
    bool transform_byte_to_ucs2(RECODE_SUBTASK)
    bool init_ucs2_to_byte(RECODE_STEP, RECODE_CONST_REQUEST,
                           RECODE_CONST_OPTION_LIST, RECODE_CONST_OPTION_LIST)
    bool transform_ucs2_to_byte(RECODE_SUBTASK)

    # charname.c and fr-charname.c

    char *ucs2_to_charname(int)
    char *ucs2_to_french_charname(int)

    # charset.c

    enum alias_find_type:
        SYMBOL_CREATE_CHARSET_ 'SYMBOL_CREATE_CHARSET'
        SYMBOL_CREATE_DATA_SURFACE_ 'SYMBOL_CREATE_DATA_SURFACE'
        ALIAS_FIND_AS_CHARSET_ 'ALIAS_FIND_AS_CHARSET'
        ALIAS_FIND_AS_SURFACE_ 'ALIAS_FIND_AS_SURFACE'
        ALIAS_FIND_AS_EITHER_ 'ALIAS_FIND_AS_EITHER'

    int code_to_ucs2(RECODE_CONST_SYMBOL, unsigned)
    bool prepare_for_aliases(RECODE_OUTER)
    RECODE_ALIAS declare_alias(RECODE_OUTER, char *, char *)
    bool declare_implied_surface(RECODE_OUTER, RECODE_ALIAS,
                                 RECODE_CONST_SYMBOL)
    bool make_argmatch_arrays(RECODE_OUTER)
    RECODE_ALIAS find_alias(RECODE_OUTER, char *, alias_find_type)
    bool find_and_report_subsets(RECODE_OUTER)
    bool decode_known_pairs(RECODE_OUTER, char *)

    # combine.c

    enum:
        DONE
        ELSE_ 'ELSE'

    bool init_explode(RECODE_STEP, RECODE_CONST_REQUEST,
                      RECODE_CONST_OPTION_LIST, RECODE_CONST_OPTION_LIST)
    bool explode_byte_byte(RECODE_SUBTASK)
    bool explode_ucs2_byte(RECODE_SUBTASK)
    bool explode_byte_ucs2(RECODE_SUBTASK)
    bool explode_ucs2_ucs2(RECODE_SUBTASK)

    bool init_combine(RECODE_STEP, RECODE_CONST_REQUEST,
                      RECODE_CONST_OPTION_LIST, RECODE_CONST_OPTION_LIST)
    bool combine_byte_byte(RECODE_SUBTASK)
    bool combine_ucs2_byte(RECODE_SUBTASK)
    bool combine_byte_ucs2(RECODE_SUBTASK)
    bool combine_ucs2_ucs2(RECODE_SUBTASK)

    # freeze.c

    void recode_freeze_tables(RECODE_OUTER)

    # iconv.c

    bool transform_with_iconv(RECODE_SUBTASK)

    # localcharset.c

    char *locale_charset()

    # names.c

    bool should_prefer_french()

    # mixed.c

    bool transform_c_source(RECODE_TASK)
    bool transform_po_source(RECODE_TASK)

    # outer.c

    ctypedef bool (*declare_single_Arg5)(
            RECODE_STEP, RECODE_CONST_REQUEST,
            RECODE_CONST_OPTION_LIST, RECODE_CONST_OPTION_LIST)
    ctypedef bool (*declare_single_Arg6)(RECODE_SUBTASK)

    bool reversibility(RECODE_SUBTASK, unsigned)
    RECODE_SINGLE declare_single(
            RECODE_OUTER, char *, char *, recode_quality,
            declare_single_Arg5, declare_single_Arg6)
    bool declare_iconv(RECODE_OUTER, char *, char *)
    bool declare_explode_data(RECODE_OUTER, unsigned short *, char *, char *)
    bool declare_strip_data(RECODE_OUTER, strip_data *, char *)

    # pool.c

    extern recode_ucs2 ucs2_data_pool[]

    # request.c

    char *edit_sequence(RECODE_REQUEST, bool)

    # rfc1345.c

    char *ucs2_to_rfc1345(recode_ucs2)

    # task.c

    int get_byte(RECODE_SUBTASK)
    void put_byte(int, RECODE_SUBTASK)
    bool recode_if_nogo(recode_error_, RECODE_SUBTASK)
    bool transform_byte_to_byte(RECODE_SUBTASK)
    bool transform_byte_to_variable(RECODE_SUBTASK)

    # ucs.c

    enum:
        REPLACEMENT_CHARACTER_ 'REPLACEMENT_CHARACTER'
        NOT_A_CHARACTER_ 'NOT_A_CHARACTER'
        BYTE_ORDER_MARK_ 'BYTE_ORDER_MARK'
        BYTE_ORDER_MARK_SWAPPED_ 'BYTE_ORDER_MARK_SWAPPED'

    bool get_ucs2(unsigned *, RECODE_SUBTASK)
    bool get_ucs4(unsigned *, RECODE_SUBTASK)
    bool put_ucs2(unsigned, RECODE_SUBTASK)
    bool put_ucs4(unsigned, RECODE_SUBTASK)

    ## Recode library at OUTER level.

    enum:
        RECODE_AUTO_ABORT_FLAG
        RECODE_NO_ICONV_FLAG
        RECODE_STRICT_MAPPING_FLAG

    RECODE_OUTER recode_new_outer(unsigned)
    bool recode_delete_outer(RECODE_OUTER)
    bool list_all_symbols(RECODE_OUTER, RECODE_CONST_SYMBOL)
    bool list_concise_charset(RECODE_OUTER, RECODE_CONST_SYMBOL,
                              recode_list_format)
    bool list_full_charset(RECODE_OUTER, RECODE_CONST_SYMBOL)

    # Recode library at REQUEST level.

    RECODE_REQUEST recode_new_request(RECODE_OUTER)
    bool recode_delete_request(RECODE_REQUEST)
    bool recode_scan_request(RECODE_REQUEST, char *)
    bool recode_format_table(
            RECODE_REQUEST, recode_programming_language, char *)
    bool recode_buffer_to_buffer(
            RECODE_CONST_REQUEST, char *, size_t, char **, size_t *, size_t *)
    bool recode_buffer_to_file(
            RECODE_CONST_REQUEST, char *, size_t, FILE *)
    bool recode_file_to_buffer(
            RECODE_CONST_REQUEST, FILE *, char **, size_t *, size_t *)
    bool recode_file_to_file(RECODE_CONST_REQUEST, FILE *, FILE *)

    # Recode library at TASK level.

    RECODE_TASK recode_new_task(RECODE_CONST_REQUEST)
    bool recode_delete_task(RECODE_TASK)
    bool recode_perform_task(RECODE_TASK)

class error(Exception):
    pass

## Enums repeated for Python.

NO_SYMBOL_TYPE = RECODE_NO_SYMBOL_TYPE
CHARSET = RECODE_CHARSET
DATA_SURFACE = RECODE_DATA_SURFACE

NO_CHARSET_DATA = RECODE_NO_CHARSET_DATA
STRIP_DATA = RECODE_STRIP_DATA
EXPLODE_DATA = RECODE_EXPLODE_DATA

SIZE_1 = RECODE_1
SIZE_2 = RECODE_2
SIZE_4 = RECODE_4
SIZE_N = RECODE_N

NO_STEP_TABLE = RECODE_NO_STEP_TABLE
BYTE_TO_BYTE = RECODE_BYTE_TO_BYTE
BYTE_TO_STRING = RECODE_BYTE_TO_STRING
UCS2_TO_BYTE = RECODE_UCS2_TO_BYTE
UCS2_TO_STRING = RECODE_UCS2_TO_STRING
STRING_TO_UCS2 = RECODE_STRING_TO_UCS2
COMBINE_EXPLODE = RECODE_COMBINE_EXPLODE
COMBINE_STEP = RECODE_COMBINE_STEP
EXPLODE_STEP = RECODE_EXPLODE_STEP

SWAP_UNDECIDED = RECODE_SWAP_UNDECIDED
SWAP_NO = RECODE_SWAP_NO
SWAP_YES = RECODE_SWAP_YES

NO_ERROR = RECODE_NO_ERROR
NOT_CANONICAL = RECODE_NOT_CANONICAL
AMBIGUOUS_OUTPUT = RECODE_AMBIGUOUS_OUTPUT
UNTRANSLATABLE = RECODE_UNTRANSLATABLE
INVALID_INPUT = RECODE_INVALID_INPUT
SYSTEM_ERROR = RECODE_SYSTEM_ERROR
USER_ERROR = RECODE_USER_ERROR
INTERNAL_ERROR = RECODE_INTERNAL_ERROR
MAXIMUM_ERROR = RECODE_MAXIMUM_ERROR

NO_FORMAT = RECODE_NO_FORMAT
DECIMAL_FORMAT = RECODE_DECIMAL_FORMAT
OCTAL_FORMAT = RECODE_OCTAL_FORMAT
HEXADECIMAL_FORMAT = RECODE_HEXADECIMAL_FORMAT
FULL_FORMAT = RECODE_FULL_FORMAT

NO_LANGUAGE = RECODE_NO_LANGUAGE
LANGUAGE_C = RECODE_LANGUAGE_C
LANGUAGE_PERL = RECODE_LANGUAGE_PERL

NUL = NUL_
STRIP_SIZE = STRIP_SIZE_

SYMBOL_CREATE_CHARSET = SYMBOL_CREATE_CHARSET_
SYMBOL_CREATE_DATA_SURFACE = SYMBOL_CREATE_DATA_SURFACE_
ALIAS_FIND_AS_CHARSET = ALIAS_FIND_AS_CHARSET_
ALIAS_FIND_AS_SURFACE = ALIAS_FIND_AS_SURFACE_
ALIAS_FIND_AS_EITHER = ALIAS_FIND_AS_EITHER_

xDONE = DONE
xELSE = ELSE_

REPLACEMENT_CHARACTER = REPLACEMENT_CHARACTER_
NOT_A_CHARACTER = NOT_A_CHARACTER_
BYTE_ORDER_MARK = BYTE_ORDER_MARK_
BYTE_ORDER_MARK_SWAPPED = BYTE_ORDER_MARK_SWAPPED_

AUTO_ABORT_FLAG = RECODE_AUTO_ABORT_FLAG
NO_ICONV_FLAG = RECODE_NO_ICONV_FLAG
STRICT_MAPPING_FLAG = RECODE_STRICT_MAPPING_FLAG

## Recode library at OUTER level.

cdef class Outer:
    cdef RECODE_OUTER outer

    def __init__(self, auto_abort=False, iconv=False, strict=False):
        cdef int flags
        cdef RECODE_SINGLE single
        flags = 0
        if auto_abort:
            flags = flags | RECODE_AUTO_ABORT_FLAG
        if not iconv:
            flags = flags | RECODE_NO_ICONV_FLAG
        if strict:
            flags = flags | RECODE_STRICT_MAPPING_FLAG
        self.outer = recode_new_outer(flags)
        if strict:
            single = self.outer.single_list
            while single is not NULL:
                single.fallback_routine = NULL
                single = single.next

    def __dealloc__(self):
        recode_delete_outer(self.outer)

    def default_charset(self):
        return locale_charset()

    def all_charsets(self):
        list = []
        cdef RECODE_SYMBOL symbol
        symbol = self.outer.symbol_list
        while symbol is not NULL:
            if (symbol.type == RECODE_CHARSET
                    and symbol is not self.outer.iconv_pivot
                    and symbol is not self.outer.data_symbol):
                list.append(symbol.name)
            symbol = symbol.next
        return list

    def all_surfaces(self):
        list = []
        cdef RECODE_SYMBOL symbol
        symbol = self.outer.symbol_list
        while symbol is not NULL:
            if symbol.type != RECODE_CHARSET:
                list.append(symbol.name)
            symbol = symbol.next
        return list

    def concise_charset(self, format=NO_FORMAT):
        ok = list_concise_charset(self.outer, NULL, format)
        if not ok:
            raise error

    def full_charset(self):
        ok = list_full_charset(self.outer, NULL)
        if not ok:
            raise error

    # Lazy, all in one call.
    def recode(self, char *command, char *input, verbose=False):
        request = Request(self)
        request.set_verbose(verbose)
        request.scan(command)
        return request.string(input)

# Recode library at REQUEST level.

cdef class Request:
    cdef RECODE_REQUEST request

    def __init__(self, Outer outer):
        self.request = recode_new_request(outer.outer)

    def __dealloc__(self):
        recode_delete_request(self.request)

    def set_verbose(self, flag):
        previous = self.request.verbose_flag != 0
        self.request.verbose_flag = int(flag)
        return previous

    def scan(self, char *text):
        ok = recode_scan_request(self.request, text)
        if not ok:
            raise error

    def pair_sequence(self):
        list = []
        cdef recode_step step
        cdef short counter
        for counter from 0 <= counter < self.request.sequence_length:
            step = self.request.sequence_array[counter]
            list.append((step.before.name, step.after.name))
        return list

    def format_table(self, int language, char *charset):
        cdef RECODE_OUTER outer
        cdef bool saved
        outer = self.request.outer
        saved = outer.iconv_pivot.ignore
        outer.iconv_pivot.ignore = True
        ok = recode_format_table(
                self.request, <recode_programming_language> language, charset)
        outer.iconv_pivot.ignore = saved
        if not ok:
            raise error

    def string(self, text):
        cdef char *input = text
        cdef size_t input_len = len(text)
        cdef char *output = NULL
        cdef size_t output_len
        cdef size_t output_allocated = 0
        result = recode_buffer_to_buffer(self.request, input, input_len, &output, &output_len, &output_allocated)
        if result is False or output is NULL:
            raise error
        try:
            py_string = output[:output_len]
        finally:
            free (output)
        return py_string

    # Unexposed APIs:

    # Don't expose recode_string; always check return value
    #
    #char *recode_string(
    #        RECODE_CONST_REQUEST, char *)

    # Prefer buffer APIs, which allow NUL characters.
    #
    #bool recode_string_to_buffer(
    #        RECODE_CONST_REQUEST, char *, char **, size_t *, size_t *)
    #bool recode_string_to_file(
    #        RECODE_CONST_REQUEST, char *, FILE *)

    # FIXME: Expose these?
    #
    #bool recode_buffer_to_file(
    #        RECODE_CONST_REQUEST, char *, size_t, FILE *)
    #bool recode_file_to_buffer(
    #        RECODE_CONST_REQUEST, FILE *, char **, size_t *, size_t *)
    #bool recode_file_to_file(RECODE_CONST_REQUEST, FILE *, FILE *)

# Recode library at TASK level.

cdef class Task:
    cdef RECODE_TASK task

    def __init__(self, Request request):
        self.task = recode_new_task(request.request)

    def __dealloc__(self):
        free (self.task.output.buffer)
        recode_delete_task(self.task)

    def set_byte_order_mark(self, flag):
        previous = self.task.byte_order_mark != 0
        self.task.byte_order_mark = int(flag)
        return previous

    def get_error(self):
        return self.task.error_so_far

    def set_fail_level(self, fail_level):
        previous = self.task.fail_level
        self.task.fail_level = fail_level
        return previous

    def set_abort_level(self, abort_level):
        previous = self.task.abort_level
        self.task.abort_level = abort_level
        return previous

    def set_input(self, text):
        cdef char *input = text
        cdef size_t input_len = len(text)
        self.task.input.buffer = input
        self.task.input.cursor = input
        self.task.input.limit = input + input_len

    def get_output(self):
        return self.task.output.buffer[:self.task.output.cursor - self.task.output.buffer]

    def perform(self):
        return recode_perform_task(self.task)
