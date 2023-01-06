# -*- coding: utf-8 -*-
# Conversion of files between different charsets and surfaces.
# Copyright © 1990-2000 Free Software Foundation, Inc.
# François Pinard <pinard@iro.umontreal.ca>, 1990.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, see <https://www.gnu.org/licenses/>.

# This Python script merges several Flex sources intended for Recode.
# It requires Flex 2.5 or later.

import re, sys

# Initial comments.
section0 = [
"/* This file is generated automatically by `mergelex.py'.  */\n"]

# Flex and C declarations.
section1 = '''\

%option nounput
%option noyywrap
%{
#include "common.h"
static RECODE_CONST_REQUEST request;
static RECODE_SUBTASK subtask;

#define YY_INPUT(buf, result, max_size) \
  { \
    int c = get_byte (subtask); \
    result = (c == EOF) ? YY_NULL : (buf[0] = c, 1); \
  }
%}
'''.splitlines(True)

# Flex rules.
section2 = '''\
%%
<<EOF>>			{ return 1; }
'''.splitlines(True)

# Rest of C code.
section3 = ['%%\n']

within_C_code = False
definitions = {}
section = 3

while True:
    # FIXME: Treating all the input as latin1 is a hack: some of the files
    # we read are UTF-8 encoded.
    line = sys.stdin.buffer.readline().decode('latin1')
    if not line:
        break

    # A `/* Step name: NAME.  */' line indicates the start of a new file.
    # Generate an interface routine.  The step name is saved for broketed
    # prefixes in rules.

    match = re.match(r'/\* Step name: (.*)\.  \*/', line)
    if match:
        section = 1
        step_name = match.group(1)
        section3.append('''\

static bool
transform_%s (RECODE_SUBTASK subtask_argument)
{
  subtask = subtask_argument;
  request = subtask->task->request;
  yy_init = 0;
  yyin = subtask->input.file;
  yyout = subtask->output.file;
  BEGIN %s;
  return yylex ();
}
'''
            % (step_name, step_name))
        continue

    # In section 1, any %% in column 1 signals the beginning of lex rules.
    # Remove all white lines.

    if section == 1:
        if re.match('^[ \t]*$', line):
            continue
        if line[:2] == '%%':
            section1.append('%%x %s\n' % step_name)
            section = 2
            section2.append('\n'
                            '<%s>{\n'
                            % step_name)
            continue

    # A %{ comment in column 1 signals the start of a C code section ended
    # by a %} line.

    if line[:2] == '%{':
        within_C_code = True
        section1.append('\n')
        section1.append(line)
        continue
    if line[:2] == '%}':
        section1.append(line)
        section1.append('\n')
        within_C_code = False
        continue
    if within_C_code:
        section1.append(line)
        continue

    # Section 1 declarations are studied and copied, once each.
    # Conflicting declaractions are reported at beginning of output.

    if section == 1 and line[0] not in (' ', '\t', '\n'):
        key, rest = line.split(None, 1)
        if key in definitions:
            if definitions[key] != line:
                sys.stderr.write("** Conflicting definition: %s" % line)
        else:
            definitions[key] = line
            section1.append(line)
        continue

    # In section 2, a %% in column 1 starts section 3.

    if section == 2:
        if line[:2] == '%%':
            section2.append('}\n')
            section = 3
        else:
            section2.append(line)
        continue

    # Section 3 contains C code, all copied verbatim.

    if section == 3:
        section3.append(line)
        continue

# At end, output all temporary buffers.  This produces the generated
# interfaces: one routine for each step name.

if section == 2:
    section2.append('}\n')

sys.stdout.writelines(section0)
sys.stdout.writelines(section1)
sys.stdout.writelines(section2)
sys.stdout.writelines(section3)
