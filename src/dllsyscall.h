/******************************************************************************
 * @file            dllsyscall.h
 *
 * Released to the public domain.
 *
 * Anyone and anything may copy, edit, publish, use, compile, sell and
 * distribute this work and all its parts in any form for any purpose,
 * commercial and non-commercial, without any restrictions, without
 * complying with any conditions and by any means.
 *****************************************************************************/
#define system_call_1(id, a1) \
 PosWinSyscall ((id), &(a1));
#define system_call_2(id, a1, a2) \
 PosWinSyscall ((id), &(a1));
#define system_call_3(id, a1, a2, a3) \
 PosWinSyscall ((id), &(a1));
#define system_call_4(id, a1, a2, a3, a4) \
 PosWinSyscall ((id), &(a1));
#define system_call_5(id, a1, a2, a3, a4, a5) \
 PosWinSyscall ((id), &(a1));
#define system_call_6(id, a1, a2, a3, a4, a5, a6) \
 PosWinSyscall ((id), &(a1));
#define system_call_7(id, a1, a2, a3, a4, a5, a6, a7) \
 PosWinSyscall ((id), &(a1));
#define system_call_8(id, a1, a2, a3, a4, a5, a6, a7, a8) \
 PosWinSyscall ((id), &(a1));
#define system_call_9(id, a1, a2, a3, a4, a5, a6, a7, a8, a9) \
 PosWinSyscall ((id), &(a1));
#define system_call_10(id, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10) \
 PosWinSyscall ((id), &(a1));
#define system_call_11(id, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11) \
 PosWinSyscall ((id), &(a1));
#define system_call_12(id, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12) \
 PosWinSyscall ((id), &(a1));
