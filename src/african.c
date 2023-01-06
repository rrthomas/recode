/* Traitement de certaines tables africaines.
   Copyright © 1999-2000 Progiciels Bourbeau-Pinard inc.
   François Pinard <pinard@iro.umontreal.ca>, 1997.  */

#include "config.h"
#include "common.h"
#include "decsteps.h"

/* Bambara, Ewondo and Fulfude.  */

static const unsigned short ful_data[] =
{
  161, 0x0181, DONE,
  162, 0x018A, DONE,
  163, 0x0194, DONE,
  164, 0x0198, DONE,
  165, 0x01A4, DONE,
  166, 0x01AC, DONE,
  167, 0x01A9, DONE,
  168, 0x019D, DONE,
  169, 0x014A, DONE,
  170, 0x0189, DONE,
  172, 0x0191, DONE,
  174, 0x01B3, DONE,
  175, 0x0193, DONE,
  177, 0x0253, DONE,
  178, 0x0257, DONE,
  179, 0x0263, DONE,
  180, 0x0199, DONE,
  181, 0x01A5, DONE,
  182, 0x01AD, DONE,
  183, 0x0283, DONE,
  184, 0x0272, DONE,
  185, 0x014B, DONE,
  186, 0x0256, DONE,
  188, 0x0192, DONE,
  189, 0x0294, DONE,
  190, 0x01B4, DONE,
  191, 0x0260, DONE,
  197, 0x018E, DONE,
  208, 0x0190, DONE,
  215, 0x0152, DONE,
  216, 0x0186, DONE,
  221, 0x018E, 0x0301, DONE,
  222, 0x0186, 0x0301, DONE,
  223, 0x01B2, DONE,
  229, 0x0259, DONE,
  240, 0x025B, DONE,
  247, 0x0153, DONE,
  248, 0x0254, DONE,
  253, 0x0259, 0x0301, DONE,
  254, 0x0254, 0x0301, DONE,
  255, 0x028B, DONE,
  DONE
};

/* Linguala, Sango and Wolof.  */

static const unsigned short lin_data[] =
{
  161, 0x0190, 0x0300, DONE,
  162, 0x0190, 0x0301, DONE,
  163, 0x0190, 0x0302, DONE,
  164, 0x0190, 0x030C, DONE,
  165, 0x0186, 0x0300, DONE,
  166, 0x0186, 0x0301, DONE,
  167, 0x0186, 0x0302, DONE,
  168, 0x019D, DONE,
  169, 0x014A, DONE,
  170, 0x004E, 0x0302, DONE,
  172, 0x004E, 0x0308, DONE,
  174, 0x01B3, DONE,
  175, 0x0186, 0x030C, DONE,
  177, 0x025B, 0x0300, DONE,
  178, 0x025B, 0x0301, DONE,
  179, 0x025B, 0x0302, DONE,
  180, 0x025B, 0x030C, DONE,
  181, 0x0254, 0x0300, DONE,
  182, 0x0254, 0x0301, DONE,
  183, 0x0254, 0x0302, DONE,
  184, 0x0272, DONE,
  185, 0x014B, DONE,
  186, 0x006E, 0x0302, DONE,
  188, 0x006E, 0x0308, DONE,
  189, 0x0294, DONE,
  190, 0x01B4, DONE,
  191, 0x0254, 0x030C, DONE,
  195, 0x01CD, DONE,
  197, 0x018E, DONE,
  208, 0x0190, DONE,
  213, 0x01D1, DONE,
  215, 0x0152, DONE,
  216, 0x0186, DONE,
  221, 0x011A, DONE,
  222, 0x01CF, DONE,
  223, 0x01D3, DONE,
  227, 0x01CE, DONE,
  229, 0x0259, DONE,
  240, 0x025B, DONE,
  245, 0x01D2, DONE,
  247, 0x0153, DONE,
  248, 0x0254, DONE,
  253, 0x011B, DONE,
  254, 0x01D0, DONE,
  255, 0x01D4, DONE,
  DONE
};

bool
module_african (RECODE_OUTER outer)
{
  return declare_explode_data (outer, ful_data, "AFRFUL-102-BPI_OCIL", NULL)
    && declare_alias (outer, "bambara", "AFRFUL-102-BPI_OCIL")
    && declare_alias (outer, "bra", "AFRFUL-102-BPI_OCIL")
    && declare_alias (outer, "ewondo", "AFRFUL-102-BPI_OCIL")
    && declare_alias (outer, "fulfulde", "AFRFUL-102-BPI_OCIL")
    && declare_explode_data (outer, lin_data, "AFRLIN-104-BPI_OCIL", NULL)
    && declare_alias (outer, "lingala", "AFRLIN-104-BPI_OCIL")
    && declare_alias (outer, "lin", "AFRLIN-104-BPI_OCIL")
    && declare_alias (outer, "sango", "AFRLIN-104-BPI_OCIL")
    && declare_alias (outer, "wolof", "AFRLIN-104-BPI_OCIL");
}

void
delmodule_african (_GL_UNUSED RECODE_OUTER outer)
{
}
