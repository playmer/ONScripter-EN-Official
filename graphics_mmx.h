/* -*- C++ -*-
 * 
 *  graphics_mmx.h - graphics routines using MMX cpu functionality
 *
 *  Copyright (c) 2009-2011 "Uncle" Mion Sonozaki
 *
 *  UncleMion@gmail.com
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, see <http://www.gnu.org/licenses/>
 *  or write to the Free Software Foundation, Inc.,
 *  59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef __GRAPHICS_MMX_H__
#define __GRAPHICS_MMX_H__

#ifdef ONS_X86_MMX_AVAILIBLE
namespace ons_gfx {

void imageFilterMean_MMX(uint8_t *src1, uint8_t *src2, uint8_t *dst, int length);
void imageFilterAddTo_MMX(uint8_t *dst, uint8_t *src, int length);
void imageFilterSubFrom_MMX(uint8_t *dst, uint8_t *src, int length);

}
#endif //ONS_X86_MMX_AVAILIBLE

#endif // __GRAPHICS_SSE2_H__
