#!/usr/bin/env python
# -*- coding: utf-8 -*-
# Copyright © 1997-2000 Progiciels Bourbeau-Pinard, Inc.
# François Pinard <pinard@iro.umontreal.ca>, 1997.

"""\
NOTE: This script has not been revised yet as a main program.  Currently,
it is only meant as a part of the Recode test suite.

--------------------------------------------------------------------------

Produce statistics from the results of the bigauto check.

Usage: bigauto [RECODE_OPTION]... [CHARSET_OPTION]...

This script makes a simple analysis for the connectivity of the various
charsets and produce a report on standard output.  The reports include
statistics about the number of steps before and after optimisation.

The option `-hNAME' would affect the resulting output, because there are
more merging rules when this option is in effect.  Other options affect
the result: `-d', `-g' and, notably, `-s'.

All non-option arguments are interpreted as charset names.  If any is given,
the study is limited to those recodings having any of the given charsets
both as a starting and ending points.  If there is no such non-option
argument, all possible possible recodings are considered.
"""

import os, sys
from common import py, Recode
from common import setup_module, teardown_module

class Test:
    avoid_as_before = b'count-characters', b'dump-with-names', b'flat'

    def test_1(self):
        if Recode is None:
            py.test.skip()
        self.outer = Recode.Outer(strict=False)
        self.charsets = sorted(self.outer.all_charsets())
        for before in self.charsets:
            if before not in self.avoid_as_before:
                yield self.validate, before

    def test_2(self):
        if Recode is None:
            py.test.skip()
        self.outer = Recode.Outer(strict=True)
        self.charsets = sorted(self.outer.all_charsets())
        for before in self.charsets:
            if before not in self.avoid_as_before:
                yield self.validate, before

    def validate(self, before):
        # As a compromise between being too terse or too verbose, we
        # consider as a single test, one "before" against all "after"s.
        # However, without a Recode module, we do not know how many
        # "before"s exist, and the skip count is then rather small.
        print(before)
        for after in self.charsets:
            if after is not before:
                request = Recode.Request(self.outer)
                request.scan(b'%s..%s' % (before, after))

def main(*arguments):
    recode_options = []
    charset_options = []
    for argument in arguments:
        if arguments[0] == '-':
            recode_options.append(argument)
        else:
            charset_options.append(argument)
    report = Report()
    report.digest_data(file(work_name).readline)
    report.produce_report(sys.stdout.write)

class Report:

    def __init__(self):
        self.recode_calls = 0
        self.original_count = {}
        self.original_example = {}
        self.original_total = 0
        self.shrunk_count = {}
        self.shrunk_example = {}
        self.shrunk_total = 0

    def digest_data(self, readline):
        lensep = len(os.linesep)
        line = readline()
        while line:
            type, request = line[:-lensep].split(':', 1)
            if type == 'Request':
                steps = self.get_steps(request)
                self.count_original_request(steps, request)
                line = readline()
                if line:
                    if len(line[:-lensep].split(':', 1)) != 2:
                        print('*', line, end='')
                    type, shrunk_to = line[:-lensep].split(':', 1)
                    if type == 'Shrunk to':
                        steps = self.get_steps(shrunk_to)
                        self.count_shrunk_request(steps, shrunk_to)
                        line = readline()
                    else:
                        self.count_shrunk_request(steps, request)
                else:
                    self.count_shrunk_request(steps, request)
            else:
                sys.stderr.write('Unrecognized line: ' + line)
                line = readline()

    def get_steps(self, text):
        if text == '*mere copy*':
            return 0
        if text[-1] == '/':
            text = text[:-1]
        text = text.replace('/..', '..')
        count = 0
        for fragment in text.split('..'):
            count += len(fragment.split('/'))
        return count - 1

    def count_original_request(self, steps, text):
        self.recode_calls += 1
        if steps in self.original_count:
            self.original_count[steps] += 1
        else:
            self.original_count[steps] = 1
            self.original_example[steps] = text.strip()
            if self.original_total == 0:
                self.original_minimum = self.original_maximum = steps
            else:
                if steps < self.original_minimum:
                    self.original_minimum = steps
                if steps > self.original_maximum:
                    self.original_maximum = steps
        self.original_total = self.original_total + steps

    def count_shrunk_request(self, steps, text):
        if steps in self.shrunk_count:
            self.shrunk_count[steps] += 1
        else:
            self.shrunk_count[steps] = 1
            self.shrunk_example[steps] = text.strip()
            if self.shrunk_total == 0:
                self.shrunk_minimum = self.shrunk_maximum = steps
            else:
                if steps < self.shrunk_minimum:
                    self.shrunk_minimum = steps
                if steps > self.shrunk_maximum:
                    self.shrunk_maximum = steps
        self.shrunk_total = self.shrunk_total + steps

    def produce_report(self, write):
        if self.recode_calls == 0:
            sys.stderr.write("No call to report\n")
            return
        write("\n"
              "Optimisation     Original  Shrunk\n"
              "              .-------------------\n"
              "Minimum       |  %2d        %2d\n"
              "Maximum       |  %2d        %2d\n"
              "Average       |  %4.1f      %4.1f\n"
              % (self.original_minimum, self.shrunk_minimum,
                 self.original_maximum, self.shrunk_maximum,
                 float(self.original_total) / float(self.recode_calls),
                 float(self.shrunk_total) / float(self.recode_calls)))
        write("\n"
              "Histogram for original requests\n")
        for steps in range(self.original_minimum, self.original_maximum + 1):
            if steps in self.original_count:
                write("%5d steps, %5d times  %s\n"
                      % (steps, self.original_count[steps],
                         self.original_example[steps]))
        write("\n"
              "Histogram for shrunk requests\n")
        for steps in range(self.shrunk_minimum, self.shrunk_maximum + 1):
            if steps in self.shrunk_count:
                write("%5d steps, %5d times  %s\n"
                      % (steps, self.shrunk_count[steps],
                         self.shrunk_example[steps]))

if __name__ == '__main__':
    main(*sys.argv[1:])
