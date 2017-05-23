// Miscellaneous defines - there are no official TIGCC headers for Fargo yet.
#define __ATTR_STD__ __attribute__((__stkparm__))
#define __ATTR_STD_NORETURN__ __attribute__((__stkparm__,__noreturn__))
#define CALLBACK __ATTR_STD__
#define __ATTR_TIOS__ __ATTR_STD__
#define __ATTR_TIOS_NORETURN__ __ATTR_STD_NORETURN__
#define __ATTR_TIOS_CALLBACK__ CALLBACK
#define __ATTR_GCC__ 
#define __ATTR_LIB_C__ __attribute__((__regparm__(4)))
#define __ATTR_LIB_ASM__ __ATTR_STD__

typedef unsigned short HANDLE; 


typedef union { 
 struct { 
 unsigned char x0, y0, x1, y1; 
 } xy; 
 unsigned long l; 
} SCR_RECT; 

#define sprintf tios__000f
short sprintf (char *buffer, const char *format, ...);


enum CommonKeys {KEY_F1 = 268, KEY_F2 = 269, KEY_F3 = 270, KEY_F4 = 271, KEY_F5 = 272, KEY_F6 = 273, KEY_F7 = 274, KEY_F8 = 275, KEY_ESC = 264, KEY_QUIT = 4360, KEY_APPS = 265, KEY_SWITCH = 4361, KEY_MODE = 266, KEY_BACKSPACE = 257, KEY_INS = 4353, KEY_CLEAR = 263, KEY_VARLNK = 4141, KEY_CHAR = 4139, KEY_ENTER = 13, KEY_ENTRY = 4109, KEY_STO = 258, KEY_RCL = 4354, KEY_SIGN = 173, KEY_MATH = 4149, KEY_MEM = 4150, KEY_ON = 267, KEY_OFF = 4363};

extern long __randseed;
#define abs(x) ({typeof(x) __x = (x); __x >= 0 ? __x : -__x;}) 


enum Fonts {F_4x6, F_6x8, F_8x10}; 
enum Attrs {A_REVERSE, A_NORMAL, A_XOR, A_SHADED, A_REPLACE, A_OR, A_AND, A_THICK1, A_SHADE_V, A_SHADE_H, A_SHADE_NS, A_SHADE_PS}; 

#define DrawStr tios__0010
void DrawStr (short x, short y, const char *str, short Attr);
#define FontSetSys tios__0012
unsigned char FontSetSys (short Font); 

#define memset tios__0032
void *memset (void *buffer, short c, unsigned long num); 
#define memcpy tios__0034
void *memcpy (void *dest, const void *src, unsigned long len); 
#define memmove tios__0035
void *memmove (void *dest, const void *src, unsigned long len); 
#define strncmp tios__0029
short strncmp (const unsigned char *s1, const unsigned char *s2, unsigned long maxlen);

#define strlen tios__0028
int strlen (const unsigned char *s1);

#define PortSet tios__0015
void PortSet (void *vm_addr, short x_max, short y_max); 

#define HeapAlloc tios__0003
HANDLE HeapAlloc (unsigned long Size); 
#define HeapFree tios__0002
void HeapFree (HANDLE Handle); 
#define HeapRealloc tios__000E
HANDLE HeapRealloc (HANDLE Handle, unsigned long NewSize);
#define __tios_globals tios__001C
extern void __tios_globals;
#define __heap (*(void***)(&__tios_globals+0x1902))
#define HeapDeref(__h) (__heap[__h])
#define HeapLock(__h) (((unsigned short*)HeapDeref((__h)))[-1]|=0x8000U)

#define MoveTo tios__0014
void MoveTo (short x, short y); 
#define DrawTo tios__0013
void DrawTo (short x, short y); 

#define __scr_attr (*(short*)(&__tios_globals+6104))
#define SetCurAttr(__a) ({short __oa=__scr_attr;__scr_attr=(__a);__oa;})
#define __scr_clip (*(unsigned long*)(&__tios_globals+6110))
#define SetCurClip(__c) (__scr_clip=*(unsigned long*)(__c))
#define ST_helpMsg tios__0001

void ST_helpMsg (const char *msg); 

typedef CALLBACK short (*compare_t) (const void *elem1, const void *elem2);
extern void qsort(void*,short,short,compare_t)__ATTR_LIB_C__;
extern short rand(void)__ATTR_LIB_ASM__;
#define random(x) ((short)((long)(unsigned short)rand()*(unsigned short)(x)/32768))
#define srand(x) (__randseed=(x))

typedef struct { 
 char name[8]; 
 union { 
 unsigned short flags_n; 
 struct { 
 unsigned short reserved : 8; 
 unsigned short folder : 1, overwritten : 1, checked : 1, hidden : 1, locked : 1, statvar : 1, graph_ref_1 : 1, graph_ref_0 : 1; 
 } bits; 
 } flags; 
 HANDLE handle; 
} SYM_ENTRY;
#define STR_TAG ((unsigned char)0x2D)
#define FindSymEntry tios__0024
SYM_ENTRY *FindSymEntry(HANDLE symlist, const char *name);
#define MainHandle 0xc
#define KEY_OFF2 (8459u)
#define __apd_init (*(unsigned long*)(&__tios_globals+0xf10))
#define __apd_current (*(volatile unsigned long*)(&__tios_globals+0xf14))
#define __apd_expired (*(volatile short*)(&__tios_globals+0xf42))
#define OSTimerExpired(__dummy) (__apd_expired)
#define OSTimerRestart(__dummy) ({__apd_expired=0;__apd_current=__apd_init;})

#define __kb_globals tios__001b
extern void __kb_globals;
#define kbhit() (*(volatile short*)(&__kb_globals+0x1c))
#define ngetchx() ({kbhit()=0;*(volatile short*)(&__kb_globals+0x1e);})

#define pokeIO(port,val) (void)(*((volatile unsigned char*)(long)(port)) = (val)) 
#define __ROM_base tios__0025
extern void __ROM_base;
#define ROM_base (&__ROM_base)
enum Buttons {BT_NONE, BT_OK, BT_SAVE, BT_YES, BT_CANCEL, BT_NO, BT_GOTO}; 

#define MenuPopup tios__001E
unsigned short MenuPopup (const void *MenuPtr, short x, short y, unsigned short start_option);

#define CENTER (-1)
#define ER_LINK_IO 650
#define ER_MEMORY 670

typedef struct ErrorFrameStruct{unsigned long A2,A3,A4,A5,A6,A7;unsigned long D3,D4,D5,D6,D7;unsigned long NG_control;char*RetIndex;unsigned long PC;struct ErrorFrameStruct*Link;}ERROR_FRAME[1];

#define ENDTRY ;_ONERR_=0;}}
#define ONERR ER_success();}else{register short _ONERR_=1;
#define TRY {ERROR_FRAME __errFrame;unsigned short errCode;errCode=ER_catch(__errFrame);if(!errCode){
asm(".set _A_LINE,0xA000");
#define __CONST_INT_TO_ERR_LABEL(x) _ER_CODE_##x
#define _ER_throw(err_no) ({extern void __CONST_INT_TO_ERR_LABEL(err_no);goto*(&(__CONST_INT_TO_ERR_LABEL(err_no)));})
#define ER_throw(err_no) _ER_throw(err_no)
#define ER_catch tios__0004
short ER_catch (void *ErrorFrame); 
#define ER_success tios__0005
void ER_success (void); 
#define ERD_dialog tios__002F
short ERD_dialog (short err_no, short prog_flag);
#define OSLinkTxQueueInquire tios__0008
unsigned short OSLinkTxQueueInquire (void); 
#define OSReadLinkBlock tios__000A
unsigned short OSReadLinkBlock (char *buffer, unsigned short num); 
#ifndef __HAVE_INT_HANDLER
#define __HAVE_INT_HANDLER
typedef struct __attribute__((__may_alias__)){short foo;}_DEREF_INT_HANDLER,*INT_HANDLER;
#endif
enum IntVecs{AUTO_INT_1=0x64,AUTO_INT_2=0x68,AUTO_INT_3=0x6C,AUTO_INT_4=0x70,AUTO_INT_5=0x74,AUTO_INT_6=0x78,AUTO_INT_7=0x7C,TRAP_0=0x80,TRAP_1=0x84,TRAP_2=0x88,TRAP_3=0x8C,TRAP_4=0x90,TRAP_5=0x94,TRAP_6=0x98,TRAP_7=0x9C,TRAP_8=0xA0,TRAP_9=0xA4,TRAP_10=0xA8,TRAP_11=0xAC,TRAP_12=0xB0,TRAP_13=0xB4,TRAP_14=0xB8,TRAP_15=0xBC,INT_VEC_RESET=0x04,INT_VEC_BUS_ERROR=0x08,INT_VEC_ADDRESS_ERROR=0x0C,INT_VEC_ILLEGAL_INSTRUCTION=0x10,INT_VEC_ZERO_DIVIDE=0x14,INT_VEC_CHK_INS=0x18,INT_VEC_TRAPV_INS=0x1C,INT_VEC_PRIVILEGE_VIOLATION=0x20,INT_VEC_TRACE=0x24,INT_VEC_LINE_1010=0x28,INT_VEC_LINE_1111=0x2C,INT_VEC_UNINITIALIZED_INT=0x3C,INT_VEC_SPURIOUS_INT=0x60,INT_VEC_KEY_PRESS=0x68,INT_VEC_LINK=0x70,INT_VEC_ON_KEY_PRESS=0x78,INT_VEC_STACK_OVERFLOW=0x7C,INT_VEC_INT_MASK=0x84,INT_VEC_MANUAL_RESET=0x88,INT_VEC_OFF=0x90,INT_VEC_SELF_TEST=0xA8,INT_VEC_ARCHIVE=0xAC,INT_VEC_ER_THROW=0xBC};
#define GetIntVec(i) (*(INT_HANDLER*)(i))
#define SetIntVec(i,h) ((void)(*(INT_HANDLER*)(0x40000+(long)(i))=(h)))
#define peekIO(port) (*((volatile unsigned char*)(long)(port)))
