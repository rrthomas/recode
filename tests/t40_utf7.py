# -*- coding: utf-8 -*-
import common
from common import setup_module, teardown_module

inputs = [
    '\n',
    'A+ImIDkQ.\n',
    'Hi Mom +Jjo!\n',
    '+ZeVnLIqe\n',
    'Item 3 is +AKM-1.\n',
    '1 +- 1 = 2\n',
    ]

outputs = [
    '0xFEFF, 0x000A\n',
    '0xFEFF, 0x0041, 0x2262, 0x0391, 0x002E, 0x000A\n',
    ('0xFEFF, 0x0048, 0x0069, 0x0020, 0x004D, 0x006F, 0x006D, 0x0020,\n'
     '0x263A, 0x0021, 0x000A\n'),
    '0xFEFF, 0x65E5, 0x672C, 0x8A9E, 0x000A\n',
    ('0xFEFF, 0x0049, 0x0074, 0x0065, 0x006D, 0x0020, 0x0033, 0x0020,\n'
     '0x0069, 0x0073, 0x0020, 0x00A3, 0x0031, 0x002E, 0x000A\n'),
    ('0xFEFF, 0x0031, 0x0020, 0x002B, 0x0020, 0x0031, 0x0020, 0x003D,\n'
     '0x0020, 0x0032, 0x000A\n')]

class Test:

    def setup_method(self, method):
        common.request('u7..u6/x2')

    def test_1(self):
        # Single lines from UTF-7.
        for input, output in zip(inputs, outputs):
            yield common.validate, input, output

    def test_2(self):
        # Single lines from UTF-7 and back.
        for input in inputs:
            yield common.validate_back, input

    def test_3(self):
        # All lines at once and back.
        yield common.validate_back, ''.join(inputs)
