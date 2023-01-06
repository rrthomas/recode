/* Conversion of files between different charsets and surfaces.
   Copyright © 2008-2018 Free Software Foundation, Inc.
   Contributed by François Pinard <pinard@iro.umontreal.ca>, 2008.

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

/* Definitions to clean up the linker namespace.  */

#ifndef CLEANER_H_
# define CLEANER_H_

# define clone_quoting_options librecode_clone_quoting_options
# define code_to_ucs2 librecode_code_to_ucs2
# define combine_byte_byte librecode_combine_byte_byte
# define combine_byte_ucs2 librecode_combine_byte_ucs2
# define combine_ucs2_byte librecode_combine_ucs2_byte
# define combine_ucs2_ucs2 librecode_combine_ucs2_ucs2
# define complete_pairs librecode_complete_pairs
# define declare_alias librecode_declare_alias
# define declare_explode_data librecode_declare_explode_data
# define declare_iconv librecode_declare_iconv
# define declare_implied_surface librecode_declare_implied_surface
# define declare_single librecode_declare_single
# define declare_strip_data librecode_declare_strip_data
# define decode_known_pairs librecode_decode_known_pairs
# define delmodule_african librecode_delmodule_african
# define delmodule_afrtran librecode_delmodule_afrtran
# define delmodule_applemac librecode_delmodule_applemac
# define delmodule_ascii_latin1 librecode_delmodule_ascii_latin1
# define delmodule_atarist librecode_delmodule_atarist
# define delmodule_bangbang librecode_delmodule_bangbang
# define delmodule_base64 librecode_delmodule_base64
# define delmodule_cdcnos librecode_delmodule_cdcnos
# define delmodule_dump librecode_delmodule_dump
# define delmodule_ebcdic librecode_delmodule_ebcdic
# define delmodule_endline librecode_delmodule_endline
# define delmodule_flat librecode_delmodule_flat
# define delmodule_html librecode_delmodule_html
# define delmodule_ibmpc librecode_delmodule_ibmpc
# define delmodule_iconqnx librecode_delmodule_iconqnx
# define delmodule_iconv librecode_delmodule_iconv
# define delmodule_latex_latin1 librecode_delmodule_latex_latin1
# define delmodule_latin1_ascii librecode_delmodule_latin1_ascii
# define delmodule_latin1_latex librecode_delmodule_latin1_latex
# define delmodule_latin1_texte librecode_delmodule_latin1_texte
# define delmodule_mule librecode_delmodule_mule
# define delmodule_permutations librecode_delmodule_permutations
# define delmodule_quoted_printable librecode_delmodule_quoted_printable
# define delmodule_rfc1345 librecode_delmodule_rfc1345
# define delmodule_strips librecode_delmodule_strips
# define delmodule_testdump librecode_delmodule_testdump
# define delmodule_texinfo librecode_delmodule_texinfo
# define delmodule_texte_latin1 librecode_delmodule_texte_latin1
# define delmodule_ucs librecode_delmodule_ucs
# define delmodule_utf16 librecode_delmodule_utf16
# define delmodule_utf7 librecode_delmodule_utf7
# define delmodule_utf8 librecode_delmodule_utf8
# define delmodule_varia librecode_delmodule_varia
# define delmodule_vietnamese librecode_delmodule_vietnamese
# define edit_sequence librecode_edit_sequence
# define explode_byte_byte librecode_explode_byte_byte
# define explode_byte_ucs2 librecode_explode_byte_ucs2
# define explode_ucs2_byte librecode_explode_ucs2_byte
# define explode_ucs2_ucs2 librecode_explode_ucs2_ucs2
# define find_alias librecode_find_alias
# define find_and_report_subsets librecode_find_and_report_subsets
# define get_byte librecode_get_byte
# define put_byte librecode_put_byte
# define get_quoting_style librecode_get_quoting_style
# define get_ucs2 librecode_get_ucs2
# define get_ucs4 librecode_get_ucs4
# define init_combine librecode_init_combine
# define init_explode librecode_init_explode
# define init_ucs2_to_byte librecode_init_ucs2_to_byte
# define invert_table librecode_invert_table
# define list_all_symbols librecode_list_all_symbols
# define list_concise_charset librecode_list_concise_charset
# define list_full_charset librecode_list_full_charset
# define module_african librecode_module_african
# define module_afrtran librecode_module_afrtran
# define module_applemac librecode_module_applemac
# define module_ascii_latin1 librecode_module_ascii_latin1
# define module_atarist librecode_module_atarist
# define module_bangbang librecode_module_bangbang
# define module_base64 librecode_module_base64
# define module_cdcnos librecode_module_cdcnos
# define module_dump librecode_module_dump
# define module_ebcdic librecode_module_ebcdic
# define module_endline librecode_module_endline
# define module_flat librecode_module_flat
# define module_html librecode_module_html
# define module_ibmpc librecode_module_ibmpc
# define module_iconqnx librecode_module_iconqnx
# define module_iconv librecode_module_iconv
# define module_latex_latin1 librecode_module_latex_latin1
# define module_latin1_ascii librecode_module_latin1_ascii
# define module_latin1_latex librecode_module_latin1_latex
# define module_latin1_texte librecode_module_latin1_texte
# define module_mule librecode_module_mule
# define module_permutations librecode_module_permutations
# define module_quoted_printable librecode_module_quoted_printable
# define module_rfc1345 librecode_module_rfc1345
# define module_strips librecode_module_strips
# define module_testdump librecode_module_testdump
# define module_texinfo librecode_module_texinfo
# define module_texte_latin1 librecode_module_texte_latin1
# define module_ucs librecode_module_ucs
# define module_utf16 librecode_module_utf16
# define module_utf7 librecode_module_utf7
# define module_utf8 librecode_module_utf8
# define module_varia librecode_module_varia
# define module_vietnamese librecode_module_vietnamese
# define prepare_for_aliases librecode_prepare_for_aliases
# define put_ucs2 librecode_put_ucs2
# define put_ucs4 librecode_put_ucs4
# define reversibility librecode_reversibility
# define set_char_quoting librecode_set_char_quoting
# define set_quoting_style librecode_set_quoting_style
# define should_prefer_french librecode_should_prefer_french
# define texte_latin1_diaeresis librecode_texte_latin1_diaeresis
# define transform_byte_to_byte librecode_transform_byte_to_byte
# define transform_byte_to_ucs2 librecode_transform_byte_to_ucs2
# define transform_byte_to_variable librecode_transform_byte_to_variable
# define transform_ucs2_to_byte librecode_transform_ucs2_to_byte
# define transform_with_iconv librecode_transform_with_iconv
# define ucs2_to_charname librecode_ucs2_to_charname
# define ucs2_to_french_charname librecode_ucs2_to_french_charname
# define ucs2_to_rfc1345 librecode_ucs2_to_rfc1345

#endif /* ! CLEANER_H_ */
