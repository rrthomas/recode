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
    # Multiple encoding levels should be decodable.  Bug #66.
    def test_4(self):
        input = '&amp;&amp;#35;40;Parenthetical&amp;&amp;#35;41;'
        intermediate1 = '&&#35;40;Parenthetical&&#35;41;'
        intermediate2 = '&#40;Parenthetical&#41;'
        output = '(Parenthetical)'
        common.request('html..')
        common.validate(input, intermediate1)
        common.validate(intermediate1, intermediate2)
        common.validate(intermediate2, output)
