# -*- coding: utf-8 -*-
import common
from common import setup_module, teardown_module

class Test:
    # Stand-alone '&' must be kept as-is.
    def test_1(self):
        input = '&amp; S&P, S & P, &'
        output =    '& S&P, S & P, &'
        common.request('html..')
        common.validate(input, output)
    # Incomplete numeric hex entities should be kept.
    def test_2(self):
        input = '&#x61 &#xZZ &#x; &#x'
        output =    'a &#xZZ &#x; &#x'
        common.request('html..')
        common.validate(input, output)
    # Incomplete numeric decimal entities should be kept.
    def test_3(self):
        input = '&#97 &#ZZ &#; &#'
        output =   'a &#ZZ &#; &#'
        common.request('html..')
        common.validate(input, output)
