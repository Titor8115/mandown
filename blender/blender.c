/*
 * Copyright (c) 2009, Natacha Porté
 * Copyright (c) 2011, Vicent Marti
 * Copyright (c) 2019, Tianze Han
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "blender.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../cli/mandown.h"
#include "houdini.h"
#include "markdown.h"

#define USE_BLENDER(opt) (opt->flags & blender_USE_BLENDER)

int sdblender_is_tag(const uint8_t *tag_data, size_t tag_size, const char *tagname) {
    size_t i;
    int closed = 0;

    if (tag_size < 3 || tag_data[0] != '<')
        return blender_TAG_NONE;

    i = 1;

    if (tag_data[i] == '/') {
        closed = 1;
        i++;
    }

    for (; i < tag_size; ++i, ++tagname) {
        if (*tagname == 0)
            break;

        if (tag_data[i] != *tagname)
            return blender_TAG_NONE;
    }

    if (i == tag_size)
        return blender_TAG_NONE;

    if (isspace(tag_data[i]) || tag_data[i] == '>')
        return closed ? blender_TAG_CLOSE : blender_TAG_OPEN;

    return blender_TAG_NONE;
}

static inline void escape_blender(struct buf *ob, const uint8_t *source, size_t length) {
    houdini_escape_blender0(ob, source, length, 0);
}

static inline void escape_href(struct buf *ob, const uint8_t *source, size_t length) {
    houdini_escape_href(ob, source, length);
}

/********************
 * GENERIC RENDERER *
 ********************/
static int
rndr_autolink(struct buf *ob, const struct buf *link, enum mkd_autolink type, void *opaque) {
    struct blender_renderopt *options = opaque;

    if (!link || !link->size)
        return 0;

    if ((options->flags & blender_SAFELINK) != 0 &&
        !sd_autolink_issafe(link->data, link->size) &&
        type != MKDA_EMAIL)
        return 0;

    BUFPUTSL(ob, "<a href=\"");
    if (type == MKDA_EMAIL)
        BUFPUTSL(ob, "mailto:");
    escape_href(ob, link->data, link->size);

    if (options->link_attributes) {
        bufputc(ob, '\"');
        options->link_attributes(ob, link, opaque);
        bufputc(ob, '>');
    } else {
        BUFPUTSL(ob, "\">");
    }

    /*
	 * Pretty printing: if we get an email address as
	 * an actual URI, e.g. `mailto:foo@bar.com`, we don't
	 * want to print the `mailto:` prefix
	 */
    if (bufprefix(link, "mailto:") == 0) {
        escape_blender(ob, link->data + 7, link->size - 7);
    } else {
        escape_blender(ob, link->data, link->size);
    }

    BUFPUTSL(ob, "</a>\n");

    return 1;
}

static void
rndr_blockcode(struct buf *ob, const struct buf *text, const struct buf *lang, void *opaque) {
    if (lang && lang->size) {
        size_t i, cls;
        BUFPUTSL(ob, "<pre><code class=\"");

        for (i = 0, cls = 0; i < lang->size; ++i, ++cls) {
            while (i < lang->size && isspace(lang->data[i]))
                i++;

            if (i < lang->size) {
                size_t org = i;
                while (i < lang->size && !isspace(lang->data[i]))
                    i++;

                if (lang->data[org] == '.')
                    org++;

                if (cls) bufputc(ob, ' ');
                escape_blender(ob, lang->data + org, i - org);
            }
        }

        BUFPUTSL(ob, "\">");
    } else
        BUFPUTSL(ob, "<pre><code>");

    if (text) {
        escape_blender(ob, text->data, text->size);
    }

    BUFPUTSL(ob, "</code></pre>\n");
}

static void
rndr_blockquote(struct buf *ob, const struct buf *text, void *opaque) {
    BUFPUTSL(ob, "<blockquote>\n");

    if (text) {
        bufput(ob, text->data, text->size);
    }
    BUFPUTSL(ob, "</blockquote>\n");
}

static int
rndr_codespan(struct buf *ob, const struct buf *text, void *opaque) {
    BUFPUTSL(ob, "<code>\n");
    if (text) escape_blender(ob, text->data, text->size);
    BUFPUTSL(ob, "</code>");
    return 1;
}

static int
rndr_strikethrough(struct buf *ob, const struct buf *text, void *opaque) {
    if (!text || !text->size)
        return 0;

    BUFPUTSL(ob, "<del>\n");
    bufput(ob, text->data, text->size);
    BUFPUTSL(ob, "\n</del>\n");
    return 1;
}

static int
rndr_double_emphasis(struct buf *ob, const struct buf *text, void *opaque) {
    if (!text || !text->size)
        return 0;

    BUFPUTSL(ob, "<strong>");
    bufput(ob, text->data, text->size);
    BUFPUTSL(ob, "</strong>\n");

    return 1;
}

static int
rndr_emphasis(struct buf *ob, const struct buf *text, void *opaque) {
    if (!text || !text->size) return 0;
    BUFPUTSL(ob, "<em>");
    if (text) bufput(ob, text->data, text->size);
    BUFPUTSL(ob, "</em>\n");
    return 1;
}

static int
rndr_linebreak(struct buf *ob, void *opaque) {
    struct blender_renderopt *options = opaque;
    bufputs(ob, USE_BLENDER(options) ? "<br/>\n" : "<br>\n");

    return 1;
}

static void
rndr_header(struct buf *ob, const struct buf *text, int level, void *opaque) {
    struct blender_renderopt *options = opaque;

    if (options->flags & blender_TOC)
        bufprintf(ob, "<h%d id=\"toc_%d\">", level, options->toc_data.header_count++);
    else {
        int i = 0;
        for (i = 0; i < level - 1; i++) {
            bufputc(ob, ' ');
            bufputc(ob, ' ');
            bufputc(ob, ' ');
        }
        // bufprintf(ob, "%.*c", (level - 1) * 3, ' ');
    }
    if (text) {
        bufput(ob, text->data, text->size);
    }
    bufputc(ob, '\n');
    // bufprintf(ob, "</h%d>\n", level);

} /* Syntax altered: mostly */

static int
rndr_link(struct buf *ob, const struct buf *link, const struct buf *title, const struct buf *content, void *opaque) {
    struct blender_renderopt *options = opaque;

    if (link != NULL && (options->flags & blender_SAFELINK) != 0 && !sd_autolink_issafe(link->data, link->size))
        return 0;

    BUFPUTSL(ob, "<a href=\"");

    if (link && link->size)
        escape_href(ob, link->data, link->size);

    if (title && title->size) {
        BUFPUTSL(ob, "\" title=\"");
        escape_blender(ob, title->data, title->size);
    }

    if (options->link_attributes) {
        bufputc(ob, '\"');
        options->link_attributes(ob, link, opaque);
        bufputc(ob, '>');
    } else {
        BUFPUTSL(ob, "\">");
    }

    if (content && content->size) bufput(ob, content->data, content->size);
    BUFPUTSL(ob, "\n</a>\n");
    return 1;
}

static void
rndr_list(struct buf *ob, const struct buf *text, int flags, void *opaque) {
    bufputs(ob, flags & MKD_LIST_ORDERED ? "<ol>\n" : "<ul>\n");
    if (text) bufput(ob, text->data, text->size);
    bufputs(ob, flags & MKD_LIST_ORDERED ? "</ol>\n" : "</ul>\n");
}

static void
rndr_listitem(struct buf *ob, const struct buf *text, int flags, void *opaque) {
    // BUFPUTSL(ob, "<li>\n");
    BUFPUTSL(ob, "\t· ");
    if (text) {
        size_t size = text->size;
        while (size && text->data[size - 1] == '\n') {
            size--;
        }
        bufput(ob, text->data, size);
    }
    bufputc(ob, '\n');
} /* Syntax altered */

static void
rndr_paragraph(struct buf *ob, const struct buf *text, void *opaque) {
    struct blender_renderopt *options = opaque;
    size_t i = 0;

    if (!text || !text->size)
        return;

    while (i < text->size && isspace(text->data[i])) i++;

    if (i == text->size)
        return;
    bufputc(ob, '\t');
    if (options->flags & blender_HARD_WRAP) {
        size_t org;
        while (i < text->size) {
            org = i;
            while (i < text->size && text->data[i] != '\n')
                i++;

            if (i > org) {
                bufput(ob, text->data + org, i - org);
            }

            /*
			 * do not insert a line break if this newline
			 * is the last character on the paragraph
			 */
            if (i >= text->size - 1)
                break;

            rndr_linebreak(ob, opaque);
            i++;
        }
    } else {
        bufput(ob, &text->data[i], text->size - i);
    }
    bufputc(ob, '\n');
    bufputc(ob, '\n');

} /* Syntax altered */

static void
rndr_raw_block(struct buf *ob, const struct buf *text, void *opaque) {
    size_t org, sz;
    if (!text) return;
    sz = text->size;
    while (sz > 0 && text->data[sz - 1] == '\n') sz--;
    org = 0;
    while (org < sz && text->data[org] == '\n') org++;
    if (org >= sz) return;
    bufput(ob, text->data + org, sz - org);
}

static int
rndr_triple_emphasis(struct buf *ob, const struct buf *text, void *opaque) {
    if (!text || !text->size) return 0;
    BUFPUTSL(ob, "<strong>\n<em>\n");
    bufput(ob, text->data, text->size);
    BUFPUTSL(ob, "\n</em>\n</strong>\n");
    return 1;
}

static void
rndr_hrule(struct buf *ob, void *opaque) {
    struct blender_renderopt *options = opaque;
    bufputs(ob, USE_BLENDER(options) ? "<hr/>\n" : "<hr>\n");
}

static int
rndr_image(struct buf *ob, const struct buf *link, const struct buf *title, const struct buf *alt, void *opaque) {
    struct blender_renderopt *options = opaque;
    if (!link || !link->size) return 0;

    BUFPUTSL(ob, "<img src=\"");
    escape_href(ob, link->data, link->size);
    BUFPUTSL(ob, "\" alt=\"");

    if (alt && alt->size)
        escape_blender(ob, alt->data, alt->size);

    if (title && title->size) {
        BUFPUTSL(ob, "\" title=\"");
        escape_blender(ob, title->data, title->size);
    }

    bufputs(ob, USE_BLENDER(options) ? "\"/>" : "\">");
    return 1;
}

static int
rndr_raw_blender(struct buf *ob, const struct buf *text, void *opaque) {
    struct blender_renderopt *options = opaque;

    /* blender_ESCAPE overrides SKIP_blender, SKIP_STYLE, SKIP_LINKS and SKIP_IMAGES
	* It doens't see if there are any valid tags, just escape all of them. */
    if ((options->flags & blender_ESCAPE) != 0) {
        escape_blender(ob, text->data, text->size);
        return 1;
    }

    if ((options->flags & blender_SKIP_blender) != 0)
        return 1;

    if ((options->flags & blender_SKIP_STYLE) != 0 &&
        sdblender_is_tag(text->data, text->size, "style"))
        return 1;

    if ((options->flags & blender_SKIP_LINKS) != 0 &&
        sdblender_is_tag(text->data, text->size, "a"))
        return 1;

    if ((options->flags & blender_SKIP_IMAGES) != 0 &&
        sdblender_is_tag(text->data, text->size, "img"))
        return 1;

    bufput(ob, text->data, text->size);
    return 1;
}

static void
rndr_table(struct buf *ob, const struct buf *header, const struct buf *body, void *opaque) {
    BUFPUTSL(ob, "<table>\n<thead>\n");

    if (header) {
        bufput(ob, header->data, header->size);
    }
    BUFPUTSL(ob, "\n</thead>\n<tbody>\n");

    if (body) {
        bufput(ob, body->data, body->size);
    }
    BUFPUTSL(ob, "\n</tbody>\n</table>\n");
}

static void
rndr_tablerow(struct buf *ob, const struct buf *text, void *opaque) {
    BUFPUTSL(ob, "<tr>\n");

    if (text) {
        bufput(ob, text->data, text->size);
    }
    BUFPUTSL(ob, "\n</tr>\n");
}

static void
rndr_tablecell(struct buf *ob, const struct buf *text, int flags, void *opaque) {
    if (flags & MKD_TABLE_HEADER) {
        BUFPUTSL(ob, "<th");
    } else {
        BUFPUTSL(ob, "<td");
    }

    switch (flags & MKD_TABLE_ALIGNMASK) {
        case MKD_TABLE_ALIGN_CENTER:
            BUFPUTSL(ob, " align=\"center\">");
            break;

        case MKD_TABLE_ALIGN_L:
            BUFPUTSL(ob, " align=\"left\">");
            break;

        case MKD_TABLE_ALIGN_R:
            BUFPUTSL(ob, " align=\"right\">");
            break;

        default:
            BUFPUTSL(ob, ">");
    }

    if (text) {
        bufput(ob, text->data, text->size);
    }
    if (flags & MKD_TABLE_HEADER) {
        BUFPUTSL(ob, "\n</th>\n");

    } else {
        BUFPUTSL(ob, "\n</td>\n");
    }
}

static int
rndr_superscript(struct buf *ob, const struct buf *text, void *opaque) {
    if (!text || !text->size) return 0;
    BUFPUTSL(ob, "<sup>\n");
    bufput(ob, text->data, text->size);
    BUFPUTSL(ob, "\n</sup>\n");
    return 1;
}

static void
rndr_normal_text(struct buf *ob, const struct buf *text, void *opaque) {
    if (text)
        escape_blender(ob, text->data, text->size);
}

static void
toc_header(struct buf *ob, const struct buf *text, int level, void *opaque) {
    struct blender_renderopt *options = opaque;

    /* set the level offset if this is the first header
	 * we're parsing for the document */
    if (options->toc_data.current_level == 0) {
        options->toc_data.level_offset = level - 1;
    }
    level -= options->toc_data.level_offset;

    if (level > options->toc_data.current_level) {
        while (level > options->toc_data.current_level) {
            BUFPUTSL(ob, "<ul>\n<li>\n");
            options->toc_data.current_level++;
        }
    } else if (level < options->toc_data.current_level) {
        BUFPUTSL(ob, "\n</li>\n");

        while (level < options->toc_data.current_level) {
            BUFPUTSL(ob, "\n</ul>\n</li>\n");
            options->toc_data.current_level--;
        }
        BUFPUTSL(ob, "<li>\n");

    } else {
        BUFPUTSL(ob, "\n</li>\n<li>\n");
    }

    bufprintf(ob, "<a href=\"#toc_%d\">", options->toc_data.header_count++);
    if (text) {
        escape_blender(ob, text->data, text->size);
    }
    BUFPUTSL(ob, "\n</a>\n");
}

static int
toc_link(struct buf *ob, const struct buf *link, const struct buf *title, const struct buf *content, void *opaque) {
    if (content && content->size)
        bufput(ob, content->data, content->size);
    return 1;
}

static void
toc_finalize(struct buf *ob, void *opaque) {
    struct blender_renderopt *options = opaque;

    while (options->toc_data.current_level > 0) {
        BUFPUTSL(ob, "\n</li>\n</ul>\n");
        options->toc_data.current_level--;
    }
}

void sdblender_toc_renderer(struct sd_callbacks *callbacks, struct blender_renderopt *options) {
    static const struct sd_callbacks cb_default = {
        NULL,
        NULL,
        NULL,
        toc_header,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,

        NULL,
        rndr_codespan,
        rndr_double_emphasis,
        rndr_emphasis,
        NULL,
        NULL,
        toc_link,
        NULL,
        rndr_triple_emphasis,
        rndr_strikethrough,
        rndr_superscript,

        NULL,
        NULL,

        NULL,
        toc_finalize,
    };

    memset(options, 0x0, sizeof(struct blender_renderopt));
    options->flags = blender_TOC;

    memcpy(callbacks, &cb_default, sizeof(struct sd_callbacks));
}

void sdblender_renderer(struct sd_callbacks *callbacks, struct blender_renderopt *options, unsigned int render_flags) {
    static const struct sd_callbacks cb_default = {
        rndr_blockcode,
        rndr_blockquote,
        rndr_raw_block,
        rndr_header,
        rndr_hrule,
        rndr_list,
        rndr_listitem,
        rndr_paragraph,
        rndr_table,
        rndr_tablerow,
        rndr_tablecell,

        rndr_autolink,
        rndr_codespan,
        rndr_double_emphasis,
        rndr_emphasis,
        rndr_image,
        rndr_linebreak,
        rndr_link,
        rndr_raw_blender,
        rndr_triple_emphasis,
        rndr_strikethrough,
        rndr_superscript,

        NULL,
        rndr_normal_text,

        NULL,
        NULL,
    };

    /* Prepare the options pointer */
    memset(options, 0x0, sizeof(struct blender_renderopt));
    options->flags = render_flags;

    /* Prepare the callbacks */
    memcpy(callbacks, &cb_default, sizeof(struct sd_callbacks));

    if (render_flags & blender_SKIP_IMAGES)
        callbacks->image = NULL;

    if (render_flags & blender_SKIP_LINKS) {
        callbacks->link = NULL;
        callbacks->autolink = NULL;
    }

    if (render_flags & blender_SKIP_blender || render_flags & blender_ESCAPE)
        callbacks->blockblender = NULL;
}