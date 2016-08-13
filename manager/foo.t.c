
typedef unsigned long sl_place_t;
extern sl_place_t get_current_place(void);
extern unsigned long get_core_id(void);
extern sl_place_t get_local_place(void);
typedef long ptrdiff_t;
typedef unsigned long size_t;
typedef int wchar_t;
int atoi(const char *);
long atol(const char *);
long long atoll(const char *);
double strtold(const char*restrict, char **restrict);
float strtof(const char*restrict, char**restrict);
long strtol(const char *restrict, char **restrict, int);
long long strtoll(const char *restrict, char **restrict, int);
unsigned long strtoul(const char *restrict, char **restrict, int);
unsigned long long strtoull(const char *restrict, char **restrict, int);
void* malloc(size_t);
void free(void*);
void* calloc(size_t, size_t);
void* realloc(void*, size_t);
void abort(void);
void exit(int status);
void _Exit(int status);
char *getenv(const char *name);
int putenv(char *string);
int setenv(const char *name, const char *value, int overwrite);
int unsetenv(const char *name);
__attribute__((__always_inline__)) static __attribute__((__unused__))
sl_place_t __inline_get_current_place(void)
{

    sl_place_t p;
    __asm__("getpid %0"  : "=r" (p));
    return p;
    abort();
}
__attribute__((__always_inline__)) static __attribute__((__unused__))
unsigned long __inline_get_core_id(void)
{

    unsigned long p;
    __asm__("getcid %0"  : "=r" (p));
    return p;
    abort();
}
__attribute__((__always_inline__)) static __attribute__((__unused__))
sl_place_t __inline_get_local_place(void)
{

    unsigned long cpid = __inline_get_current_place();
    unsigned long cpid_nosize = cpid & (cpid - 1);
    return cpid_nosize | (__inline_get_core_id() << 1) | 1;
}
typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef short int int16_t;
typedef unsigned short int uint16_t;
typedef int int32_t;
typedef unsigned int uint32_t;
typedef long int int64_t;
typedef unsigned long int uint64_t;
typedef signed char int_least8_t;
typedef unsigned char uint_least8_t;
typedef short int int_least16_t;
typedef unsigned short int uint_least16_t;
typedef int int_least32_t;
typedef unsigned int uint_least32_t;
typedef long int int_least64_t;
typedef unsigned long int uint_least64_t;
typedef signed char int_fast8_t;
typedef unsigned char uint_fast8_t;
typedef long int int_fast16_t;
typedef long int int_fast32_t;
typedef long int int_fast64_t;
typedef unsigned long int uint_fast16_t;
typedef unsigned long int uint_fast32_t;
typedef unsigned long int uint_fast64_t;
typedef long int intptr_t;
typedef unsigned long int uintptr_t;
typedef long int intmax_t;
typedef unsigned long int uintmax_t;
typedef struct {

 int nrTests;
 int nrFailed;
} result_t;
void printRead(uint64_t addr, uint64_t data, uint64_t expect, char failed);
char read8( uint64_t addr, uint8_t expect, char abort, char quiet);
char read16(uint64_t addr, uint16_t expect, char abort, char quiet);
char read32(uint64_t addr, uint32_t expect, char abort, char quiet);
char read64(uint64_t addr, uint64_t expect, char abort, char quiet);
void printWrite(uint64_t addr, uint64_t data);
void write8( uint64_t addr, uint8_t data, char quiet);
void write16(uint64_t addr, uint16_t data, char quiet);
void write32(uint64_t addr, uint32_t data, char quiet);
void write64(uint64_t addr, uint64_t data, char quiet);
void printTestStart(char quiet, char name[]);
void printTestEnd(result_t *result, char quiet, char name[]);
void printString(char text[], char quiet);
void pad();
void addSResult(result_t* destination, char result);
void addResults(result_t* destination, result_t* other);
void resetResults(result_t* result);
void run(char abort);
void runAll(result_t *result, sl_place_t dst, char abort);
void runTest(char test, sl_place_t dst, char name[], result_t* result, char abort, char quiet);
void mgsim_control(unsigned long val, unsigned int type, unsigned int command, unsigned int flags);
static inline void
__inline_mgsim_control(unsigned long val, unsigned int type, unsigned int command, unsigned int flags)
{

    volatile unsigned long *base;
    if( type == 0)
        base = (unsigned long*)(0x260UL) + (flags ? 4 : 0) + (command & 3);
    else if( type == 1)
        base = (unsigned long*)(0x300UL) + ((command & 3) << 3) + (flags & 7);
    *base = val;
}
  typedef void * restrict const (*testPtr_t)[2];
  void __slFfmta_testSyscall(void) ;  extern long __slFfseq_testSyscall(const long __slI, register  result_t*  * const __restrict__ __slP_result , register  char  * const __restrict__ __slP_abort , register  char  * const __restrict__ __slP_quiet ) ;  extern void * restrict const testSyscall[2];
  void __slFfmta_testSmallPageAlignedRW(void) ;  extern long __slFfseq_testSmallPageAlignedRW(const long __slI, register  result_t*  * const __restrict__ __slP_result , register  char  * const __restrict__ __slP_abort , register  char  * const __restrict__ __slP_quiet ) ;  extern void * restrict const testSmallPageAlignedRW[2];
  void __slFfmta_testNotLineAlignedR(void) ;  extern long __slFfseq_testNotLineAlignedR(const long __slI, register  result_t*  * const __restrict__ __slP_result , register  char  * const __restrict__ __slP_abort , register  char  * const __restrict__ __slP_quiet ) ;  extern void * restrict const testNotLineAlignedR[2];
  void __slFfmta_testNotLineAlignedRW(void) ;  extern long __slFfseq_testNotLineAlignedRW(const long __slI, register  result_t*  * const __restrict__ __slP_result , register  char  * const __restrict__ __slP_abort , register  char  * const __restrict__ __slP_quiet ) ;  extern void * restrict const testNotLineAlignedRW[2];
  void __slFfmta_testLineBoundariesR(void) ;  extern long __slFfseq_testLineBoundariesR(const long __slI, register  result_t*  * const __restrict__ __slP_result , register  char  * const __restrict__ __slP_abort , register  char  * const __restrict__ __slP_quiet ) ;  extern void * restrict const testLineBoundariesR[2];
  void __slFfmta_testLineBoundariesRW(void) ;  extern long __slFfseq_testLineBoundariesRW(const long __slI, register  result_t*  * const __restrict__ __slP_result , register  char  * const __restrict__ __slP_abort , register  char  * const __restrict__ __slP_quiet ) ;  extern void * restrict const testLineBoundariesRW[2];
  void __slFfmta_testUnalignedR(void) ;  extern long __slFfseq_testUnalignedR(const long __slI, register  result_t*  * const __restrict__ __slP_result , register  char  * const __restrict__ __slP_abort , register  char  * const __restrict__ __slP_quiet ) ;  extern void * restrict const testUnalignedR[2];
  void __slFfmta_testUnalignedRW(void) ;  extern long __slFfseq_testUnalignedRW(const long __slI, register  result_t*  * const __restrict__ __slP_result , register  char  * const __restrict__ __slP_abort , register  char  * const __restrict__ __slP_quiet ) ;  extern void * restrict const testUnalignedRW[2];
  void __slFfmta_testSmallPageBoundariesR(void) ;  extern long __slFfseq_testSmallPageBoundariesR(const long __slI, register  result_t*  * const __restrict__ __slP_result , register  char  * const __restrict__ __slP_abort , register  char  * const __restrict__ __slP_quiet ) ;  extern void * restrict const testSmallPageBoundariesR[2];
  void __slFfmta_testSmallPageBoundariesRW(void) ;  extern long __slFfseq_testSmallPageBoundariesRW(const long __slI, register  result_t*  * const __restrict__ __slP_result , register  char  * const __restrict__ __slP_abort , register  char  * const __restrict__ __slP_quiet ) ;  extern void * restrict const testSmallPageBoundariesRW[2];
  void __slFfmta_testMediumPageBoundariesRW(void) ;  extern long __slFfseq_testMediumPageBoundariesRW(const long __slI, register  result_t*  * const __restrict__ __slP_result , register  char  * const __restrict__ __slP_abort , register  char  * const __restrict__ __slP_quiet ) ;  extern void * restrict const testMediumPageBoundariesRW[2];
  void __slFfmta_testNotSmallPageAligned64RW(void) ;  extern long __slFfseq_testNotSmallPageAligned64RW(const long __slI, register  result_t*  * const __restrict__ __slP_result , register  char  * const __restrict__ __slP_abort , register  char  * const __restrict__ __slP_quiet ) ;  extern void * restrict const testNotSmallPageAligned64RW[2];
  void __slFfmta_testNotMediumPageAligned4096RW(void) ;  extern long __slFfseq_testNotMediumPageAligned4096RW(const long __slI, register  result_t*  * const __restrict__ __slP_result , register  char  * const __restrict__ __slP_abort , register  char  * const __restrict__ __slP_quiet ) ;  extern void * restrict const testNotMediumPageAligned4096RW[2];
  void __slFfmta_testNotLargePageAligned2MRW(void) ;  extern long __slFfseq_testNotLargePageAligned2MRW(const long __slI, register  result_t*  * const __restrict__ __slP_result , register  char  * const __restrict__ __slP_abort , register  char  * const __restrict__ __slP_quiet ) ;  extern void * restrict const testNotLargePageAligned2MRW[2];
  void __slFfmta_testAllPageSizes(void) ;  extern long __slFfseq_testAllPageSizes(const long __slI, register  result_t*  * const __restrict__ __slP_result , register  char  * const __restrict__ __slP_abort , register  char  * const __restrict__ __slP_quiet ) ;  extern void * restrict const testAllPageSizes[2];
  void __slFfmta_testTLBEntryUnlock(void) ;  extern long __slFfseq_testTLBEntryUnlock(const long __slI, register  result_t*  * const __restrict__ __slP_result , register  char  * const __restrict__ __slP_abort , register  char  * const __restrict__ __slP_quiet ) ;  extern void * restrict const testTLBEntryUnlock[2];
  void __slFfmta_testFullSet(void) ;  extern long __slFfseq_testFullSet(const long __slI, register  result_t*  * const __restrict__ __slP_result , register  char  * const __restrict__ __slP_abort , register  char  * const __restrict__ __slP_quiet ) ;  extern void * restrict const testFullSet[2];
  void __slFfmta_testFullTLB(void) ;  extern long __slFfseq_testFullTLB(const long __slI, register  result_t*  * const __restrict__ __slP_result , register  char  * const __restrict__ __slP_abort , register  char  * const __restrict__ __slP_quiet ) ;  extern void * restrict const testFullTLB[2];
testPtr_t p_testSyscall = testSyscall;
testPtr_t p_testSmallPageAlignedRW = &testSmallPageAlignedRW;
testPtr_t p_testNotLineAlignedR = &testNotLineAlignedR;
testPtr_t p_testNotLineAlignedRW = &testNotLineAlignedRW;
testPtr_t p_testLineBoundariesR = &testLineBoundariesR;
testPtr_t p_testLineBoundariesRW = &testLineBoundariesRW;
testPtr_t p_testUnalignedR = &testUnalignedR;
testPtr_t p_testUnalignedRW = &testUnalignedRW;
testPtr_t p_testSmallPageBoundariesR = &testSmallPageBoundariesR;
testPtr_t p_testSmallPageBoundariesRW = &testSmallPageBoundariesRW;
testPtr_t p_testMediumPageBoundariesRW = &testMediumPageBoundariesRW;
testPtr_t p_testNotSmallPageAligned64RW = &testNotSmallPageAligned64RW;
testPtr_t p_testNotMediumPageAligned4096RW = &testNotMediumPageAligned4096RW;
testPtr_t p_testNotLargePageAligned2MRW = &testNotLargePageAligned2MRW;
testPtr_t p_testAllPageSizes = &testAllPageSizes;
testPtr_t p_testTLBEntryUnlock = &testTLBEntryUnlock;
testPtr_t p_testFullSet = &testFullSet;
testPtr_t p_testFullTLB = &testFullTLB;
void runAll(result_t* result, sl_place_t dst, char abort){

 __inline_mgsim_control(0, 0, 1, 0);
 runTest(0 , dst, "Small Page Aligned RW" , result, abort, 1);
 __inline_mgsim_control(0, 0, 1, 0);
 runTest(1 , dst, "Not line aligned R" , result, abort, 1);
 runTest(2 , dst, "Not line aligned RW" , result, abort, 1);
 runTest(3 , dst, "Line boundaries R" , result, abort, 1);
 runTest(4 , dst, "Line boundaries RW" , result, abort, 1);
 runTest(5 , dst, "Unaligned R" , result, abort, 1);
 runTest(6 , dst, "Unaligned RW" , result, abort, 1);
 runTest(7 , dst, "Page boundaries of smallest page R" , result, abort, 1);
 runTest(8 , dst, "Page boundaries of smallest page RW" , result, abort, 1);
 runTest(9 , dst, "Page boundaries of medium page RW" , result, abort, 1);
 runTest(10 , dst, "Not page aligned RW (small page, 64 bits)" , result, abort, 1);
 runTest(11 , dst, "Not page aligned RW (medium page, 64 bits per 4096 bytes)" , result, abort, 1);
 runTest(12 , dst, "Not page aligned RW (large page, 64 bits per 2MiB)" , result, abort, 1);
 runTest(13 , dst, "Syscall test" , result, abort, 1);
}
void runTest(char test, sl_place_t dst, char name[], result_t* result, char abort, char quiet){

 result_t localResult;
 resetResults(&localResult);
 printTestStart(quiet, name);
 switch(test){  long __Scv$C$R$F1$0 __attribute__((unused));  long __Scv$C$F$F1$1;  long __Scv$C$place$F1$2;  long __Scv$C$start$F1$3;  long __Scv$C$limit$F1$4;  long __Scv$C$step$F1$5;  long __Scv$C$block$F1$6;  result_t* __Scv$C$a$result0$7;  result_t* __Scv$C$ai$result0$8;  char __Scv$C$a$__slanon59$9;  char __Scv$C$ai$__slanon59$10;  char __Scv$C$a$__slanon60$11;  char __Scv$C$ai$__slanon60$12;  long __Scv$C$R$F2$13 __attribute__((unused));  long __Scv$C$F$F2$14;  long __Scv$C$place$F2$15;  long __Scv$C$start$F2$16;  long __Scv$C$limit$F2$17;  long __Scv$C$step$F2$18;  long __Scv$C$block$F2$19;  result_t* __Scv$C$a$result1$20;  result_t* __Scv$C$ai$result1$21;  char __Scv$C$a$__slanon62$22;  char __Scv$C$ai$__slanon62$23;  char __Scv$C$a$__slanon63$24;  char __Scv$C$ai$__slanon63$25;  int __Scv$C$S$F1$26 = (0);  int __Scv$C$S$F2$27 = (0);

  case 0:
    __Scv$C$place$F1$2 = ( dst  ); __Scv$C$start$F1$3 = ( 0  ); __Scv$C$limit$F1$4 = ( 1  ); __Scv$C$step$F1$5 = ( 1  ); __Scv$C$block$F1$6 = ( 0  ); __Scv$C$ai$result0$8 = ( &localResult  ); __Scv$C$ai$__slanon59$10 = ( abort  ); __Scv$C$ai$__slanon60$12 = ( quiet  ); __Scv$C$S$F1$26 = 3;   goto __Scl$Cn$cmta$F1$0 ;  __Scl$Cn$cmta$F1$0: (void)0; __asm__ __volatile__("allocate/x %2, %1, %0\t# MT: CREATE F1" : "=r"( __Scv$C$F$F1$1) : "rI"( __Scv$C$S$F1$26), "r"( __Scv$C$place$F1$2)); __asm__ ("setstart %0, %2\t# MT: CREATE F1" : "=r"( __Scv$C$F$F1$1) : "0"( __Scv$C$F$F1$1), "rI"( __Scv$C$start$F1$3)); __asm__ ("setlimit %0, %2\t# MT: CREATE F1" : "=r"( __Scv$C$F$F1$1) : "0"( __Scv$C$F$F1$1), "rI"( __Scv$C$limit$F1$4)); __asm__ ("setstep %0, %2\t# MT: CREATE F1" : "=r"( __Scv$C$F$F1$1) : "0"( __Scv$C$F$F1$1), "rI"( __Scv$C$step$F1$5)); __asm__ ("setblock %0, %2\t# MT: CREATE F1" : "=r"( __Scv$C$F$F1$1) : "0"( __Scv$C$F$F1$1), "rI"( __Scv$C$block$F1$6)); __asm__ __volatile__("crei %0, 0(%2)\t# MT: CREATE F1" : "=r"( __Scv$C$F$F1$1) : "0"( __Scv$C$F$F1$1),   "r"(__slFfmta_testSmallPageAlignedRW) : "memory");;  __asm__ ("puts %2, %0, 0\t# MT: set sharg" : "=r"( __Scv$C$F$F1$1) : "0"( __Scv$C$F$F1$1), "rI"((result_t*)( __Scv$C$ai$result0$8)));  __asm__ ("puts %2, %0, 1\t# MT: set sharg" : "=r"( __Scv$C$F$F1$1) : "0"( __Scv$C$F$F1$1), "rI"((char)( __Scv$C$ai$__slanon59$10)));  __asm__ ("puts %2, %0, 2\t# MT: set sharg" : "=r"( __Scv$C$F$F1$1) : "0"( __Scv$C$F$F1$1), "rI"((char)( __Scv$C$ai$__slanon60$12)))
;
    __asm__ __volatile__("sync %0, %1;  mov %1, $31\t# MT: SYNC F1" : "=r"( __Scv$C$F$F1$1), "=r"( __Scv$C$R$F1$0) : "0"( __Scv$C$F$F1$1) : "memory");  __asm__ __volatile__("release %0\t#MT: SYNC F1" : : "r"( __Scv$C$F$F1$1));  goto __Scl$Ce$F1$1 ; ;  __Scl$Ce$F1$1: (void)0;;
  break;
  case 1:
    __Scv$C$place$F2$15 = ( dst  ); __Scv$C$start$F2$16 = ( 0  ); __Scv$C$limit$F2$17 = ( 1  ); __Scv$C$step$F2$18 = ( 1  ); __Scv$C$block$F2$19 = ( 0  ); __Scv$C$ai$result1$21 = ( &localResult  ); __Scv$C$ai$__slanon62$23 = ( abort  ); __Scv$C$ai$__slanon63$25 = ( quiet  ); __Scv$C$S$F2$27 = 3;   goto __Scl$Cn$cmta$F2$2 ;  __Scl$Cn$cmta$F2$2: (void)0; __asm__ __volatile__("allocate/x %2, %1, %0\t# MT: CREATE F2" : "=r"( __Scv$C$F$F2$14) : "rI"( __Scv$C$S$F2$27), "r"( __Scv$C$place$F2$15)); __asm__ ("setstart %0, %2\t# MT: CREATE F2" : "=r"( __Scv$C$F$F2$14) : "0"( __Scv$C$F$F2$14), "rI"( __Scv$C$start$F2$16)); __asm__ ("setlimit %0, %2\t# MT: CREATE F2" : "=r"( __Scv$C$F$F2$14) : "0"( __Scv$C$F$F2$14), "rI"( __Scv$C$limit$F2$17)); __asm__ ("setstep %0, %2\t# MT: CREATE F2" : "=r"( __Scv$C$F$F2$14) : "0"( __Scv$C$F$F2$14), "rI"( __Scv$C$step$F2$18)); __asm__ ("setblock %0, %2\t# MT: CREATE F2" : "=r"( __Scv$C$F$F2$14) : "0"( __Scv$C$F$F2$14), "rI"( __Scv$C$block$F2$19)); __asm__ __volatile__("crei %0, 0(%2)\t# MT: CREATE F2" : "=r"( __Scv$C$F$F2$14) : "0"( __Scv$C$F$F2$14),   "r"(__slFfmta_testNotLineAlignedR) : "memory");;  __asm__ ("puts %2, %0, 0\t# MT: set sharg" : "=r"( __Scv$C$F$F2$14) : "0"( __Scv$C$F$F2$14), "rI"((result_t*)( __Scv$C$ai$result1$21)));  __asm__ ("puts %2, %0, 1\t# MT: set sharg" : "=r"( __Scv$C$F$F2$14) : "0"( __Scv$C$F$F2$14), "rI"((char)( __Scv$C$ai$__slanon62$23)));  __asm__ ("puts %2, %0, 2\t# MT: set sharg" : "=r"( __Scv$C$F$F2$14) : "0"( __Scv$C$F$F2$14), "rI"((char)( __Scv$C$ai$__slanon63$25)))
;
    __asm__ __volatile__("sync %0, %1;  mov %1, $31\t# MT: SYNC F2" : "=r"( __Scv$C$F$F2$14), "=r"( __Scv$C$R$F2$13) : "0"( __Scv$C$F$F2$14) : "memory");  __asm__ __volatile__("release %0\t#MT: SYNC F2" : : "r"( __Scv$C$F$F2$14));  goto __Scl$Ce$F2$3 ; ;  __Scl$Ce$F2$3: (void)0;;
   break;
 }
 printTestEnd(&localResult, quiet, name);
 addResults(result, &localResult);
}
