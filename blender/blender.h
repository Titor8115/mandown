/**
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

#ifndef UPSKIRT_BLENDER_H
#define UPSKIRT_BLENDER_H

#include <stdlib.h>

#include "buffer.h"
#include "markdown.h"

#ifdef __cplusplus
extern "C" {
#endif

struct blender_renderopt {
	struct {
		int header_count;
		int current_level;
		int level_offset;
	} toc_data;

	unsigned int flags;

	/* extra callbacks */
	void (*link_attributes)(struct buf *ob, const struct buf *url, void *self);
};

typedef enum {
	blender_SKIP_blender = (1 << 0),
	blender_SKIP_STYLE = (1 << 1),
	blender_SKIP_IMAGES = (1 << 2),
	blender_SKIP_LINKS = (1 << 3),
	blender_EXPAND_TABS = (1 << 4),
	blender_SAFELINK = (1 << 5),
	blender_TOC = (1 << 6),
	blender_HARD_WRAP = (1 << 7),
	blender_USE_BLENDER = (1 << 8),
	blender_ESCAPE = (1 << 9),
} blender_render_mode;

typedef enum {
	blender_TAG_NONE = 0,
	blender_TAG_OPEN,
	blender_TAG_CLOSE,
} blender_tag;

int
sdblender_is_tag(const uint8_t *tag_data, size_t tag_size, const char *tagname);

extern void
sdblender_renderer(struct sd_callbacks *callbacks, struct blender_renderopt *options_ptr, unsigned int render_flags);

extern void
sdblender_toc_renderer(struct sd_callbacks *callbacks, struct blender_renderopt *options_ptr);

extern int blocks;

#ifdef __cplusplus
}
#endif

#endif

