/* -*- C++ -*-
 * 
 *  graphics_routines.cpp - ons_gfx namespace interface functions for
 *                          image blends and resizing
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

#include "graphics_cpu.h"
#include "graphics_sum.h"
#include "graphics_blend.h"
#include "graphics_resize.h"

#if defined(USE_X86_GFX)
#include "graphics_mmx.h"
#include "graphics_sse2.h"
#endif

#if defined(USE_PPC_GFX)
#include "graphics_altivec.h"
#endif

#include "resize_image.h"

namespace ons_gfx {

//Mion: for special graphics routine handling
static unsigned int cpufuncs;
static void (*imageFilterMeanFn)(unsigned char *src1, unsigned char *src2, unsigned char *dst, int length) = NULL;
static void (*imageFilterAddToFn)(unsigned char *dst, unsigned char *src, int length) = NULL;
static void (*imageFilterSubFromFn)(unsigned char *dst, unsigned char *src, int length) = NULL;
static void (*imageFilterBlendFn)(Uint32 *dst_buffer, Uint32 *src_buffer, Uint8 *alphap, int alpha, int length) = NULL;
static void (*imageFilterEffectBlendFn)(Uint32 *dst_buffer, Uint32 *src1_buffer, Uint32 *src2_buffer, Uint32 mask2, int length) = NULL;
static void (*imageFilterEffectMaskBlendFn)(Uint32 *dst_buffer, Uint32 *src1_buffer, Uint32 *src2_buffer, Uint32 *mask_buffer, Uint32 overflow_mask, Uint32 mask_value, int length) = NULL;

void setCpufuncs(unsigned int func)
{
    cpufuncs = func;

#if defined(USE_PPC_GFX)
    if(cpufuncs & CPUF_PPC_ALTIVEC) {
        imageFilterMeanFn = imageFilterMean_Altivec;
        imageFilterAddToFn = imageFilterAddTo_Altivec;
        imageFilterSubFromFn = imageFilterSubFrom_Altivec;
    }
#elif defined(USE_X86_GFX)
    if (cpufuncs & CPUF_X86_MMX) {
        imageFilterMeanFn = imageFilterMean_MMX;
        imageFilterAddToFn = imageFilterAddTo_MMX;
        imageFilterSubFromFn = imageFilterSubFrom_MMX;
    }
    if (cpufuncs & CPUF_X86_SSE2) {
        imageFilterMeanFn = imageFilterMean_SSE2;
        imageFilterAddToFn = imageFilterAddTo_SSE2;
        imageFilterSubFromFn = imageFilterSubFrom_SSE2;
        imageFilterBlendFn = imageFilterBlend_SSE2;
        imageFilterEffectBlendFn = imageFilterEffectBlend_SSE2;
        imageFilterEffectMaskBlendFn = imageFilterEffectMaskBlend_SSE2;
    }
#endif
}

unsigned int getCpufuncs()
{
    return cpufuncs;
}

#ifndef BPP16 // currently none of the fast CPU routines support 16bpp
void imageFilterMean(unsigned char *src1, unsigned char *src2, unsigned char *dst, int length)
{
    if(imageFilterMeanFn) {
        imageFilterMeanFn(src1, src2, dst, length);
    } else {
        int n = length + 1;
        BASIC_MEAN();
    }
}

void imageFilterAddTo(unsigned char *dst, unsigned char *src, int length)
{
    if(imageFilterAddToFn) {
        imageFilterAddToFn(dst, src, length);
    } else {
        int n = length + 1;
        BASIC_ADDTO();
    }
}

void imageFilterSubFrom(unsigned char *dst, unsigned char *src, int length)
{
    if(imageFilterSubFromFn) {
        imageFilterSubFromFn(dst, src, length);
    } else {
        int n = length + 1;
        BASIC_SUBFROM();
    }
}

void imageFilterBlend(Uint32 *dst_buffer, Uint32 *src_buffer,
                                     Uint8 *alphap, int alpha, int length)
{
    if(imageFilterBlendFn) {
        imageFilterBlendFn(dst, src, length);
    } else {
        int n = length + 1;
        BASIC_BLEND();
    }
}

void imageFilterEffectBlend(Uint32 *dst_buffer, Uint32 *src1_buffer,
                                           Uint32 *src2_buffer, Uint32 mask2, int length)
{
    if(imageFilterEffectBlendFn) {
        imageFilterEffectBlendFn(dst, src, length);
    } else {
        int n = length + 1;
        while(--n > 0) {
            BLEND_EFFECT_PIXEL();
            ++dst_buffer, ++src1_buffer, ++src2_buffer;
        }
    }
}

void imageFilterEffectMaskBlend(Uint32 *dst_buffer, Uint32 *src1_buffer,
                                               Uint32 *src2_buffer, Uint32 *mask_buffer,
                                               Uint32 overflow_mask, Uint32 mask_value,
                                               int length)
{
    if(imageFilterEffectMaskBlendFn) {
        imageFilterEffectMaskBlendFn(dst, src, length);
    } else {
        int n = length + 1;
        while(--n > 0) {
            BLEND_EFFECT_MASK_PIXEL();
            ++dst_buffer, ++src1_buffer, ++src2_buffer, ++mask_buffer;
        }
    }
}

#endif //!BPP16


static unsigned char *resize_buffer = NULL;
static size_t resize_buffer_size = 0;

void resetResizeBuffer() {
    if (resize_buffer_size != 16){
        if (resize_buffer) delete[] resize_buffer;
        resize_buffer = new unsigned char[16];
        resize_buffer_size = 16;
    }
}


// resize 32bit surface to 32bit surface
int resizeSurface( SDL_Surface *src, SDL_Surface *dst, int num_cells )
{
    SDL_LockSurface( dst );
    SDL_LockSurface( src );
    Uint32 *src_buffer = (Uint32 *)src->pixels;
    Uint32 *dst_buffer = (Uint32 *)dst->pixels;

    /* size of tmp_buffer must be larger than 16 bytes */
    size_t len = src->w * (src->h+1) * 4 + 4;
    if (resize_buffer_size < len){
        if (resize_buffer) delete[] resize_buffer;
        resize_buffer = new unsigned char[len];
        resize_buffer_size = len;
    }
    resizeImage( (unsigned char*)dst_buffer, dst->w, dst->h, dst->w * 4,
                 (unsigned char*)src_buffer, src->w, src->h, src->w * 4,
                 4, resize_buffer, src->w * 4, num_cells );

    SDL_UnlockSurface( src );
    SDL_UnlockSurface( dst );

    return 0;
}

} //namespace ons_gfx
