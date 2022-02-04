/* Conversion of files between different charsets and surfaces.
   Copyright © 1999, 2000, 2001, 2008 Free Software Foundation, Inc.
   Contributed by François Pinard <pinard@iro.umontreal.ca>, 1999,
   and Bruno Haible <haible@clisp.cons.org>, 2000.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License
   as published by the Free Software Foundation; either version 3 of the
   License, or (at your option) any later version.

   This library is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the Recode Library; see the file `COPYING.LIB'.
   If not, see <https://www.gnu.org/licenses/>.
*/

#include "config.h"
#include "common.h"
#include "decsteps.h"
#include <iconv.h>
#include "iconvdecl.h"

/*--------------------------------------.
| Use `iconv' to handle a double step.  |
`--------------------------------------*/

#define BUFFER_SIZE 2048

static bool
wrapped_transform (iconv_t conversion, RECODE_SUBTASK subtask)
{
  char output_buffer[BUFFER_SIZE];
  char input_buffer[BUFFER_SIZE];
  int input_char = get_byte (subtask);
  char *cursor = input_buffer;
  bool drain_first = false;

  while (true)
    {
      /* The output buffer is fully available at this point.  */

      char *input = input_buffer;
      char *output = output_buffer;
      size_t input_left = 0;
      size_t output_left = BUFFER_SIZE;
      int saved_errno = 0;
      size_t converted;

      if (drain_first)
        {
          /* Drain all accumulated partial state and emit output
             to return to the initial shift state.  */
          converted = iconv (conversion, NULL, NULL, &output, &output_left);
          if (converted == (size_t) -1)
            saved_errno = errno;
        }

      if (saved_errno == 0)
        {
          /* Continue filling the input buffer.  */
          while (input_char != EOF && cursor < input_buffer + BUFFER_SIZE)
            {
              *cursor++ = input_char;
              input_char = get_byte (subtask);
            }

          if (cursor == input_buffer)
            {
              if (output == output_buffer)
                {
                  /* All work has been done, just make sure we drained.  */
                  if (drain_first)
                    break;
                  drain_first = true;
                  continue;
                }
            }
          else
            {
              /* Convert accumulated input and add it to the output buffer.  */
              input = input_buffer;
              input_left = cursor - input_buffer;
              converted = iconv (conversion,
                                 &input, &input_left,
                                 &output, &output_left);
              if (converted == (size_t) -1)
                saved_errno = errno;
            }
        }

      /* Send the converted result, so freeing the output buffer.  */
      for (cursor = output_buffer; cursor < output; cursor++)
        put_byte (*cursor, subtask);

      /* Act according to the outcome of the iconv call.  */

      drain_first = false;
      if (saved_errno != 0 && saved_errno != E2BIG)
	{
	  if (saved_errno == EILSEQ)
	    {
	      /* Check whether the input was really just untranslatable.  */
              enum recode_error recode_error = RECODE_INVALID_INPUT;
	      RECODE_CONST_STEP step = subtask->step;
	      iconv_t check_conversion = iconv_open (step->before->iconv_name,
						     step->before->iconv_name);

	      /* On error, give up and assume input is invalid.  */
	      if (input_left > 0 && check_conversion != (iconv_t) -1)
		{
                  /* Assume iconv does not modify its input.  */
		  char *check_input = input;
		  size_t check_input_left = input_left;
                  size_t check_output_left = input_left;
		  char *check_output_buffer, *check_output;
                  RECODE_OUTER outer = subtask->task->request->outer;

                  if ((check_output = ALLOC (check_output_buffer, input_left, char)) != NULL)
                    {
                      size_t check_converted = iconv (check_conversion,
                                                      &check_input, &check_input_left,
                                                      &check_output, &check_output_left);

                      if (check_converted != (size_t) -1)
                        recode_error = RECODE_UNTRANSLATABLE;

                      free (check_output_buffer);
                    }

                  iconv_close (check_conversion);
		}

	      /* Invalid or untranslatable input.  */
	      RETURN_IF_NOGO (recode_error, subtask);
	    }
	  else if (saved_errno == EINVAL)
	    {
	      if (input + input_left < input_buffer + BUFFER_SIZE
		  && input_char == EOF)
		/* Incomplete multibyte sequence at end of input.  */
		RETURN_IF_NOGO (RECODE_INVALID_INPUT, subtask);
	    }
	  else
	    {
	      recode_perror (subtask->task->request->outer, "iconv ()");
	      RETURN_IF_NOGO (RECODE_SYSTEM_ERROR, subtask);
	    }
	}

      /* Move back any unprocessed part of the input buffer.  */
      for (cursor = input_buffer; input_left != 0; input_left--)
        *cursor++ = *input++;
    }

  SUBTASK_RETURN (subtask);
}

static bool
ends_with (const char *s, size_t s_len, const char *suff, size_t suff_len)
{
  return suff_len <= s_len && !memcmp (s + s_len - suff_len, suff, suff_len);
}

static char *
iconv_fix_options (const char *charset)
{
  size_t charset_len = strlen (charset);
  bool ignore = false, translit = false;

  do {
    if (ends_with (charset, charset_len, "-translit", strlen ("-translit")))
      {
        translit = true;
        charset_len -= strlen ("-translit");
      }
    else if (ends_with (charset, charset_len, "-ignore", strlen ("-ignore")))
      {
        ignore = true;
        charset_len -= strlen ("-ignore");
      }
    else
      break;
  } while (true);

  char *result;
  if (asprintf (&result, "%.*s%s%s", (int) charset_len, charset,
                translit ? "//TRANSLIT" : "",
                ignore ? "//IGNORE": "")
      == -1)
    return NULL;
  return result;
}

bool
transform_with_iconv (RECODE_SUBTASK subtask)
{
  RECODE_CONST_STEP step = subtask->step;
  char *tocode = iconv_fix_options (step->after->iconv_name);
  char *fromcode = iconv_fix_options (step->before->iconv_name);
  iconv_t conversion = (iconv_t) -1;

  if (tocode && fromcode)
    conversion = iconv_open (tocode, fromcode);
  if (conversion == (iconv_t) -1)
    {
      recode_if_nogo (RECODE_SYSTEM_ERROR, subtask);
      free (fromcode);
      free (tocode);
      SUBTASK_RETURN (subtask);
    }

  bool status = wrapped_transform (conversion, subtask);
  iconv_close (conversion);
  free (fromcode);
  free (tocode);
  return status;
}

/*------------------------------------------------------.
| Declare all character sets which `iconv' may handle.  |
`------------------------------------------------------*/

bool
module_iconv (RECODE_OUTER outer)
{
  const char **cursor;

  for (cursor = iconv_name_list; *cursor; cursor++)
    {
      const char **aliases = cursor;
      const char *charset_name = *cursor;

      /* Scan aliases for some charset which would already be known.  If any,
	 use its official name as a charset.  Else, use the first alias.  */

      while (*cursor)
	{
          RECODE_ALIAS alias
            = find_alias (outer, *cursor, ALIAS_FIND_AS_CHARSET);

	  if (alias)
	    {
	      charset_name = alias->symbol->name;
	      break;
	    }
	  cursor++;
	}

      if (!declare_iconv (outer, charset_name, *aliases))
	return false;

      /* Declare all aliases, given they bring something we do not already
	 know.  Even then, we still declare too many useless aliases, as the
	 disambiguating tables are not recomputed as we go.  FIXME!  */

      for (cursor = aliases; *cursor; cursor++)
	{
	  RECODE_ALIAS alias
	    = find_alias (outer, *cursor, ALIAS_FIND_AS_CHARSET);

	  /* If there is a charset contradiction, call declare_alias
	     nevertheless, as the error processing will occur there.  */
	  if (!alias || alias->symbol->name != charset_name)
	    if (!declare_alias (outer, *cursor, charset_name))
	      return false;
	}
    }

  return true;
}

void
delmodule_iconv (_GL_UNUSED RECODE_OUTER outer)
{
}
