/* -*- C++ -*-
 * 
 *  graphics_cpu.h - ons_gfx namespace cpu-checking routines
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

#ifndef __GRAPHICS_CPU_H__
#define __GRAPHICS_CPU_H__

#include <stdint.h>

#include "graphics_common.h"

namespace ons_gfx {

    enum{
        CPUF_NONE           =  1 <<  0,
        CPUF_X86_MMX         =  1 <<  1,
        CPUF_X86_SSE         =  1 <<  2,
        CPUF_X86_SSE2        =  1 <<  3,
        CPUF_X86_SSE3        =  1 <<  4,
        CPUF_X86_SSSE3       =  1 <<  5,
        CPUF_X86_SSE4_1      =  1 <<  6,
        CPUF_X86_SSE4_2      =  1 <<  7,
        CPUF_X86_AVX         =  1 <<  8,
        CPUF_X86_AVX2        =  1 <<  9,
        CPUF_PPC_ALTIVEC    =  1 << 10,
        CPUF_ARM_NEON       =  1 << 11,
        CPUF_ARM_SVE        =  1 << 12,
        CPUF_ARM_SVE2       =  1 << 13,
    };

    void initCpuFuncs();
    void disableCpuAccelFuncs();
    
    void imageFilterMean_CPU(unsigned char *src1, unsigned char *src2, unsigned char *dst, int length);
    void imageFilterAddTo_CPU(unsigned char *dst, unsigned char *src, int length);
    void imageFilterSubFrom_CPU(unsigned char *dst, unsigned char *src, int length);
    void imageFilterBlend_CPU(uint32_t *dst_buffer, uint32_t *src_buffer,
                            uint8_t *alphap, int alpha, int length);
    void imageFilterEffectBlend_CPU(uint32_t *dst_buffer, uint32_t *src1_buffer,
                                    uint32_t *src2_buffer, uint32_t mask2, int length);
    void imageFilterEffectMaskBlend_CPU(uint32_t *dst_buffer, uint32_t *src1_buffer,
                                        uint32_t *src2_buffer, uint32_t *mask_buffer,
                                        uint32_t overflow_mask, uint32_t mask_value,
                                        int length);

}

#endif // __GRAPHICS_CPU_H__
