# -*- coding: utf-8 -*-

__metaclass__ = type
import os
import subprocess
from __main__ import py

recode_program = os.environ.get('RECODE')

import Recode
outer = Recode.Outer(iconv=False)
outer_iconv = Recode.Outer(iconv=True)

class Run(dict):

    def __getattr__(self, attribute):
        return self[attribute]

    def __setattr__(self, attribute, value):
        self[attribute] = value

run = Run()

def external(flag):
    run.external = flag

def request(text, encoding='utf-8'):
    run.request = bytes(text, encoding)

# Functions only meant to be imported into real testing modules, where
# pytest is meant to find and use them.

def setup_module(module):
    for variable in ('LANG', 'LANGUAGE',
                     'LC_ALL', 'LC_MESSAGES', 'LC_COLLATE'):
        if variable in os.environ:
            del os.environ[variable]
    os.environ['DEFAULT_CHARSET'] = 'ASCII'
    run.external = outer is None
    import tempfile
    run.work = tempfile.mktemp()

def teardown_module(module):
    if os.path.exists(run.work):
        os.remove(run.work)

# Useful functions for various test modules.

def assert_or_diff(output, expected):
    if output != expected:
        import difflib
        diff = difflib.ndiff(expected.splitlines(True),
                             output.splitlines(True))
        import sys
        sys.stdout.write(''.join(diff))
        assert False, (len(output), len(expected))

def external_output(command):
    if not recode_program:
        py.test.skip()
    command = command.replace('$R', recode_program + ' --ignore=:iconv:')
    # FIXME: Find a more portable solution than checking the OS
    return subprocess.check_output(command, universal_newlines=True, shell=os.name != 'nt')

def recode_output(input, encoding='utf-8'):
    if type(input) != bytes:
        input = bytes(input, encoding)
    if run.external:
        open(run.work, 'wb').write(input)
        return external_output('$R %s < %s' % (run.request, run.work))
    if outer is None:
        py.test.skip()
    return outer.recode(run.request, input)

def recode_iconv_output(input, encoding='utf-8'):
    if type(input) != bytes:
        input = bytes(input, encoding)
    if run.external or outer_iconv is None:
        py.test.skip()
    return outer_iconv.recode(run.request, input)

def recode_back_output(input, encoding='utf-8'):
    if type(input) != bytes:
        input = bytes(input, encoding)
    before, after = run.request.split(b'..')
    if run.external:
        open(run.work, 'wb').write(input)
        external_output(b'$R %s %s' % (run.request, run.work))
        return external_output(b'$R %s..%s < %s' % (after, before, run.work))
    if outer is None:
        py.test.skip()
    temp = outer.recode(run.request, input)
    return outer.recode(b'%s..%s' % (after, before), temp)

def validate(input, expected, encoding='utf-8'):
    output = recode_output(input)
    if type(input) != bytes:
        output = output.decode(encoding)
    import sys
    print(type(input), type(output), type(expected))
    assert_or_diff(output, expected)

def validate_back(input, encoding='utf-8'):
    output = recode_back_output(input)
    if type(input) != bytes:
        output = output.decode(encoding)
    assert_or_diff(output, input)
