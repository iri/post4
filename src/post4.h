/*
 * post4.h
 *
 * Copyright 2007, 2024 by Anthony Howe. All rights reserved.
 */

#ifndef __post4_h__
#define __post4_h__	1

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************************
 *** Configurables
 ***********************************************************************/

#define _DEFAULT_SOURCE			1
#define _XOPEN_SOURCE			700
#define _POSIX_C_SOURCE			200809L

#ifdef __APPLE__
# define _DARWIN_C_SOURCE 		1
#endif

#ifdef __NetBSD__
# define _NETBSD_SOURCE			1
#endif
#ifndef ECHOCTL
# define ECHOCTL			0
#endif

#include "config.h"
#include "build.h"

#ifndef NDEBUG
/* With debugging, start with small stacks to test stack growth. */
#define P4_DATA_STACK_SIZE		8		/* in CELLS */
#define P4_FLOAT_STACK_SIZE		6		/* in CELLS */
#define P4_RETURN_STACK_SIZE		16		/* in CELLS */
#endif

#ifndef P4_MEM_SIZE
/* When selecting the default data space memory size, consider the number
 * of cells that will be available for defining the core and new words.
 * Historically a 64KB system with 16 bit cell allowed for 32K of cells.
 * For 64KB with 64 bit cells that is only 8K of cells.  See -m option.
 */
#define P4_MEM_SIZE			128		/* in kilo-bytes */
#endif

#ifndef P4_BLOCK_SIZE
#define P4_BLOCK_SIZE			1024		/* in bytes */
#endif

#ifndef P4_NAME_SIZE
#define P4_NAME_SIZE			32		/* in bytes */
#endif

#ifndef P4_DATA_STACK_SIZE
#define P4_DATA_STACK_SIZE		64		/* in CELLS */
#endif

#ifndef P4_RETURN_STACK_SIZE
#define P4_RETURN_STACK_SIZE		64		/* in CELLS */
#endif

#ifndef P4_FLOAT_STACK_SIZE
#define P4_FLOAT_STACK_SIZE		6		/* in CELLS */
#endif

#ifndef P4_STACK_EXTRA
#define P4_STACK_EXTRA			16		/* in CELLS, power of 2 */
#endif

#ifndef P4_INPUT_SIZE
#define P4_INPUT_SIZE			256		/* in bytes */
#endif

#ifndef P4_WORDLISTS
#define P4_WORDLISTS			12
#endif

#ifndef P4_CORE_PATH
/* Path list of potential package library directories where the core
 * file and friends can be found.  Used to include /usr/lib/post4,
 * but /usr/lib should strictly be reserved for OS and tool libraries,
 * not 3rd party stuff.
 */
#define P4_CORE_PATH			"/usr/local/lib/post4:/usr/pkg/lib/post4"
#endif

#ifndef P4_CORE_FILE
#define P4_CORE_FILE			"post4.p4"
#endif

#ifndef P4_PAD_SIZE
#define P4_PAD_SIZE			(P4_INPUT_SIZE)
#endif

#ifndef P4_PIC_SIZE
#define P4_PIC_SIZE			(2 * sizeof (P4_Cell) * CHAR_BIT + 2)
#endif

#ifndef P4_TRACE
#define P4_TRACE			1
#endif

#ifdef WITH_JAVA
#define HAVE_HOOKS			1
#endif

#ifndef NL
#define NL				"\n"
#endif

/***********************************************************************
 *** No configuration below this point.
 ***********************************************************************/

#include <ctype.h>
#include <errno.h>
#include <assert.h>
#include <getopt.h>
#include <limits.h>
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>	/* GH-35 strncasecmp.  Bugger me. */
#include <setjmp.h>
#include <signal.h>
#include <time.h>

#include "ansiterm.h"

#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#ifdef HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif

#if HAVE_INTTYPES_H
# include <inttypes.h>
#else
# if HAVE_STDINT_H
#  include <stdint.h>
# endif
#endif

#ifdef HAVE_MATH_H
# include <math.h>
# include <float.h>
#endif
#ifdef HAVE_TERMIOS_H
# include <termios.h>
#endif
#ifdef HAVE_SYS_IOCTL_H
# include <sys/ioctl.h>
#endif
#ifdef HAVE_FCNTL_H
# include <fcntl.h>
#endif
#ifdef HAVE_SYS_FILE_H
# include <sys/file.h>
#endif
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif

/***********************************************************************
 ***
 ***********************************************************************/

#define QUOTE(x)	QUOTE_THIS(x)
#define QUOTE_THIS(x)	#x
#define STRLEN(s)	(sizeof (s)-1)

/*
 * Help detect uninitialised memory issues or memory corruption.  Some
 * systems allocate cleared (NetBSD) or undefined (Cygwin) memory.  The
 * former can hide uninitialised data problems.  When debugging, setting
 * newly allocated memory to garbage can help expose these issues.
 *
 * Also debugging with Valgrind, often reports "...points to uninitialised
 * byte(s)" when part of a buffer remains unused.  Should only be disabled
 * once you are sure that what Valgrind is reporting can be safely ignored.
 */
#ifdef NDEBUG
# define MEMSET(p, b, s)
#else
# define MEMSET(p, b, s)	if ((p) != NULL && 0 < (s)) { memset((void *)(p),(int)(b),(size_t)(s)); }
#endif

#define BYTE_ME		(0x7F)

#ifndef LONG_BIT
# if LONG_MAX == 0x7FFFFFFFL
#  define LONG_BIT	32
# elif LONG_MAX == 0x7FFFFFFFFFFFFFFFL
#  define LONG_BIT	64
# else
#  error "Unexpected sizeof long."
# endif
#endif

#define STDERR		stdout

#ifdef USE_EXCEPTION_STRINGS
# define THROW_MSG(e)	(void) fprintf(STDERR, "%d thrown: %s", (e), P4_THROW_future <= (e) && (e) < 0 ? p4_exceptions[-(e)] : "?")
#else
# define THROW_MSG(e)	(void) fprintf(STDERR, "%d thrown", (e))
#endif

/***********************************************************************
 *** Types
 ***********************************************************************/

#define P4_FALSE	( 0)
#define P4_TRUE		(~0)
#define P4_BOOL(x)	((P4_Int) -(x))		/* negate 2's complement C boolean */

typedef void *P4_Code;			/* Address of labels, eg. ptr = &&label; */
typedef void *P4_Ptr;
#define P4_PTR_FMT "%p"

typedef unsigned char	P4_Char;
typedef uintptr_t 	P4_Uint;
typedef intptr_t	P4_Int;

#define P4_CHAR_FMT	"0x%.2x"
#define P4_CHAR_BIT	CHAR_BIT
#define P4_CHAR_MAX	UCHAR_MAX

#define P4_INT_MAX	INTPTR_MAX
#define P4_UINT_MAX	UINTPTR_MAX
#define P4_UINT_BITS	LONG_BIT

#if P4_UINT_BITS == 64
typedef uint32_t	P4_Uint_Half;
typedef double		P4_Float;
# define P4_INT_FMT	"%ld"
# define P4_UINT_FMT	"%lu"
# define P4_FLT_PRE_FMT	"%.*lF"
# define P4_SCI_FMT	"%lE"
# define P4_SCI_PRE_FMT	"%.*lE"
# define P4_HEX_FMT 	"$%lx"
# define P4_H0X_FMT 	"$%.16lx"
# define DIV		ldiv
# define DIV_T		ldiv_t
# define P4_SENTINEL	(0xdeadbeefcafebabeUL)
# define P4_MARKER	(0xfeedfacedeafdeedUL)
#elif P4_UINT_BITS == 32
typedef uint16_t	P4_Uint_Half;
typedef float		P4_Float;
# define P4_INT_FMT	"%d"
# define P4_UINT_FMT	"%u"
# define P4_HEX_FMT 	"$x"
# define P4_H0X_FMT 	"$%.8x"
# define P4_FLT_PRE_FMT	"%.*F"
# define P4_SCI_FMT	"%E"
# define P4_SCI_PRE_FMT	"%.*E"
# define DIV		div
# define DIV_T		div_t
# define P4_SENTINEL	(0xdeadbeefUL)
# define P4_MARKER	(0xfeedfaceUL)
#else
# error "Unsupported cell size."
#endif

#define P4_UINT_MSB	(~(~(P4_Uint)0 >> 1))
#define P4_HALF_SHIFT	(P4_UINT_BITS >> 1)
#define P4_LOWER_MASK	(~(P4_Uint)0 >> P4_HALF_SHIFT)

typedef size_t P4_Size;
#define P4_SIZE_FMT "%zu"

typedef time_t P4_Time;
#define P4_TIME_FMT 	P4_UINT_FMT

typedef union p4_cell P4_Cell;
typedef struct p4_word P4_Word;
typedef struct p4_ctx P4_Ctx;

typedef P4_Word *P4_Nt;
typedef P4_Word *P4_Xt;			/* Currently same as an nt, but could change. */

typedef struct {
	P4_Int argc;
	char **argv;
	P4_Int trace;
	P4_Uint mem_size;
	P4_Uint hist_size;
	const char *core_file;
	const char *block_file;
} P4_Options;

typedef struct {
	P4_Size		length;		/* Length of string less NUL byte. */
	char *		string;		/* Pointer to content plus terminating NUL byte. */
} P4_String;

typedef struct {
	FILE *		fp;		/* stdin or an open file, -1 string. */
	P4_Uint		blk;		/* If 0< then buffer is a block and this is the block number. */
	P4_Size		length;		/* Length of input in buffer. */
	P4_Size		offset;		/* Offset of unconsumed input. */
	const char *	path;
	char *		buffer;
	char		data[P4_INPUT_SIZE];
} P4_Input;

#define P4_INPUT_IS_TERM(input)	((input)->fp == stdin)
#define P4_INPUT_IS_EVAL(input)	((input)->fp == (FILE *) -1)
#define P4_INPUT_IS_BLK(input)	(P4_INPUT_IS_EVAL(input) && (input)->blk > 0)
#define P4_INPUT_IS_STR(input)	(P4_INPUT_IS_EVAL(input) && (input)->blk == 0)
#define P4_INPUT_IS_FILE(input) (!P4_INPUT_IS_EVAL(input) && !P4_INPUT_IS_TERM(input))
#define P4_INPUT_PUSH(input)	{ P4_Input *input_save = (input); input = p4CreateInput()
#define P4_INPUT_POP(input)	free(input); (input) = input_save; }

typedef enum {
	P4_BLOCK_FREE,
	P4_BLOCK_CLEAN,
	P4_BLOCK_DIRTY,
} P4_Block_State;

typedef struct {
	P4_Int		state;
	P4_Uint		number;
	P4_Char		buffer[P4_BLOCK_SIZE];
} P4_Block;

union p4_cell {
	P4_Int		n;
	P4_Uint		u;
	P4_Size		z;
	P4_Time		t;
	P4_Float	f;
	P4_Cell	*	p;
	char *		s;
	void *		v;
	P4_Nt		nt;
	P4_Xt		xt;
	const P4_Word *	cw;
};

#define P4_CELL				sizeof (P4_Cell)
#define P4_ALIGN_SIZE(sz, pow2)    	(((P4_Uint) (sz) + (pow2-1)) & -(pow2))
#define P4_CELL_ALIGN(nbytes)		P4_ALIGN_SIZE(nbytes, sizeof (P4_Cell))
#define P4_ALIGN_BY(nbytes)		(P4_CELL_ALIGN(nbytes) - (P4_Uint)(nbytes))

struct p4_word {
	/* Header */
	P4_Word *	prev;		/* Previous word definition. */
	P4_Size		length;
	char *		name;
	P4_Uint		bits;

#define P4_BIT_IMM			0x0001
#define P4_BIT_CREATED			0x0002
#define P4_BIT_HIDDEN			0x0004
#define P4_BIT_COMPILE			0x0008

#define P4_WORD_IS(w, bit)		(((w)->bits & (bit)) == (bit))
#define P4_WORD_SET(w, bit)		((w)->bits |= (bit))
#define P4_WORD_CLEAR(w, bit)		((w)->bits &= ~(bit))

#define P4_WORD_IS_IMM(w)		P4_WORD_IS(w, P4_BIT_IMM)
#define P4_WORD_SET_IMM(w)		P4_WORD_SET(w, P4_BIT_IMM)
#define P4_WORD_CLEAR_IMM(w)		P4_WORD_CLEAR(w, P4_BIT_IMM)

#define P4_WORD_IS_HIDDEN(w)		P4_WORD_IS(w, P4_BIT_HIDDEN)
#define P4_WORD_SET_HIDDEN(w)		P4_WORD_SET(w, P4_BIT_HIDDEN)
#define P4_WORD_CLEAR_HIDDEN(w)		P4_WORD_CLEAR(w,P4_BIT_HIDDEN)

#define P4_WORD_IS_COMPILE(w)		P4_WORD_IS(w, P4_BIT_COMPILE)
#define P4_WORD_SET_COMPILE(w)		P4_WORD_SET(w, P4_BIT_COMPILE)
#define P4_WORD_CLEAR_COMPILE(w)	P4_WORD_CLEAR(w, P4_BIT_COMPILE)

	P4_Uint		poppush;

#define P4_WD_LIT(w)			(((w)->poppush >> 24) & 0x0F)
#define P4_WD_LIT_SET(w, u)		(((w)->poppush | (((u)& 0x0F) << 24)
#define P4_FS_CAN_POP(w)		(((w)->poppush >> 20) & 0x0F)
#define P4_FS_CAN_PUSH(w)		(((w)->poppush >> 16) & 0x0F)
#define P4_RS_CAN_POP(w)		(((w)->poppush >> 12) & 0x0F)
#define P4_RS_CAN_PUSH(w)		(((w)->poppush >>  8) & 0x0F)
#define P4_DS_CAN_POP(w)		(((w)->poppush >>  4) & 0x0F)
#define P4_DS_CAN_PUSH(w)		(((w)->poppush      ) & 0x0F)

	/* Body */
	P4_Code		code;		/* Code field points of primative. */
	P4_Size		ndata;		/* Size of data[] in bytes. */
	P4_Cell *	data;		/* Word grows by data cells. */
};

#define P4_WORD(name, code, bits, pp)	{ NULL, STRLEN(name), name, bits, pp, code, 0 }
#define P4_FVAL(name, val)		{ NULL, STRLEN(name), name, 0, 0x01, &&_dofloat, (P4_Uint)(P4_Float)(val) }
#define P4_VAL(name, val)		{ NULL, STRLEN(name), name, 0, 0x01, &&_doconst, val }

typedef struct {
	P4_Int		size;		/* Size of table in cells. */
	P4_Cell *	top;		/* Last element in the stack / array. */
	P4_Cell *	base;		/* Base of array; might be reallocated. */
} P4_Array, P4_Stack;

# define P4_PICK(stack, offset)		((stack).top[-(offset)])
# define P4_INDEX(stack, index)		((stack).base[index])
# define P4_TOP(stack)			(*(stack).top)
# define P4_DROPTOP(stack)		(*--(stack).top)
# define P4_POP(stack)			(*(stack).top--)
# define P4_PUSH(stack, x)		(*++(stack).top = (P4_Cell)(x))
# define P4_LENGTH(stack)		((ptrdiff_t)((stack).top + 1 - (stack).base))
# define P4_DROP(stack, n)		((stack).top -= (n))
# define P4_SET(stack, n)		((stack).top = (stack).base + (n) - 1)
# define P4_RESET(stack)		P4_SET(stack, 0)
# define P4_GUARD_CELLS			4

# define P4_PLENGTH(stk)		((ptrdiff_t)((stk)->top + 1 - (stk)->base))
# define P4_PSET(stk, n)		((stk)->top = (stk)->base + (n) - 1)
# define P4_PRESET(stk)			P4_PSET(stk, 0)


typedef enum {
	P4_STATE_COMPILE = (-1),	/* Match Forth value for TRUE. */
	P4_STATE_INTERPRET,
} P4_State;

struct p4_ctx {
	P4_Char *	end;		/* End of data space memory. */
	P4_Char *	here;		/* Next unused data space. */
	P4_Int		state;
	P4_Cell *	frame;		/* See CATCH and THROW. */
	P4_Int          trace;          /* Word trace for debugging. */
	P4_Int		level;		/* Tracing depth. */
	P4_Uint		radix;		/* Input/Output radix */
	P4_Stack	ds;		/* Data stack */
	P4_Stack	rs;		/* Return stack */
	/* Leave these in place even if float support is disabled. */
	P4_Stack	fs;		/* Float stack */
	P4_Int		precision;
	/* ... */
	P4_Int		unkey;		/* KEY and KEY? */
	P4_Input *	input;
	P4_Block *	block;
	void *		block_fd;
	P4_Word **	active;		/* Active compiliation word list. */
	P4_Word *	locals;		/* locals = lists[-1] */
	P4_Word *	lists[P4_WORDLISTS];
	P4_Uint		norder;		/* Order length, [0, P4_WORDLISTS) */
	P4_Uint		order[P4_WORDLISTS];
	P4_Options *	options;
	/* Leave this in place even if JNI support is disabled. */
	void *		jenv;
	/* ... */
	JMP_BUF		longjmp;	/* Must be last in struct; size can
					 * vary by CPU and implementation.
					 */
};

typedef struct {
	size_t length;
	P4_Uint	poppush;
	const char *name;
	void (*func)(P4_Ctx *);
} P4_Hook;

#define P4_FLOAT_STACK	fs

/***********************************************************************
 *** Exit Status
 ***********************************************************************/

#define P4_EXIT_OK		(0)
#define P4_EXIT_FAIL		(1)
#define P4_EXIT_USAGE		(2)
#define P4_EXIT_EXCEPTION	(3)
#define P4_EXIT_STATUS(ex)	((ex != P4_EXIT_OK) * P4_EXIT_EXCEPTION)
#define P4_EXIT_SIGNAL(sig)	(128+(sig))

/***********************************************************************
 *** Exceptions
 ***********************************************************************/

#define P4_THROW_OK		( 0)

/* -255..-1 reserved by the standard. */

#define P4_THROW_ABORT		(-1)	/* ABORT */
#define P4_THROW_ABORT_MSG	(-2)	/* ABORT" */
#define P4_THROW_DS_OVER	(-3)	/* stack overflow */
#define P4_THROW_DS_UNDER	(-4)	/* stack underflow */
#define P4_THROW_RS_OVER	(-5)	/* return stack overflow */
#define P4_THROW_RS_UNDER	(-6)	/* return stack underflow */
#define P4_THROW_LOOP_DEPTH	(-7)	/* do-loops nested too deeply during execution */
#define P4_THROW_DICT_OVER	(-8)	/* dictionary overflow */
#define P4_THROW_SIGSEGV	(-9)	/* invalid memory address */
#define P4_THROW_DIV_ZERO	(-10)	/* division by zero */
#define P4_THROW_ERANGE		(-11)	/* result out of range */
#define P4_THROW_EINVAL		(-12)	/* argument type mismatch */
#define P4_THROW_UNDEFINED	(-13)	/* undefined word */
#define P4_THROW_COMPILE_ONLY	(-14)	/* interpreting a compile-only word */
#define P4_THROW__15		(-15)	/* invalid FORGET */
#define P4_THROW_EMPTY_NAME	(-16)	/* attempt to use zero-length string as a name */
#define P4_THROW_PIC_OVER	(-17)	/* pictured numeric output string overflow */
#define P4_THROW_PARSE_OVER	(-18)	/* parsed string overflow */
#define P4_THROW_NAME_TOO_LONG	(-19)	/* definition name too long */
#define P4_THROW__20		(-20)	/* write to a read-only location */
#define P4_THROW_UNSUPPORTED	(-21)	/* unsupported operation (e.g., AT-XY on a too-dumb terminal) */
#define P4_THROW_BAD_CONTROL	(-22)	/* control structure mismatch */
#define P4_THROW_SIGBUS		(-23)	/* address alignment exception */
#define P4_THROW_BAD_NUMBER	(-24)	/* invalid numeric argument */
#define P4_THROW_RS_IMBALANCE	(-25)	/* return stack imbalance */
#define P4_THROW__26		(-26)	/* loop parameters unavailable */
#define P4_THROW__27		(-27)	/* invalid recursion */
#define P4_THROW_SIGINT		(-28)	/* user interrupt */
#define P4_THROW_COMPILING	(-29)	/* compiler nesting */
#define P4_THROW__30		(-30)	/* obsolescent feature */
#define P4_THROW_NOT_CREATED	(-31)	/* word not defined by CREATE */
#define P4_THROW_BAD_NAME	(-32)	/* invalid name argument (e.g., TO xxx) */
#define P4_THROW_BLOCK_RD	(-33)	/* block read exception */
#define P4_THROW_BLOCK_WR	(-34)	/* block write exception */
#define P4_THROW_BLOCK_BAD	(-35)	/* invalid block number */
#define P4_THROW__36		(-36)	/* invalid file position */
#define P4_THROW_EIO		(-37)	/* file I/O exception */
#define P4_THROW_ENOENT		(-38)	/* non-existent file */
#define P4_THROW_BAD_EOF	(-39)	/* unexpected end of file */
#define P4_THROW_BAD_BASE	(-40)	/* invalid BASE for floating point conversion */
#define P4_THROW__41		(-41)	/* loss of precision */
#define P4_THROW__42		(-42)	/* floating-point divide by zero */
#define P4_THROW__43		(-43)	/* floating-point result out of range */
#define P4_THROW_FS_OVER	(-44)	/* floating-point stack overflow */
#define P4_THROW_FS_UNDER	(-45)	/* floating-point stack underflow */
#define P4_THROW__46		(-46)	/* floating-point invalid argument */
#define P4_THROW__47		(-47)	/* compilation word list deleted */
#define P4_THROW__48		(-48)	/* invalid POSTPONE */
#define P4_THROW__49		(-49)	/* search-order overflow */
#define P4_THROW__50		(-50)	/* search-order underflow */
#define P4_THROW__51		(-51)	/* compilation word list changed */
#define P4_THROW__52		(-52)	/* control-flow stack overflow */
#define P4_THROW__53		(-53)	/* exception stack overflow */
#define P4_THROW__54		(-54)	/* floating-point underflow */
#define P4_THROW_SIGFPE		(-55)	/* floating-point unidentified fault */
#define P4_THROW_QUIT		(-56)	/* QUIT */
#define P4_THROW__57		(-57)	/* exception in sending or receiving a character */
#define P4_THROW__58		(-58)	/* [IF], [ELSE], or [THEN] exception */
#define P4_THROW_ALLOCATE	(-59)	/* ALLOCATE */
#define P4_THROW__60		(-60)	/* FREE */
#define P4_THROW_RESIZE		(-61)	/* RESIZE */
#define P4_THROW__62		(-62)	/* CLOSE-FILE */
#define P4_THROW__63		(-63)	/* CREATE-FILE */
#define P4_THROW__64		(-64)	/* DELETE-FILE */
#define P4_THROW__65		(-65)	/* FILE-POSITION */
#define P4_THROW__66		(-66)	/* FILE-SIZE */
#define P4_THROW__67		(-67)	/* FILE-STATUS */
#define P4_THROW__68		(-68)	/* FLUSH-FILE */
#define P4_THROW_OPEN		(-69)	/* OPEN-FILE */
#define P4_THROW__70		(-70)	/* READ-FILE */
#define P4_THROW__71		(-71)	/* READ-LINE */
#define P4_THROW__72		(-72)	/* RENAME-FILE */
#define P4_THROW__73		(-73)	/* REPOSITION-FILE */
#define P4_THROW__74		(-74)	/* RESIZE-FILE */
#define P4_THROW__75		(-75)	/* WRITE-FILE */
#define P4_THROW__76		(-76)	/* WRITE-LINE */
#define P4_THROW__77		(-77)	/* Malformed xchar */
#define P4_THROW__78		(-78)	/* SUBSTITUTE */
#define P4_THROW__79		(-79)	/* REPLACES */
#define P4_THROW_future		(-80)	/* -80 .. -255 reserved for future assignment */

/* -4095..-256 reserved for the system (that's us). */

#define P4_THROW_SIGTERM	(-256)
#define P4_THROW_WORDLIST	(-257)	/* Out of word list space; invalid wid. */
#define P4_THROW_GENERIC	(-4095)

/***********************************************************************
 *** API
 ***********************************************************************/

extern const char newline[];

extern const char p4_commit[];

/**
 * Initialise the global environment.
 */
extern void p4Init(P4_Options *opts);

/**
 * Create a new interpreter context.
 *
 * @param opts
 *	Pointer to P4_Options.
 *
 * @return
 *	A pointer to an allocated P4_Ctx structure.
 */
extern P4_Ctx *p4Create(P4_Options *opts);

/**
 * @param ctx
 *	A pointer to an allocated P4_Ctx structure to free.
 */
extern void p4Free(P4_Ctx *ctx);

/**
 * @param ctx
 *	A pointer to an allocated P4_Ctx structure.
 *
 * @return
 *	Zero on success, otherwise an exception code.
 */
extern int p4Repl(P4_Ctx *ctx, int code);

/**
 * @param ctx
 *	A pointer to an allocated P4_Ctx structure.
 *
 * @param fp
 *	An open FILE * for reading.
 *
 * @return
 *	Zero on success, otherwise an exception code other than BYE.
 */
extern int p4EvalFp(P4_Ctx *ctx, FILE *fp);

/**
 * @param ctx
 *	A pointer to an allocated P4_Ctx structure.
 *
 * @param filepath
 *	A C string of a file path name to interpret.
 *	If NULL, then standard input will be read.
 *
 * @return
 *	Zero on success, otherwise an exception code other than BYE.
 */
extern int p4EvalFile(P4_Ctx *ctx, const char *file);
extern int p4EvalFilePath(P4_Ctx *ctx, const char *file);

/**
 * @param ctx
 *	A pointer to an allocated P4_Ctx structure.
 *
 * @param string
 *	A string to interpret, possible not NUL terminated.
 *
 * @param length
 *	Length of string.
 *
 * @return
 *	Zero on success, otherwise an exception code other than BYE.
 */
extern int p4EvalString(P4_Ctx *ctx, const char *string, size_t length);

extern void p4AllocStack(P4_Ctx *ctx, P4_Stack *stk, unsigned size);

extern const char *p4_exceptions[];
extern JMP_BUF sig_break_glass;
extern void sig_init(void);
extern void sig_fini(void);

#ifdef HAVE_HOOKS
extern P4_Hook p4_hooks[];
extern P4_Word *p4_hook_call;
extern void p4HookInit(P4_Ctx *ctx, P4_Hook *hooks);
extern P4_Word *p4HookAdd(P4_Ctx *ctx, P4_Hook *hook);
# define P4_HOOK(pp, name, func)	{ STRLEN(name), pp, name, func }
#else
# define p4HookAdd(ctx, hook)		(NULL)
# define p4HookInit(ctx, hooks)
#endif

/***********************************************************************
 *** Utility Functions
 ***********************************************************************/

extern void p4ResetInput(P4_Ctx *ctx, FILE *fp);


/**
 * @param ch
 *	A backslash escaped character.
 *
 * @return
 *	The value that the backslash escape character represents.
 *
 *	\a	bell
 *	\b	backspace
 *	\e	escape
 *	\f	formfeed
 *	\n	linefeed
 *	\r	carriage-return
 *	\s	space
 *	\t	tab
 *	\v	vertical tab
 *	\?	delete
 *
 *	Other characters remain as themselves.
 */
extern int p4CharLiteral(int ch);

/**
 * @param s
 *	A pointer to a C string to reverse in place.
 *
 * @param length
 *	The length of the C string.
 */
extern void p4StrRev(P4_Char *s, P4_Size length);

extern int p4StrNum(P4_String str, unsigned base, P4_Cell out[2], int *is_float, int *is_double);

extern int p4Accept(P4_Input *source, char *buffer, size_t size);

/**
 * Handles parsing of "ccc<char>".
 *
 * @param input
 *	A pointer to an already filled input buffer.
 *
 * @param delim
 *	Input is read until this delimiter is seen. If delimiter
 *	is space ( ), then treat all white space as a delimiter.
 *
 * @param escape
 *	When true, allow for backslash escaped characters.
 *
 * @return
 *	A P4_String structure containing a pointer within the
 *	input buffer and the length of the string upto the delim
 *	character.
 *
 * @note
 *	Backslash escaped characters are converted to a literal and
 *	the input buffer reduced in size.
 *
 * @standard ANS-Forth 1994, extended
 */
extern P4_String p4Parse(P4_Input *input, int delim, int escape);

/**
 * Handles parsing of "<space>ccc<space>", skipping leading occurences of <char>.
 *
 * @param input
 *	A pointer to an already filled input buffer.
 *
 * @return
 *	The length of the parsed input; otherwise 0 on EOF or error.
 */
extern P4_String p4ParseName(P4_Input *input);

extern P4_Word *p4FindName(P4_Ctx *ctx, const char *caddr, P4_Size length);

extern P4_Word *p4WordCreate(P4_Ctx *ctx, const char *name, size_t length, P4_Code code);

extern void p4WordAppend(P4_Ctx *ctx, P4_Cell data);

/***********************************************************************
 *** END
 ***********************************************************************/

#ifdef  __cplusplus
}
#endif

#endif /* __post4_h__ */
