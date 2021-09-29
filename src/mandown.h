/**
 * Copyright (C) 2019 Tianze Han
 * 
 * This file is part of Mandown.
 * 
 * Mandown is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * Mandown is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Mandown.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MDN_MANDOWN_H
#define MDN_MANDOWN_H

#include "buffer.h"
#include "config.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PAGE_MODE 0
#define FILE_MODE 1

extern struct mdn_cfg *configure();
extern int view(const struct mdn_cfg *, const struct buf *, int);

#ifdef __cplusplus
}
#endif

#endif /* MDN_MANDOWN_H */