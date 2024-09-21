/* -*- C++ -*-
 * 
 *  graphics_blend.h - graphics macros and ons_gfx namespace utility functions
 *                     for blends, monochrome filters
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

#ifndef __GRAPHICS_BLEND_H__
#define __GRAPHICS_BLEND_H__

//#include <SDL.h>

#include "stdint.h"
#include "graphics_common.h"

// Incorporates code originally from AnimationInfo.cpp & ONScripterLabel_image.cpp,
// copyright (c) 2001-2011 Ogapee
#ifdef BPP16

/* Used in AnimationInfo */

#define BLEND_PIXEL(){\
    if ((*alphap == 255) && (alpha == 256)) {\
        *dst_buffer = *src_buffer;\
    } else if (*alphap != 0) {\
        uint32_t mask2 = (*alphap * alpha) >> 11;\
        uint32_t s1 = (*src_buffer | *src_buffer << 16) & BLENDMASK;\
        uint32_t d1 = (*dst_buffer | *dst_buffer << 16) & BLENDMASK;\
        uint32_t mask1 = (d1 + ((s1-d1) * mask2 >> BLENDSHIFT)) & BLENDMASK;\
        *dst_buffer = mask1 | mask1 >> 16;\
    }\
    alphap++;\
}

#define BLEND_TEXT_ALPHA(){\
    uint32_t mask2 = *src_buffer; \
    if (mask2 != 0){ \
        *alphap = 0xff ^ ((0xff ^ *alphap)*(0xff ^ *src_buffer) >> 8); \
        mask2 = (mask2 << 5) / *alphap; \
        uint32_t d1 = (*dst_buffer | *dst_buffer << 16) & BLENDMASK; \
        uint32_t mask = (d1 + ((src_color-d1) * mask2 >> BLENDSHIFT)) & BLENDMASK; \
        *dst_buffer = mask | mask >> 16; \
    } \
    alphap++; \
}

#define ADDBLEND_PIXEL(){\
    if ((*src_buffer != 0) && (*alphap != 0)){\
        uint32_t mask2 = (*alphap * alpha) >> 11;\
        uint32_t s1 = (*src_buffer | *src_buffer << 16) & BLENDMASK;\
        uint32_t d1 = (*dst_buffer | *dst_buffer << 16) & BLENDMASK;\
        uint32_t mask1 = d1 + (((s1 * mask2) >> BLENDSHIFT) & BLENDMASK);\
        mask1 |= ((mask1 & RMASKHI) ? GMASKHI : 0) |\
                 ((mask1 & BMASKHI) ? RMASK : 0) |\
                 ((mask1 & GMASK) ? BMASK : 0);\
        mask1 &= BLENDMASK;\
        *dst_buffer = mask1 | mask1 >> 16;\
    }\
    alphap++;\
}

#define SUBBLEND_PIXEL(){\
    if ((*src_buffer != 0) && (*alphap != 0)){\
        uint32_t mask2 = (*alphap * alpha) >> 11;\
        uint32_t mask_r = (*dst_buffer & RMASK) -\
                        ((((*src_buffer & RMASK) * mask2) >> 5) & RMASK);\
        mask_r &= ((mask_r & BMASKHI) ? 0 : RMASK);\
        uint32_t mask_g = (*dst_buffer & GMASK) -\
                        ((((*src_buffer & GMASK) * mask2) >> 5) & GMASK);\
        mask_g &= ((mask_g & ~GBMASK) ? 0 : GMASK);\
        uint32_t mask_b = (*dst_buffer & BMASK) -\
                        ((((*src_buffer & BMASK) * mask2) >> 5) & BMASK);\
        mask_b &= ((mask_b & ~BMASK) ? 0 : BMASK);\
        *dst_buffer = (mask_r & RMASK) | (mask_g & GMASK) | (mask_b & BMASK);\
    }\
    alphap++;\
}

// monocro 85/86/85 uses 85=(4+1)*(16+1)
#define MONOCRO_PIXEL(){\
    const ONSBuf tmp = ((*buffer & GMASK) >> GSHIFT) << GLOSS; \
    ONSBuf c = (((*buffer & RMASK) >> RSHIFT) << RLOSS) + \
                ((*buffer & BMASK) << BLOSS) + tmp; \
    c += c<<2; \
    c += (c<<4) + tmp; \
    c >>= 8; \
    *buffer = ((monocro_color_lut[c][0] >> RLOSS) << RSHIFT) | \
              ((monocro_color_lut[c][1] >> GLOSS) << GSHIFT) | \
              (monocro_color_lut[c][2] >> BLOSS); \
}

/* Used in ONScripterLabel_image */

#define BLEND_TEXT(){\
    uint32_t mask2 = *src_buffer >> 3; \
    if (mask2 != 0){ \
        uint32_t d1 = (*dst_buffer | *dst_buffer << 16) & BLENDMASK; \
        uint32_t mask = (d1 + ((src_color-d1) * mask2 >> BLENDSHIFT)) & BLENDMASK; \
        *dst_buffer = mask | mask >> 16; \
    } \
}

#define BLEND_EFFECT_PIXEL(){\
    uint32_t s1 = (*src1_buffer | *src1_buffer << 16) & BLENDMASK; \
    uint32_t s2 = (*src2_buffer | *src2_buffer << 16) & BLENDMASK; \
    uint32_t mask = (s1 + ((s2-s1) * mask2 >> BLENDSHIFT)) & BLENDMASK; \
    *dst_buffer = mask | mask >> 16; \
}

#else //!BPP16

/* Used in AnimationInfo */

#define BLEND_PIXEL(){\
    if ((*alphap == 255) && (alpha == 256)) {\
        *dst_buffer = *src_buffer;\
    } else if (*alphap != 0) {\
        uint32_t mask2 = (*alphap * alpha) >> 8;\
        uint32_t temp = *dst_buffer & RBMASK;\
        uint32_t mask_rb = (((((*src_buffer & RBMASK) - temp) * mask2) >> 8) + temp) & RBMASK;\
        temp = *dst_buffer & GMASK;\
        uint32_t mask_g  = (((((*src_buffer & GMASK) - temp) * mask2) >> 8) + temp) & GMASK;\
        *dst_buffer = mask_rb | mask_g;\
    }\
    alphap += 4;\
}

#define BLEND_TEXT_ALPHA(){\
    uint32_t mask2 = *src_buffer; \
    if (mask2 == 255){ \
        *dst_buffer = src_color3; \
    } \
    else if (mask2 != 0){ \
        uint32_t alpha = *dst_buffer >> ASHIFT; \
        uint32_t mask1 = ((0xff ^ mask2) * alpha) >> 8; \
        alpha = inv_alpha_lut[mask1 + mask2]; \
        uint32_t mask_rb = (*dst_buffer & RBMASK) * mask1 + \
                         src_color1 * mask2; \
        mask_rb = (((mask_rb >> 16) * alpha) & RMASK) | \
                  ((((mask_rb & GBMASK) * alpha) >> 16) & BMASK); \
        uint32_t mask_g = ((((*dst_buffer & GMASK) * mask1 + \
                           src_color2 * mask2) * alpha) >> 16) & GMASK; \
        *dst_buffer = mask_rb | mask_g | ((mask1+mask2) << ASHIFT); \
    } \
}

#define ADDBLEND_PIXEL(){\
    if ((*src_buffer != 0) && (*alphap != 0)){\
        uint32_t mask2 = (*alphap * alpha) >> 8;\
        uint32_t mask_rb = (*dst_buffer & RBMASK) +\
                         ((((*src_buffer & RBMASK) * mask2) >> 8) & RBMASK);\
        mask_rb |= ((mask_rb & AMASK) ? RMASK : 0) |\
                   ((mask_rb & GMASK) ? BMASK : 0);\
        uint32_t mask_g = (*dst_buffer & GMASK) +\
                        ((((*src_buffer & GMASK) * mask2) >> 8) & GMASK);\
        mask_g |= ((mask_g & RMASK) ? GMASK : 0);\
        *dst_buffer = (mask_rb & RBMASK) | (mask_g & GMASK);\
    }\
    alphap += 4;\
}

#define SUBBLEND_PIXEL(){\
    if ((*src_buffer != 0) && (*alphap != 0)){\
        uint32_t mask2 = (*alphap * alpha) >> 8;\
        uint32_t mask_r = (*dst_buffer & RMASK) -\
                        ((((*src_buffer & RMASK) * mask2) >> 8) & RMASK);\
        mask_r &= ((mask_r & AMASK) ? 0 : RMASK);\
        uint32_t mask_g = (*dst_buffer & GMASK) -\
                        ((((*src_buffer & GMASK) * mask2) >> 8) & GMASK);\
        mask_g &= ((mask_g & ~GBMASK) ? 0 : GMASK);\
        uint32_t mask_b = (*dst_buffer & BMASK) -\
                        ((((*src_buffer & BMASK) * mask2) >> 8) & BMASK);\
        mask_b &= ((mask_b & ~BMASK) ? 0 : BMASK);\
        *dst_buffer = (mask_r & RMASK) | (mask_g & GMASK) | (mask_b & BMASK);\
    }\
    alphap += 4;\
}

// monocro 85/86/85 uses 85=(4+1)*(16+1)
#define MONOCRO_PIXEL(){\
    const ONSBuf tmp = ((*buffer & GMASK) >> GSHIFT); \
    ONSBuf c = ((*buffer & RMASK) >> RSHIFT) + (*buffer & BMASK) + tmp; \
    c += c<<2; \
    c += (c<<4) + tmp; \
    c >>= 8; \
    *buffer = (monocro_color_lut[c][0] << RSHIFT) | \
              (monocro_color_lut[c][1] << GSHIFT) | \
              monocro_color_lut[c][2]; \
}

/* Used in ONScripterLabel_image */

#define BLEND_TEXT(){\
    uint32_t mask2 = *src_buffer; \
    if (mask2 == 255){ \
        *dst_buffer = src_color3; \
    } \
    else if (mask2 != 0){ \
        uint32_t mask1 = mask2 ^ 0xff; \
        uint32_t mask_rb = (((*dst_buffer & RBMASK) * mask1 + \
                            src_color1 * mask2) >> 8) & RBMASK; \
        uint32_t mask_g  = (((*dst_buffer & GMASK) * mask1 + \
                            src_color2 * mask2) >> 8) & GMASK; \
        *dst_buffer = mask_rb | mask_g; \
    } \
}

#define BLEND_EFFECT_PIXEL(){\
    uint32_t temp = *src1_buffer & RBMASK;\
    uint32_t mask_rb = (((((*src2_buffer & RBMASK) - temp) * mask2) >> 8) + temp) & RBMASK;\
    temp = *src1_buffer & GMASK;\
    uint32_t mask_g  = (((((*src2_buffer & GMASK) - temp) * mask2) >> 8) + temp) & GMASK;\
    *dst_buffer = mask_rb | mask_g;\
}

#endif //ndef BPP16

#define BLEND_EFFECT_MASK_PIXEL(){\
    uint32_t mask2 = 0;\
    uint32_t mask = *mask_buffer & BMASK;\
    if ( mask_value > mask ){\
        mask2 = mask_value - mask;\
        if ( mask2 & overflow_mask ) mask2 = BMASK;\
    }\
    BLEND_EFFECT_PIXEL();\
}

#define BASIC_BLEND(){\
    while(--n > 0) {  \
        BLEND_PIXEL();  \
        ++dst_buffer, ++src_buffer;  \
    } \
}

#define BASIC_ADDBLEND(){\
    while(--n > 0) {  \
        ADDBLEND_PIXEL();  \
        ++dst_buffer, ++src_buffer;  \
    } \
}

#define BASIC_SUBBLEND(){\
    while(--n > 0) {  \
        SUBBLEND_PIXEL();  \
        ++dst_buffer, ++src_buffer;  \
    } \
}


namespace ons_gfx {

#ifndef BPP16 // currently none of the fast CPU routines support 16bpp
    extern void (*imageFilterBlend)(uint32_t *dst_buffer, uint32_t *src_buffer, uint8_t *alphap, int alpha, int length);
    extern void (*imageFilterEffectBlend)(uint32_t *dst_buffer, uint32_t *src1_buffer, uint32_t *src2_buffer, uint32_t mask2, int length);
    extern void (*imageFilterEffectMaskBlend)(uint32_t *dst_buffer, uint32_t *src1_buffer, uint32_t *src2_buffer, uint32_t *mask_buffer, uint32_t overflow_mask, uint32_t mask_value, int length);
#endif //!BPP16

}

#endif // __GRAPHICS_BLEND_H__
