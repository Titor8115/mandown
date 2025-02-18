#ifndef MDN_LIB_H
#define MDN_LIB_H

#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * render_XXX: renders input to screen or file
 *   fp    = open FILE *
 *   str   = input string
 *   ext   = type of buffer data
 *           (supported: "html", "md", "txt")
 *   title = title to display
 *   out   = filename of output (NULL = display on screen)
 */
int render_file(FILE *fp, const char *ext, const char *title, const char *out);
int render_str(const char *, const char *ext, const char *title, const char *out);

#ifdef __cplusplus
}
#endif

#endif /* MDN_LIB_H */
