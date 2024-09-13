# -*- coding: utf-8 -*-
import common
from common import setup_module, teardown_module

# Some characters were lost while u8..l1 recoding goes.
# Submitted 2001-09-28 by Volker Wysk <post@volker-wysk.de>.

input = r'''\
:- module(inferenz, [anfrage/2, anfrage/1, anfrage_l/3, anfrage_l/2,
                     ohne/4,
                     loese/4, loese/6, loese_pos/6
                    ]).

:- module_transparent(loese/4).

loese(Term, Sich, Schr, Prot) :-
        (
                    Term = (A,B)
        *->
          call(Schr, tzk, loese(A), S_1, Prot_1),
          call(Schr, tzk, loese(B), S_2, Prot_2),
          Sich is min(S_1, S_2),
          Prot = k:(Prot_1,Prot_2)
        ;
          Term = (A;B)
        *->
          ( call(Schr, tzd, loese(A), Sich, Prot_1)
          ; call(Schr, tzd, loese(B), Sich, Prot_1)
          ),
          Prot = d:Prot_1
        ;
          Term = (\+ (A,B))
        *->
          verneint(A,A_1),
          verneint(B,B_1),
          call(Schr, umf, loese(A_1;B_1), Sich, Prot)
        ;
          Term = (\+ (A;B))
        *->
          verneint(A,A_1),
          verneint(B,B_1),
          call(Schr, umf, loese(A_1,B_1), Sich, Prot)
        ;
          Term = (\+ \+ Term_1)
        *->
          call(Schr, umf, loese(Term_1), Sich, Prot)
        ;
          Term = true
        *->
          Sich = 1,
          Prot = triv:''
        ;
          Term = (\+ true)
        *->
          fail

        ;
          % Prädikat
          ( Term = (+> Term_1)
          ->
              % positive Anfrage
              Fkt = loese_pos(Grad, Vern, Kern)
          ;
              % abwägende Anfrage
              Term_1 = Term,
              Fkt = loese(Grad, Vern, Kern)
          ),

          % Anfrage ausführen
          zerlegen(Term_1, ('', Grad, Vern, Kern, '')),
          call(Schr, z, Fkt, Sich, Prot),
          Sich > 0
        ).
loese(Grad, Vern, Kern, Sich, Schr, Prot) :-
        (
                    Vern = true, Kern = (A = B)   *-> A = B,     Sich = 1
        ; Vern = true, Kern = (A \= B)  *-> A \= B,    Sich = 1
        ; Vern = true, Kern = (A > B)   *-> A > B,     Sich = 1
        ; Vern = true, Kern = (A < B)   *-> A < B,     Sich = 1
        ; Vern = true, Kern = (A >= B)  *-> A >= B,    Sich = 1
        ; Vern = true, Kern = (A =< B)  *-> A =< B,    Sich = 1
        ; Vern = true, Kern = (A is B)  *-> A is B,    Sich = 1
        ; Vern = fail, Kern = (A = B)   *-> A \= B,    Sich = 1
'''

def test_1():
    import sys
    common.request('u8..l1')
    output = common.recode_iconv_output(input)
    expected = b''.join(input.encode('latin-1').splitlines(True)[-6:])
    output = b''.join(output.splitlines(True)[-6:])
    common.assert_or_diff(output, expected)
