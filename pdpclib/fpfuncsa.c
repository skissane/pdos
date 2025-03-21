/*********************************************************************/
/*                                                                   */
/*  This Program Written by Paul Edwards.                            */
/*  Released to the Public Domain                                    */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  fpfuncsa.c - some support routines for floating point work       */
/*  for ARM work. I had started implementing a really bad algorithm  */
/*  that would at least work for whole numbers. After seeing the     */
/*  internal representation, I lowered the goal to being able to add */
/*  up 16 + 32 (powers of two). Then I realized I couldn't even do   */
/*  that. All I could do was add up identical powers of 2, ie        */
/*  16 + 16. But just before I came to that realization,             */
/*  Jean-Marc Lienher pointed me to the below public domain code     */
/*  from Steve Rhoads which can be found here:                       */
/*  https://github.com/public-domain/plasma-cpu/blob/main/src/       */
/*          kernel/math.c                                            */
/*                                                                   */
/*  Unfortunately they only support single precision, so I needed    */
/*  to change gccarm to treat all floating point as single precision */
/*  See armsupa.asm for further comments                             */
/*                                                                   */
/*********************************************************************/

/* IEEE double is 1 sign bit, 11 bits for exponent and 53 bits
   precision (52 stored and 1 (of value 1) implied I believe) */

/* IEEE single is 1 sign bit, 8 bits for exponent and 24 bits
   precision (23 stored and 1 (of value 1) implied I believe) */

/* the exponent effectively has the high bit set for 0 and above
   - called the bias */

/* we might be able to make use of the single precision floating
   point by converting double to float and just living within
   the limits */


#include <stddef.h>
#include <assert.h>

/* these routines call other routines to do truncation, and I don't
   know how to do that, so we will instead need to switch to
   single precision only for ARM */

#if 0
double __adddf3(double a, double b)
{
    return ((double)((float)a + (float)b));
}

unsigned long __fixdfsi(double a)
{
    return ((long)((float)a));
}
#endif



#if 1

/* rtos.h: */

/*--------------------------------------------------------------------
 * TITLE: Plasma Real Time Operating System
 * AUTHOR: Steve Rhoads (rhoadss@yahoo.com)
 * DATE CREATED: 12/17/05
 * FILENAME: rtos.h
 * PROJECT: Plasma CPU core
 * COPYRIGHT: Software placed into the public domain by the author.
 *    Software 'as is' without warranty.  Author liable for nothing.
 * DESCRIPTION:
 *    Plasma Real Time Operating System
 *    Fully pre-emptive RTOS with support for:
 *       Heaps, Threads, Semaphores, Mutexes, Message Queues, and Timers.
 *--------------------------------------------------------------------*/
#ifndef __RTOS_H__
#define __RTOS_H__

/* Symmetric Multi-Processing */
#define OS_CPU_COUNT 1

/* Typedefs */
typedef unsigned int   uint32;
typedef unsigned short uint16;
typedef unsigned char  uint8;

/* Memory Access */
#ifdef __TINYC__
   #define WIN32
#endif
#ifdef WIN32
   #define _CRT_SECURE_NO_WARNINGS 1
   #include <stdio.h>
   #include <assert.h>
   #include <setjmp.h>
   #define _LIBC
   uint32 MemoryRead(uint32 Address);
   void MemoryWrite(uint32 Address, uint32 Value);
#else
   #define MemoryRead(A) (*(volatile uint32*)(A))
   #define MemoryWrite(A,V) *(volatile uint32*)(A)=(V)
#endif

/***************** LibC ******************/
#if 0
#undef isprint
#undef isspace
#undef isdigit
#undef islower
#undef isupper
#undef isalpha
#undef isalnum
#undef min
#define isprint(c) (' '<=(c)&&(c)<='~')
#define isspace(c) ((c)==' '||(c)=='\t'||(c)=='\n'||(c)=='\r')
#define isdigit(c) ('0'<=(c)&&(c)<='9')
#define islower(c) ('a'<=(c)&&(c)<='z')
#define isupper(c) ('A'<=(c)&&(c)<='Z')
#define isalpha(c) (islower(c)||isupper(c))
#define isalnum(c) (isalpha(c)||isdigit(c))
#define min(a,b)   ((a)<(b)?(a):(b))
#define strcpy     strcpy2  /*don't use intrinsic functions*/
#define strncpy    strncpy2
#define strcat     strcat2
#define strncat    strncat2
#define strcmp     strcmp2
#define strncmp    strncmp2
#define strstr     strstr2
#define strlen     strlen2
#define memcpy     memcpy2
#define memmove    memmove2
#define memcmp     memcmp2
#define memset     memset2
#define abs        abs2
#define atoi       atoi2
#define rand       rand2
#define srand      srand2
#define strtol     strtol2
#define itoa       itoa2
#define sprintf    sprintf2
#define sscanf     sscanf2
#define malloc(S)  OS_HeapMalloc(NULL, S)
#define free(S)    OS_HeapFree(S)

char *strcpy(char *dst, const char *src);
char *strncpy(char *dst, const char *src, int count);
char *strcat(char *dst, const char *src);
char *strncat(char *dst, const char *src, int count);
int   strcmp(const char *string1, const char *string2);
int   strncmp(const char *string1, const char *string2, int count);
char *strstr(const char *string, const char *find);
int   strlen(const char *string);
void *memcpy(void *dst, const void *src, unsigned long bytes);
void *memmove(void *dst, const void *src, unsigned long bytes);
int   memcmp(const void *cs, const void *ct, unsigned long bytes);
void *memset(void *dst, int c, unsigned long bytes);
int   abs(int n);
int   atoi(const char *s);
int   rand(void);
void  srand(unsigned int seed);
long  strtol(const char *s, char **end, int base);
char *itoa(int num, char *dst, int base);
#endif

#if 0 /* ndef NO_ELLIPSIS */
   typedef char* va_list;
   #define va_start(AP,P) (AP=(char*)&(P)+sizeof(char*))
   #define va_arg(AP,T) (*(T*)((AP+=sizeof(T))-sizeof(T)))
   #define va_end(AP)
   int sprintf(char *s, const char *format, ...);
   int sscanf(const char *s, const char *format, ...);
#endif

#if 0
#define printf     UartPrintf
#endif
#ifndef _LIBC

#if 0
   #define assert(A) if((A)==0){OS_Assert();UartPrintfCritical("\r\nAssert %s:%d\r\n", __FILE__, __LINE__);}
   #define scanf     UartScanf
   #define NULL      (void*)0
#endif

#else
   #define UartPrintfCritical UartPrintf
#endif /*_LIBC */

#ifdef INCLUDE_DUMP
   void dump(const unsigned char *data, int length);
#endif
#ifdef INCLUDE_QSORT
   void qsort(void *base, 
              long n, 
              long size, 
              int (*cmp)(const void *,const void *));
   void *bsearch(const void *key,
                 const void *base,
                 long n,
                 long size,
                 int (*cmp)(const void *,const void *));
#endif
#ifdef INCLUDE_TIMELIB
   #define difftime(time2,time1) (time2-time1)
   typedef unsigned long time_t;  /* start at 1/1/80 */
   struct tm {
      int tm_sec;      /*(0,59)*/
      int tm_min;      /*(0,59)*/
      int tm_hour;     /*(0,23)*/
      int tm_mday;     /*(1,31)*/
      int tm_mon;      /*(0,11)*/
      int tm_year;     /*(0,n) from 1900*/
      int tm_wday;     /*(0,6)     calculated*/
      int tm_yday;     /*(0,365)   calculated*/
      int tm_isdst;    /*          calculated*/
   };
   time_t mktime(struct tm *tp);
   void gmtime_r(const time_t *tp, struct tm *out);
   void gmtimeDst(time_t dstTimeIn, time_t dstTimeOut);
   void gmtimeDstSet(time_t *tp, time_t *dstTimeIn, time_t *dstTimeOut);
#endif

/***************** Assembly **************/
#ifndef WIN32
/* not sure what this is, but it stopped makefile.aga from building */
/* typedef uint32 jmp_buf[20];
extern int setjmp(jmp_buf env);
extern void longjmp(jmp_buf env, int val); */
#endif
extern uint32 OS_AsmInterruptEnable(uint32 state);
extern void OS_AsmInterruptInit(void);
extern uint32 OS_AsmMult(uint32 a, uint32 b, unsigned long *hi);
extern void *OS_Syscall(uint32 value);

/***************** Heap ******************/
typedef struct OS_Heap_s OS_Heap_t;
#define HEAP_USER    (OS_Heap_t*)0
#define HEAP_SYSTEM  (OS_Heap_t*)1
#define HEAP_SMALL   (OS_Heap_t*)2
#define HEAP_UI      (OS_Heap_t*)3
OS_Heap_t *OS_HeapCreate(const char *name, void *memory, uint32 size);
void OS_HeapDestroy(OS_Heap_t *heap);
void *OS_HeapMalloc(OS_Heap_t *heap, int bytes);
void OS_HeapFree(void *block);
void OS_HeapAlternate(OS_Heap_t *heap, OS_Heap_t *alternate);
void OS_HeapRegister(void *index, OS_Heap_t *heap);

/***************** Critical Sections *****************/
#if OS_CPU_COUNT <= 1
   /* Single CPU */
   #define OS_CpuIndex() 0
   #define OS_CriticalBegin() OS_AsmInterruptEnable(0)
   #define OS_CriticalEnd(S) OS_AsmInterruptEnable(S)
   #define OS_SpinLock() 0
   #define OS_SpinUnlock(S) 
#else
   /* Symmetric multiprocessing */
   uint32 OS_CpuIndex(void);
   #define OS_CriticalBegin() OS_SpinLock()
   #define OS_CriticalEnd(S) OS_SpinUnlock(S)
   uint32 OS_SpinLock(void);
   void OS_SpinUnlock(uint32 state);
#endif

/***************** Thread *****************/
#ifdef WIN32
   #define STACK_SIZE_MINIMUM (1024*8)
#else
   #define STACK_SIZE_MINIMUM (1024*1)
#endif
#define STACK_SIZE_DEFAULT 1024*2
#undef THREAD_PRIORITY_IDLE
#define THREAD_PRIORITY_IDLE 0
#define THREAD_PRIORITY_MAX 255

typedef void (*OS_FuncPtr_t)(void *arg);
typedef struct OS_Thread_s OS_Thread_t;
OS_Thread_t *OS_ThreadCreate(const char *name,
                             OS_FuncPtr_t funcPtr, 
                             void *arg, 
                             uint32 priority, 
                             uint32 stackSize);
void OS_ThreadExit(void);
OS_Thread_t *OS_ThreadSelf(void);
void OS_ThreadSleep(int ticks);
uint32 OS_ThreadTime(void);
void OS_ThreadInfoSet(OS_Thread_t *thread, uint32 index, void *info);
void *OS_ThreadInfoGet(OS_Thread_t *thread, uint32 index);
uint32 OS_ThreadPriorityGet(OS_Thread_t *thread);
void OS_ThreadPrioritySet(OS_Thread_t *thread, uint32 priority);
void OS_ThreadProcessId(OS_Thread_t *thread, uint32 processId, OS_Heap_t *heap);
void OS_ThreadCpuLock(OS_Thread_t *thread, int cpuIndex);

/***************** Semaphore **************/
#define OS_SUCCESS 0
#define OS_ERROR  -1
#define OS_WAIT_FOREVER -1
#define OS_NO_WAIT 0
typedef struct OS_Semaphore_s OS_Semaphore_t;
OS_Semaphore_t *OS_SemaphoreCreate(const char *name, uint32 count);
void OS_SemaphoreDelete(OS_Semaphore_t *semaphore);
int OS_SemaphorePend(OS_Semaphore_t *semaphore, int ticks); /* tick ~= 10ms */
void OS_SemaphorePost(OS_Semaphore_t *semaphore);

/***************** Mutex ******************/
typedef struct OS_Mutex_s OS_Mutex_t;
OS_Mutex_t *OS_MutexCreate(const char *name);
void OS_MutexDelete(OS_Mutex_t *semaphore);
void OS_MutexPend(OS_Mutex_t *semaphore);
void OS_MutexPost(OS_Mutex_t *semaphore);

/***************** MQueue *****************/
enum {
   MESSAGE_TYPE_USER = 0,
   MESSAGE_TYPE_TIMER = 5
};
typedef struct OS_MQueue_s OS_MQueue_t;
OS_MQueue_t *OS_MQueueCreate(const char *name,
                             int messageCount,
                             int messageBytes);
void OS_MQueueDelete(OS_MQueue_t *mQueue);
int OS_MQueueSend(OS_MQueue_t *mQueue, void *message);
int OS_MQueueGet(OS_MQueue_t *mQueue, void *message, int ticks);

/***************** Job ********************/
typedef void (*JobFunc_t)(void *a0, void *a1, void *a2);
void OS_Job(JobFunc_t funcPtr, void *arg0, void *arg1, void *arg2);

/***************** Timer ******************/
typedef struct OS_Timer_s OS_Timer_t;
typedef void (*OS_TimerFuncPtr_t)(OS_Timer_t *timer, uint32 info);
OS_Timer_t *OS_TimerCreate(const char *name, OS_MQueue_t *mQueue, uint32 info);
void OS_TimerDelete(OS_Timer_t *timer);
void OS_TimerCallback(OS_Timer_t *timer, OS_TimerFuncPtr_t callback);
void OS_TimerStart(OS_Timer_t *timer, uint32 ticks, uint32 ticksRestart);
void OS_TimerStop(OS_Timer_t *timer);

/***************** ISR ********************/
#define STACK_EPC 88/4
void OS_InterruptServiceRoutine(uint32 status, uint32 *stack);
void OS_InterruptRegister(uint32 mask, OS_FuncPtr_t funcPtr);
uint32 OS_InterruptStatus(void);
uint32 OS_InterruptMaskSet(uint32 mask);
uint32 OS_InterruptMaskClear(uint32 mask);

/***************** Init ******************/
void OS_Init(uint32 *heapStorage, uint32 bytes);
void OS_InitSimulation(void);
void OS_Start(void);
void OS_Assert(void);
void MainThread(void *Arg);

/***************** UART ******************/
typedef uint8* (*PacketGetFunc_t)(void);
void UartInit(void);
void UartWrite(int ch);
uint8 UartRead(void);
void UartWriteData(uint8 *data, int length);
void UartReadData(uint8 *data, int length);
#ifndef NO_ELLIPSIS2
void UartPrintf(const char *format, ...);
void UartPrintfPoll(const char *format, ...);
void UartPrintfCritical(const char *format, ...);
void UartPrintfNull(const char *format, ...);
void UartScanf(const char *format, ...);
#endif
void UartPacketConfig(PacketGetFunc_t packetGetFunc, 
                      int packetSize, 
                      OS_MQueue_t *mQueue);
void UartPacketSend(uint8 *data, int bytes);
int OS_puts(const char *string);
int OS_getch(void);
int OS_kbhit(void);
void LogWrite(int a);
void LogDump(void);
void Led(int mask, int value);

/***************** Keyboard **************/
#define KEYBOARD_RAW     0x100
#define KEYBOARD_E0      0x200
#define KEYBOARD_RELEASE 0x400
void KeyboardInit(void);
int KeyboardGetch(void);

/***************** Math ******************/
/*IEEE single precision floating point math */
#ifndef WIN32
#define FP_Neg     __negsf2
#define FP_Add     __addsf3
#define FP_Sub     __subsf3
#define FP_Mult    __mulsf3
#define FP_Div     __divsf3
#define FP_ToLong  __fixsfsi

/* this one is guessed */
#define FP_ToULong  __fixunssfsi

#define FP_ToFloat __floatsisf
#if 0
#define sqrt FP_Sqrt
#define cos  FP_Cos
#define sin  FP_Sin
#define atan FP_Atan
#define log  FP_Log
#define exp  FP_Exp
#endif
#endif
float FP_Neg(float a_fp);
float FP_Add(float a_fp, float b_fp);
float FP_Sub(float a_fp, float b_fp);
float FP_Mult(float a_fp, float b_fp);
float FP_Div(float a_fp, float b_fp);
long  FP_ToLong(float a_fp);
unsigned long  FP_ToULong(float a_fp);
float FP_ToFloat(long af);
int   FP_Cmp(float a_fp, float b_fp);
float FP_Sqrt(float a);
float FP_Cos(float rad);
float FP_Sin(float rad);
float FP_Atan(float x);
float FP_Atan2(float y, float x);
float FP_Exp(float x);
float FP_Log(float x);
float FP_Pow(float x, float y);

/***************** Filesys ******************/
#ifndef EXCLUDE_FILESYS
#if 0
#define FILE   OS_FILE
#define fopen  OS_fopen
#define fclose OS_fclose
#define fread  OS_fread
#define fwrite OS_fwrite
#define fseek  OS_fseek
#endif
#endif
#define _FILESYS_
typedef struct OS_FILE_s OS_FILE;
OS_FILE *OS_fopen(char *name, char *mode);
void OS_fclose(OS_FILE *file);
int OS_fread(void *buffer, int size, int count, OS_FILE *file);
int OS_fwrite(void *buffer, int size, int count, OS_FILE *file);
int OS_fseek(OS_FILE *file, int offset, int mode);
int OS_fmkdir(char *name);
int OS_fdir(OS_FILE *dir, char name[64]);
void OS_fdelete(char *name);
int OS_flength(char *entry);

/***************** Flash ******************/
void FlashLock(void);
void FlashUnlock(void);
void FlashRead(uint16 *dst, uint32 byteOffset, int bytes);
void FlashWrite(uint16 *src, uint32 byteOffset, int bytes);
void FlashErase(uint32 byteOffset);

#endif /*__RTOS_H__*/



/* math.c: */

/*--------------------------------------------------------------------
 * TITLE: Plasma Floating Point Library
 * AUTHOR: Steve Rhoads (rhoadss@yahoo.com)
 * DATE CREATED: 3/2/06
 * FILENAME: math.c
 * PROJECT: Plasma CPU core
 * COPYRIGHT: Software placed into the public domain by the author.
 *    Software 'as is' without warranty.  Author liable for nothing.
 * DESCRIPTION:
 *    Plasma Floating Point Library
 *--------------------------------------------------------------------
 * IEEE_fp = sign(1) | exponent(8) | fraction(23)
 * cos(x)=1-x^2/2!+x^4/4!-x^6/6!+...
 * exp(x)=1+x+x^2/2!+x^3/3!+...
 * e^x=2^y; ln2(e^x)=ln2(2^y); ln(e^x)/ln(2)=y; x/ln(2)=y; e^x=2^(x/ln(2))
 * ln(1+x)=x-x^2/2+x^3/3-x^4/4+...
 * atan(x)=x-x^3/3+x^5/5-x^7/7+...
 * pow(x,y)=exp(y*ln(x))
 * x=tan(a+b)=(tan(a)+tan(b))/(1-tan(a)*tan(b))
 * atan(x)=b+atan((x-atan(b))/(1+x*atan(b)))
 * ln(a*x)=ln(a)+ln(x); ln(x^n)=n*ln(x)
 * sqrt(x)=sqrt(f*2^e)=sqrt(f)*2^(e/2)
 *--------------------------------------------------------------------*/
/* #include "rtos.h" */

#define USE_SW_MULT
#if !defined(WIN32) && !defined(USE_SW_MULT)
#define USE_MULT64
#endif

#define PI ((float)3.1415926)
#define PI_2 ((float)(PI/2.0))
#define PI2 ((float)(PI*2.0))

#define FtoL(X) (*(unsigned long*)&(X))
#define LtoF(X) (*(float*)&(X))


float FP_Neg(float a_fp)
{
   unsigned long a;
   a = FtoL(a_fp);
   a ^= 0x80000000;
   return LtoF(a);
}


float FP_Add(float a_fp, float b_fp)
{
   unsigned long a, b, c;
   unsigned long as, bs, cs;     /*sign*/
   long ae, af, be, bf, ce, cf;  /*exponent and fraction*/
   a = FtoL(a_fp);
   b = FtoL(b_fp);
   as = a >> 31;                        /*sign*/
   ae = (a >> 23) & 0xff;               /*exponent*/
   af = 0x00800000 | (a & 0x007fffff);  /*fraction*/
   bs = b >> 31;
   be = (b >> 23) & 0xff;
   bf = 0x00800000 | (b & 0x007fffff);
   if(ae > be) 
   {
      if(ae - be < 30) 
         bf >>= ae - be;
      else 
         bf = 0;
      ce = ae;
   } 
   else 
   {
      if(be - ae < 30) 
         af >>= be - ae;
      else 
         af = 0;
      ce = be;
   }
   cf = (as ? -af : af) + (bs ? -bf : bf);
   cs = cf < 0;
   cf = cf>=0 ? cf : -cf;
   if(cf == 0) 
      return LtoF(cf);
   while(cf & 0xff000000) 
   {
      ++ce;
      cf >>= 1;
   }
   while((cf & 0xff800000) == 0) 
   {
      --ce;
      cf <<= 1;
   }
   c = (cs << 31) | (ce << 23) | (cf & 0x007fffff);
   if(ce < 1) 
      c = 0;
   return LtoF(c);
}


float FP_Sub(float a_fp, float b_fp)
{
   return FP_Add(a_fp, FP_Neg(b_fp));
}


float FP_Mult(float a_fp, float b_fp)
{
   unsigned long a, b, c;
   unsigned long as, af, bs, bf, cs, cf;
   long ae, be, ce;
#ifndef USE_MULT64
   unsigned long a2, a1, b2, b1, med1, med2;
#endif
   unsigned long hi, lo;
   a = FtoL(a_fp);
   b = FtoL(b_fp);
   as = a >> 31;
   ae = (a >> 23) & 0xff;
   af = 0x00800000 | (a & 0x007fffff);
   bs = b >> 31;
   be = (b >> 23) & 0xff;
   bf = 0x00800000 | (b & 0x007fffff);
   cs = as ^ bs;
#ifndef USE_MULT64
   a1 = af & 0xffff;
   a2 = af >> 16;
   b1 = bf & 0xffff;
   b2 = bf >> 16;
   lo = a1 * b1;
   med1 = a2 * b1 + (lo >> 16);
   med2 = a1 * b2;
   hi = a2 * b2 + (med1 >> 16) + (med2 >> 16);
   med1 = (med1 & 0xffff) + (med2 & 0xffff);
   hi += (med1 >> 16);
   lo = (med1 << 16) | (lo & 0xffff);
#else
   lo = OS_AsmMult(af, bf, &hi);
#endif
   cf = (hi << 9) | (lo >> 23);
   ce = ae + be - 0x80 + 1;
   if(cf == 0) 
      return LtoF(cf);
   while(cf & 0xff000000) 
   {
      ++ce;
      cf >>= 1;
   }
   c = (cs << 31) | (ce << 23) | (cf & 0x007fffff);
   if(ce < 1) 
      c = 0;
   return LtoF(c);
}


float FP_Div(float a_fp, float b_fp)
{
   unsigned long a, b, c;
   unsigned long as, af, bs, bf, cs, cf;
   unsigned long a1, b1;
#ifndef USE_MULT64
   unsigned long a2, b2, med1, med2;
#endif
   unsigned long hi, lo;
   long ae, be, ce, d;
   a = FtoL(a_fp);
   b = FtoL(b_fp);
   as = a >> 31;
   ae = (a >> 23) & 0xff;
   af = 0x00800000 | (a & 0x007fffff);
   bs = b >> 31;
   be = (b >> 23) & 0xff;
   bf = 0x00800000 | (b & 0x007fffff);
   cs = as ^ bs;
   ce = ae - (be - 0x80) + 6 - 8;
   a1 = af << 4; /*8*/
   b1 = bf >> 8;
   cf = a1 / b1;
   cf <<= 12; /*8*/
#if 1                  /*non-quick*/
#ifndef USE_MULT64
   a1 = cf & 0xffff;
   a2 = cf >> 16;
   b1 = bf & 0xffff;
   b2 = bf >> 16;
   lo = a1 * b1;
   med1 =a2 * b1 + (lo >> 16);
   med2 = a1 * b2;
   hi = a2 * b2 + (med1 >> 16) + (med2 >> 16);
   med1 = (med1 & 0xffff) + (med2 & 0xffff);
   hi += (med1 >> 16);
   lo = (med1 << 16) | (lo & 0xffff);
#else
   lo = OS_AsmMult(cf, bf, &hi);
#endif
   lo = (hi << 8) | (lo >> 24);
   d = af - lo;    /*remainder*/
   assert(-0xffff < d && d < 0xffff);
   d <<= 16;
   b1 = bf >> 8;
   d = d / (long)b1;
   cf += d;
#endif
   if(cf == 0) 
      return LtoF(cf);
   while(cf & 0xff000000) 
   {
      ++ce;
      cf >>= 1;
   }
   if(ce < 0) 
      ce = 0;
   c = (cs << 31) | (ce << 23) | (cf & 0x007fffff);
   if(ce < 1) 
      c = 0;
   return LtoF(c);
}


long FP_ToLong(float a_fp)
{
   unsigned long a;
   unsigned long as;
   long ae;
   long af, shift;
   a = FtoL(a_fp);
   as = a >> 31;
   ae = (a >> 23) & 0xff;
   af = 0x00800000 | (a & 0x007fffff);
   af <<= 7;
   shift = -(ae - 0x80 - 29);
   if(shift > 0) 
   {
      if(shift < 31) 
         af >>= shift;
      else 
         af = 0;
   }
   af = as ? -af: af;
   return af;
}


/* guessed */
unsigned long FP_ToULong(float a_fp)
{
   unsigned long a;
   unsigned long as;
   long ae;
   long af, shift;
   a = FtoL(a_fp);
   as = a >> 31;
   ae = (a >> 23) & 0xff;
   af = 0x00800000 | (a & 0x007fffff);
   af <<= 7;
   shift = -(ae - 0x80 - 29);
   if(shift > 0) 
   {
      if(shift < 31) 
         af >>= shift;
      else 
         af = 0;
   }
   af = as ? -af: af;
   return af;
}


float FP_ToFloat(long af)
{
   unsigned long a;
   unsigned long as, ae;
   as = af>=0 ? 0: 1;
   af = af>=0 ? af: -af;
   ae = 0x80 + 22;
   if(af == 0) 
      return LtoF(af);
   while(af & 0xff000000) 
   {
      ++ae;
      af >>= 1;
   }
   while((af & 0xff800000) == 0) 
   {
      --ae;
      af <<= 1;
   }
   a = (as << 31) | (ae << 23) | (af & 0x007fffff);
   return LtoF(a);
}


/*0 iff a==b; 1 iff a>b; -1 iff a<b*/
int FP_Cmp(float a_fp, float b_fp)
{
   unsigned long a, b;
   unsigned long as, ae, af, bs, be, bf;
   int gt;
   a = FtoL(a_fp);
   b = FtoL(b_fp);
   if(a == b)
      return 0;
   as = a >> 31;
   bs = b >> 31;
   if(as > bs)
      return -1;
   if(as < bs)
      return 1;
   gt = as ? -1 : 1;
   ae = (a >> 23) & 0xff;
   be = (b >> 23) & 0xff;
   if(ae > be)
      return gt;
   if(ae < be)
      return -gt;
   af = 0x00800000 | (a & 0x007fffff);
   bf = 0x00800000 | (b & 0x007fffff);
   if(af > bf)
      return gt;
   return -gt;
}


int __ltsf2(float a, float b)
{
   return FP_Cmp(a, b);
}

int __lesf2(float a, float b)
{
   return FP_Cmp(a, b);
}

int __gtsf2(float a, float b)
{
   return FP_Cmp(a, b);
}

int __gesf2(float a, float b)
{
   return FP_Cmp(a, b);
}

int __eqsf2(float a, float b)
{
   return FtoL(a) != FtoL(b);
}

int __nesf2(float a, float b)
{
   return FtoL(a) != FtoL(b);
}


float FP_Sqrt(float a)
{
   float x1, y1, x2, y2, x3;
   long i;
   x1 = FP_ToFloat(1);
   y1 = FP_Sub(FP_Mult(x1, x1), a);  /*y1=x1*x1-a;*/
   x2 = FP_ToFloat(100);
   y2 = FP_Sub(FP_Mult(x2, x2), a);
   for(i = 0; i < 10; ++i) 
   {
      if(FtoL(y1) == FtoL(y2)) 
         return x2;     
      /*x3=x2-(x1-x2)*y2/(y1-y2);*/
      x3 = FP_Sub(x2, FP_Div(FP_Mult(FP_Sub(x1, x2), y2), FP_Sub(y1, y2)));
      x1 = x2;
      y1 = y2;
      x2 = x3;
      y2 = FP_Sub(FP_Mult(x2, x2), a);
   }
   return x2;
}


float FP_Cos(float rad)
{
   int n;
   float answer, x2, top, bottom, sign;
   while(FP_Cmp(rad, PI2) > 0) 
      rad = FP_Sub(rad, PI2);
   while(FP_Cmp(rad, (float)0.0) < 0) 
      rad = FP_Add(rad, PI2);
   answer = (float)1.0;
   sign = (float)1.0;
   if(FP_Cmp(rad, PI) >= 0) 
   {
      rad = FP_Sub(rad, PI);
      sign = FP_ToFloat(-1);
   }
   if(FP_Cmp(rad, PI_2) >= 0)
   {
      rad = FP_Sub(PI, rad);
      sign = FP_Neg(sign);
   }
   x2 = FP_Mult(rad, rad);
   top = (float)1.0;
   bottom = (float)1.0;
   for(n = 2; n < 12; n += 2) 
   {
      top = FP_Mult(top, FP_Neg(x2));
      bottom = FP_Mult(bottom, FP_ToFloat((n - 1) * n));
      answer = FP_Add(answer, FP_Div(top, bottom));
   }
   return FP_Mult(answer, sign);
}


float FP_Sin(float rad)
{
   const float pi_2=(float)(PI/2.0);
   return FP_Cos(FP_Sub(rad, pi_2));
}


float FP_Atan(float x)
{
   const float b=(float)(PI/8.0);
   const float atan_b=(float)0.37419668; /*atan(b);*/
   int n;
   float answer, x2, top;
   if(FP_Cmp(x, (float)0.0) >= 0) 
   {
      if(FP_Cmp(x, (float)1.0) > 0) 
         return FP_Sub(PI_2, FP_Atan(FP_Div((float)1.0, x)));
   } 
   else 
   {
      if(FP_Cmp(x, (float)-1.0) > 0) 
         return FP_Sub(-PI_2, FP_Atan(FP_Div((float)1.0, x)));
   }
   if(FP_Cmp(x, (float)0.45) > 0) 
   {
      /*answer = (x - atan_b) / (1 + x * atan_b);*/
      answer = FP_Div(FP_Sub(x, atan_b), FP_Add(1.0, FP_Mult(x, atan_b)));
      /*answer = b + FP_Atan(answer) - (float)0.034633;*/ /*FIXME fudge?*/
      answer = FP_Sub(FP_Add(b, FP_Atan(answer)), (float)0.034633);
      return answer;
   }
   if(FP_Cmp(x, (float)-0.45) < 0)
   {
      x = FP_Neg(x);
      /*answer = (x - atan_b) / (1 + x * atan_b);*/
      answer = FP_Div(FP_Sub(x, atan_b), FP_Add(1.0, FP_Mult(x, atan_b)));
      /*answer = b + FP_Atan(answer) - (float)0.034633;*/ /*FIXME*/
      answer = FP_Sub(FP_Add(b, FP_Atan(answer)), (float)0.034633);
      return FP_Neg(answer);
   }
   answer = x;
   x2 = FP_Mult(FP_Neg(x), x);
   top = x;
   for(n = 3; n < 14; n += 2) 
   {
      top = FP_Mult(top, x2);
      answer = FP_Add(answer, FP_Div(top, FP_ToFloat(n)));
   }
   return answer;
}


float FP_Atan2(float y, float x)
{
   float answer,r;
   r = y / x;
   answer = FP_Atan(r);
   if(FP_Cmp(x, (float)0.0) < 0) 
   {
      if(FP_Cmp(y, (float)0.0) > 0) 
         answer = FP_Add(answer, PI);
      else 
         answer = FP_Sub(answer, PI);
   }
   return answer;
}


float FP_Exp(float x)
{
   const float e2=(float)7.389056099;
   const float inv_e2=(float)0.135335283;
   float answer, top, bottom, mult;
   int n;

   mult = (float)1.0;
   while(FP_Cmp(x, (float)2.0) > 0) 
   {
      mult = FP_Mult(mult, e2);
      x = FP_Add(x, (float)-2.0);
   }
   while(FP_Cmp(x, (float)-2.0) < 0)
   {
      mult = FP_Mult(mult, inv_e2);
      x = FP_Add(x, (float)2.0);
   }
   answer = FP_Add((float)1.0, x);
   top = x;
   bottom = (float)1.0;
   for(n = 2; n < 15; ++n) 
   {
      top = FP_Mult(top, x);
      bottom = FP_Mult(bottom, FP_ToFloat(n));
      answer = FP_Add(answer, FP_Div(top, bottom));
   }
   return FP_Mult(answer, mult);
}


float FP_Log(float x)
{
   const float log_2=(float)0.69314718; /*log(2.0)*/
   int n;
   float answer, top, add;
   add = (float)0.0;
   while(FP_Cmp(x, (float)16.0) > 0)
   {
      x = FP_Mult(x, (float)0.0625);
      add = FP_Add(add, (float)(log_2 * 4));
   }
   while(FP_Cmp(x, (float)1.5) > 0)
   {
      x = FP_Mult(x, (float)0.5);
      add = FP_Add(add, log_2);
   }
   while(FP_Cmp(x, 0.5) < 0)
   {
      x = FP_Mult(x, (float)2.0);
      add = FP_Sub(add, log_2);
   }
   x = FP_Sub(x, (float)1.0);
   answer = (float)0.0;
   top = (float)-1.0;
   for(n = 1; n < 14; ++n) 
   {
      top = FP_Mult(top, FP_Neg(x));
      answer = FP_Add(answer, FP_Div(top, FP_ToFloat(n)));
   }
   return FP_Add(answer, add);
}


float FP_Pow(float x, float y)
{
   return FP_Exp(y * FP_Log(x));
}


/********************************************/
/*These five functions will only be used if the flag "-mno-mul" is enabled*/
#ifdef USE_SW_MULT
unsigned long __mulsi3(unsigned long a, unsigned long b)
{
   unsigned long answer = 0;
   while(b)
   {
      if(b & 1)
         answer += a;
      a <<= 1;
      b >>= 1;
   }
   return answer;
}
#endif


#endif


/*************** Test *****************/
#ifdef WIN32
#undef _LIBC
#include <math.h>
struct {
   char *name;
   float low, high;
   double (*func1)(double);
   float (*func2)(float);
} test_info[]={
   {"cos", -2*PI, 2*PI, cos, FP_Cos},
   {"sin", -2*PI, 2*PI, sin, FP_Sin},
   {"atan", -3.0, 2.0, atan, FP_Atan},
   {"log", (float)0.01, (float)4.0, log, FP_Log},
   {"exp", (float)-5.01, (float)30.0, exp, FP_Exp},
   {"sqrt", (float)0.01, (float)1000.0, sqrt, FP_Sqrt}
};


void TestMathFull(void)
{
   float a, b, c, d;
   float error1, error2, error3, error4, error5;
   int test;

   a = PI * PI;
   b = PI;
   c = FP_Div(a, b);
   printf("%10f %10f %10f %10f %10f\n",
      (double)a, (double)b, (double)(a/b), (double)c, (double)(a/b-c));
   a = a * 200;
   for(b = -(float)2.718281828*100; b < 300; b += (float)23.678) 
   {
      c = FP_Div(a, b);
      d = a / b - c;
      printf("%10f %10f %10f %10f %10f\n",
         (double)a, (double)b, (double)(a/b), (double)c, (double)(a/b-c));
   }
   /*getch();*/

   for(test = 0; test < 6; ++test) 
   {
      printf("\nTesting %s\n", test_info[test].name);
      for(a = test_info[test].low; 
          a <= test_info[test].high;
          a += (test_info[test].high-test_info[test].low)/(float)20.0) 
      {
         b = (float)test_info[test].func1(a);
         c = test_info[test].func2(a);
         d = b - c;
         printf("%s %10f %10f %10f %10f\n", test_info[test].name, a, b, c, d);
      }
      /*getch();*/
   }

   a = FP_ToFloat((long)6.0);
   b = FP_ToFloat((long)2.0);
   printf("%f %f\n", (double)a, (double)b);
   c = FP_Add(a, b);
   printf("add %f %f\n", (double)(a + b), (double)c);
   c = FP_Sub(a, b);
   printf("sub %f %f\n", (double)(a - b), (double)c);
   c = FP_Mult(a, b);
   printf("mult %f %f\n", (double)(a * b), (double)c);
   c = FP_Div(a, b);
   printf("div %f %f\n", (double)(a / b), (double)c);
   /*getch();*/

   for(a = (float)-13756.54; a < (float)17400.0; a += (float)64.45) 
   {
      for(b = (float)-875.36; b < (float)935.8; b += (float)36.7) 
      {
         error1 = (float)1.0 - (a + b) / FP_Add(a, b);
         error2 = (float)1.0 - (a * b) / FP_Mult(a, b);
         error3 = (float)1.0 - (a / b) / FP_Div(a, b);
         error4 = (float)1.0 - a / FP_ToFloat(FP_ToLong(a));
         error5 = error1 + error2 + error3 + error4;
         if(error5 < 0.00005) 
            continue;
         printf("ERROR!\n");
         printf("a=%f b=%f\n", (double)a, (double)b);
         printf("  a+b=%f %f\n", (double)(a+b), (double)FP_Add(a, b));
         printf("  a*b=%f %f\n", (double)(a*b), (double)FP_Mult(a, b));
         printf("  a/b=%f %f\n", (double)(a/b), (double)FP_Div(a, b));
         printf("  a=%f %ld %f\n", (double)a, FP_ToLong(a),
                                   (double)FP_ToFloat((long)a));
         printf("  %f %f %f %f\n", (double)error1, (double)error2,
            (double)error3, (double)error4);
         /*if(error5 > 0.001)*/
         /*   getch(); */
      }
   }
   printf("done.\n");
   /*getch();*/
}
#endif



#if NEEDDIV
static unsigned long DivideMod(unsigned long a, unsigned long b, int doMod)
{
   unsigned long upper=a, lower=0;
   int i;
   a = b << 31;
   for(i = 0; i < 32; ++i)
   {
      lower = lower << 1;
      if(upper >= a && a && b < 2)
      {
         upper = upper - a;
         lower |= 1;
      }
      a = ((b&2) << 30) | (a >> 1);
      b = b >> 1;
   }
   if(!doMod)
      return lower;
   return upper;
}


unsigned long __udivsi3(unsigned long a, unsigned long b)
{
   return DivideMod(a, b, 0);
}


long __divsi3(long a, long b)
{
   long answer, negate=0;
   if(a < 0)
   {
      a = -a;
      negate = !negate;
   }
   if(b < 0)
   {
      b = -b;
      negate = !negate;
   }
   answer = DivideMod(a, b, 0);
   if(negate)
      answer = -answer;
   return answer;
}


unsigned long __umodsi3(unsigned long a, unsigned long b)
{
   return DivideMod(a, b, 1);
}

/* guessing this */
long __modsi3(long a, long b)
{
   long answer, negate=0;
   if(a < 0)
   {
      a = -a;
      negate = !negate;
   }
   if(b < 0)
   {
      b = -b;
      negate = !negate;
   }
   answer = DivideMod(a, b, 1);
   if(negate)
      answer = -answer;
   return answer;
}
#endif





double __negdf2(double a_fp)
{
float a = a_fp;
return -a;
}

double __adddf3(double a_fp, double b_fp)
{
float a = a_fp;
float b = b_fp;
return a + b;
}

double __subdf3(double a_fp, double b_fp)
{
float a = a_fp;
float b = b_fp;
return a - b;
}

double __muldf3(double a_fp, double b_fp)
{
float a = a_fp;
float b = b_fp;
return a * b;
}

double __divdf3(double a_fp, double b_fp)
{
float a = a_fp;
float b = b_fp;
return a / b;
}

int __ltdf2(double a, double b)
{
   return FP_Cmp(a, b);
}

int __ledf2(double a, double b)
{
   return FP_Cmp(a, b);
}

int __gtdf2(double a, double b)
{
   return FP_Cmp(a, b);
}

int __gedf2(double a, double b)
{
   return FP_Cmp(a, b);
}

int __eqdf2(double a_fp, double b_fp)
{
   float a = a_fp;
   float b = b_fp;
   /* we do != here, because the caller is expecting
      0 for equality */
   return FtoL(a) != FtoL(b);
}

int __nedf2(double a_fp, double b_fp)
{
   float a = a_fp;
   float b = b_fp;
   return FtoL(a) != FtoL(b);
}

/* convert long to double */
double __floatsidf(long x)
{
    float y = x;
    return y;
}

/* convert double to long */
long __fixdfsi(double x)
{
    float y = x;
    return (long)y;
}

unsigned long __fixunsdfsi(double x)
{
    float y = x;
    return (unsigned long)y;
}

/* convert double into float */
float __truncdfsf2(double x)
{
    unsigned long y1;
    unsigned long y2;
    unsigned long res;

    y1 = *((unsigned int *)&x + 1);
    y2 = *((unsigned int *)&x + 0);
    res = (y1 & 0xc0000000UL) | ((y1 & 0x07ffffffUL) << 3) | ((y2 & 0xE000000UL) >> 29);
    return (*(float *)&res);
}

/* convert float to double */
double __extendsfdf2(float x)
{
    unsigned long y;
    unsigned long res1;
    unsigned long res2;
    double res;

    y = *(unsigned int *)&x;
    res1 = (y & 0xc0000000UL) | ((y & 0x3fffffffUL) >> 3);
    res2 = (y & 7) << 29;
    *(unsigned int *)&res = res2;
    *((unsigned int *)&res + 1) = res1;
    return res;
}
