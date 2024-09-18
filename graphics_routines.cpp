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

#include <cstddef>


#if (ONS_X86 || ONS_X8664) && !defined(MACOSX) && !defined(_MSC_VER)
#include <cpuid.h>
#endif

#include <SDL.h>

#include "graphics_cpu.h"
#include "graphics_sum.h"
#include "graphics_blend.h"
#include "graphics_resize.h"

// These are internally guarded, and we already guard for usage of their functions below, 
// so there's no need to check availiblity for the headers..
#include "graphics_altivec.h"
#include "graphics_mmx.h"
#include "graphics_sse2.h"

#include "resize_image.h"

namespace ons_gfx {

void imageFilterMean_CPU(uint8_t *src1, uint8_t *src2, uint8_t *dst, int length)
{
    int n = length + 1;
    BASIC_MEAN();
}

void imageFilterAddTo_CPU(uint8_t *dst, uint8_t *src, int length)
{
    int n = length + 1;
    BASIC_ADDTO();
}

void imageFilterSubFrom_CPU(uint8_t *dst, uint8_t *src, int length)
{
    int n = length + 1;
    BASIC_SUBFROM();
}

void imageFilterBlend_CPU(uint32_t *dst_buffer, uint32_t *src_buffer,
                          uint8_t *alphap, int alpha, int length)
{
    int n = length + 1;
    BASIC_BLEND();
}


void imageFilterEffectBlend_CPU(uint32_t *dst_buffer, uint32_t *src1_buffer,
                                uint32_t *src2_buffer, uint32_t mask2, int length)
{
    int n = length + 1;
    while(--n > 0) {
        BLEND_EFFECT_PIXEL();
        ++dst_buffer, ++src1_buffer, ++src2_buffer;
    }
}

void imageFilterEffectMaskBlend_CPU(uint32_t *dst_buffer, uint32_t *src1_buffer,
                                    uint32_t *src2_buffer, uint32_t *mask_buffer,
                                    uint32_t overflow_mask, uint32_t mask_value,
                                    int length)
{
    int n = length + 1;
    while(--n > 0) {
        BLEND_EFFECT_MASK_PIXEL();
        ++dst_buffer, ++src1_buffer, ++src2_buffer, ++mask_buffer;
    }
}


//Mion: for special graphics routine handling
static unsigned int cpufuncs;



void (*imageFilterMean)(uint8_t *src1, uint8_t *src2, uint8_t *dst, int length) = imageFilterMean_CPU;
void (*imageFilterAddTo)(uint8_t *dst, uint8_t *src, int length) = imageFilterAddTo_CPU;
void (*imageFilterSubFrom)(uint8_t *dst, uint8_t *src, int length) = imageFilterSubFrom_CPU;
void (*imageFilterBlend)(uint32_t *dst_buffer, uint32_t *src_buffer, uint8_t *alphap, int alpha, int length) = imageFilterBlend_CPU;
void (*imageFilterEffectBlend)(uint32_t *dst_buffer, uint32_t *src1_buffer, uint32_t *src2_buffer, uint32_t mask2, int length) = imageFilterEffectBlend_CPU;
void (*imageFilterEffectMaskBlend)(uint32_t *dst_buffer, uint32_t *src1_buffer, uint32_t *src2_buffer, uint32_t *mask_buffer, uint32_t overflow_mask, uint32_t mask_value, int length) = imageFilterEffectMaskBlend_CPU;


void initCpuFuncs()
{
    unsigned int func = CPUF_NONE;

#ifndef BPP16 // currently none of the fast CPU routines support 16bpp
#if (ONS_X86 || ONS_X8664) && !defined(MACOSX) && !defined(_MSC_VER)
    // determine what functions the cpu supports (Mion)
    {
        unsigned int func, eax, ebx, ecx, edx;
        func = CPUF_NONE;
        if (__get_cpuid(1, &eax, &ebx, &ecx, &edx) != 0) {
            printf("System info: Intel CPU, with functions: ");
            if (edx & bit_MMX) {
                func |= CPUF_X86_MMX;
                printf("MMX ");
            }
            if (edx & bit_SSE) {
                func |= CPUF_X86_SSE;
                printf("SSE ");
            }
            if (edx & bit_SSE2) {
                func |= CPUF_X86_SSE2;
                printf("SSE2 ");
            }
            printf("\n");
        }
    }
#elif ONS_X8664 && defined(MACOSX)
    {
        // x86 CPU on Mac OS X all support SSE2
        func = ons_gfx::CPUF_X86_SSE2;
        printf("System info: Intel CPU with SSE2 functionality\n");   
    }
#elif ONS_PPC && defined(MACOSX)
    // Determine if this PPC CPU supports AltiVec (Roto)
    {
        unsigned int func = CPUF_NONE;
        int altivec_present = 0;
    
        size_t length = sizeof(altivec_present);
        int error = sysctlbyname("hw.optional.altivec", &altivec_present, &length, NULL, 0);

        if((error == 0) && altivec_present) {
            func |= CPUF_PPC_ALTIVEC;
            printf("System info: PowerPC CPU, supports altivec\n");
        } else {
            printf("System info: PowerPC CPU, DOES NOT support altivec\n");
        }
    }
#endif
#endif //!BPP16

    cpufuncs = func;

#ifndef BPP16 // currently none of the fast CPU routines support 16bpp
    #if ONS_PPC_ALTIVEC_AVAILIBLE
        if(cpufuncs & CPUF_PPC_ALTIVEC) {
            imageFilterMean = imageFilterMean_Altivec;
            imageFilterAddTo = imageFilterAddTo_Altivec;
            imageFilterSubFrom = imageFilterSubFrom_Altivec;
        }
    #elif ONS_X86_MMX_AVAILIBLE
        if (cpufuncs & CPUF_X86_MMX) {
            imageFilterMean = imageFilterMean_MMX;
            imageFilterAddTo = imageFilterAddTo_MMX;
            imageFilterSubFrom = imageFilterSubFrom_MMX;
        }
    #elif ONS_X86_SSE2_AVAILIBLE
        if (cpufuncs & CPUF_X86_SSE2) {
            imageFilterMean = imageFilterMean_SSE2;
            imageFilterAddTo = imageFilterAddTo_SSE2;
            imageFilterSubFrom = imageFilterSubFrom_SSE2;
            imageFilterBlend = imageFilterBlend_SSE2;
            imageFilterEffectBlend = imageFilterEffectBlend_SSE2;
            imageFilterEffectMaskBlend = imageFilterEffectMaskBlend_SSE2;
        }
    #endif
#endif //!BPP16
}

void disableCpuAccelFuncs()
{
    imageFilterMean = imageFilterMean_CPU;
    imageFilterAddTo = imageFilterAddTo_CPU;
    imageFilterSubFrom = imageFilterSubFrom_CPU;
    imageFilterBlend = imageFilterBlend_CPU;
    imageFilterEffectBlend = imageFilterEffectBlend_CPU;
    imageFilterEffectMaskBlend = imageFilterEffectMaskBlend_CPU;

    cpufuncs = CPUF_NONE;
}

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
    uint32_t *src_buffer = (uint32_t *)src->pixels;
    uint32_t *dst_buffer = (uint32_t *)dst->pixels;

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
