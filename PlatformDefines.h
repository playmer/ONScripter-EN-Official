#ifndef __PLATFORMDEFINES_H__
#define __PLATFORMDEFINES_H__

#if defined(__x86_64__) || defined(_M_X64)
    #define ONS_X8664 1
#elif defined(i386) || defined(__i386__) || defined(__i386) || defined(_M_IX86)
    #define ONS_X86 1
#elif defined(__powerpc) || defined(__powerpc__) || defined(__powerpc64__) || defined(__POWERPC__) || defined(__ppc__) || defined(__PPC__) || defined(_ARCH_PPC)
    #define ONS_PPC 1
#elif defined(__PPC64__) || defined(__ppc64__) || defined(_ARCH_PPC64)
    #define ONS_PPC64 1
#else
    #error "Unknown architecture, please add it and report to GitHub."
#endif

#if defined(ONS_X8664) || defined(ONS_X86) 
    #define ONS_X86_MMX_AVAILIBLE 1
    #define ONS_X86_SSE_AVAILIBLE 1
    #define ONS_X86_SSE2_AVAILIBLE 1
#endif 

#ifdef ONS_PPC
    #define ONS_PPC_ALTIVEC_AVAILIBLE 1
#endif

#endif