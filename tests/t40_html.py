# -*- coding: utf-8 -*-
import common
from common import setup_module, teardown_module

input = '&amp; S&P, S & P, &'
output =    '& S&P, S & P, &'

class Test:
    def test_1(self):
        common.request('html..')
        common.validate(input, output)
