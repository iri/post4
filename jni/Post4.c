/*
 * Post4.c
 *
 * Copyright 2023, 2024 by Anthony Howe.  All rights reserved.
 */

#include <stdlib.h>
#include <stdint.h>

#ifdef __GNUC__
typedef int64_t __int64;
#endif

#include <jni.h>

#include "../src/post4.h"

#define POST4_CLASS	"post4/jni/Post4"
#define ERROR_CLASS	"post4/jni/Post4Exception"

static const char *empty_argv[] = { NULL };

static P4_Ctx *
getCtx(JNIEnv *env, jobject self)
{
	jclass post4 = (*env)->GetObjectClass(env, self);
	jfieldID fid = (*env)->GetFieldID(env, post4, "ctx", "J");
	jlong ctx = (*env)->GetLongField(env, self, fid);
	(*env)->DeleteLocalRef(env, post4);
	return (P4_Ctx *) ctx;
}

static int
isClassClass(JNIEnv *env, jobject obj)
{
	jclass cls = (*env)->FindClass(env, "java/lang/Class");
	int is_class = (*env)->IsInstanceOf(env, obj, cls);
	(*env)->DeleteLocalRef(env, cls);
	return is_class;
}

#ifdef NDEBUG
# define getClassName(e, o)	(NULL)
# define prints(e, s)
#else
static jstring
getClassName(JNIEnv *env, jobject obj)
{
	jclass cls = (*env)->FindClass(env, "java/lang/Class");
	jmethodID mid = (*env)->GetMethodID(env, cls, "getName", "()Ljava/lang/String;");
	jstring name = (*env)->CallObjectMethod(env, obj, mid);
	(*env)->DeleteLocalRef(env, cls);
	return name;
}

static void
prints(JNIEnv *env, jstring jstr)
{
	const char *str = (*env)->GetStringUTFChars(env, jstr, NULL);
	jsize size = (*env)->GetStringUTFLength(env, jstr);
	(void) fprintf(stderr, "%#lx %d \"%.*s\"\n", jstr, size, size, str);
	(*env)->ReleaseStringUTFChars(env, jstr, str);
}
#endif

static jobject
post4Exception(JNIEnv *env, int code)
{
	jclass oops = (*env)->FindClass(env, ERROR_CLASS);
	jmethodID mid = (*env)->GetMethodID(env, oops, "<init>", "(I)V");
	jobject p4err = (*env)->NewObject(env, oops, mid, code);
	(*env)->DeleteLocalRef(env, oops);
	return p4err;
}

JNIEXPORT void JNICALL
Java_post4_jni_Post4_p4Init(JNIEnv *env, jobject self)
{
	p4Init();
}

JNIEXPORT void JNICALL
Java_post4_jni_Post4_p4Free(JNIEnv *env, jobject self, jlong xtc)
{
	P4_Ctx *ctx = (P4_Ctx *) xtc;
	if (ctx->argv != (char **) empty_argv) {
		for (int argi = 0; argi < ctx->argc; argi++) {
			free(ctx->argv[argi]);
		}
		free(ctx->argv);
	}
	p4Free(ctx);
}

static jobject
getStacks(JNIEnv *env, P4_Ctx *ctx)
{
	jsize size;

	size = P4_LENGTH(ctx->ds);
	jlongArray longs = (*env)->NewLongArray(env, size);
	jlong tmp_long[size];
	for (int i = 0; i < size; i++) {
		tmp_long[i] = P4_PICK(ctx->ds, i).n;
	}
	(*env)->SetLongArrayRegion(env, longs, 0, size, tmp_long);

	size = P4_LENGTH(ctx->fs);
	jdoubleArray doubles = (*env)->NewDoubleArray(env, size);
	jdouble tmp_double[size];
	for (int i = 0; i < size; i++) {
		tmp_double[i] = P4_PICK(ctx->fs, i).f;
	}
	(*env)->SetDoubleArrayRegion(env, doubles, 0, size, tmp_double);

	jclass stacks = (*env)->FindClass(env, "post4/jni/Post4Stacks");
	jmethodID mid = (*env)->GetMethodID(env, stacks, "<init>", "([J[D)V");
	jobject results = (*env)->NewObject(env, stacks, mid, longs, doubles);
	(*env)->DeleteLocalRef(env, stacks);

	return results;
}

JNIEXPORT jint JNICALL
Java_post4_jni_Post4_repl(JNIEnv *env, jobject self)
{
	P4_Ctx *ctx = getCtx(env, self);
	ctx->jenv = env;
	return p4Repl(ctx, P4_THROW_OK);
}

JNIEXPORT jobject JNICALL
Java_post4_jni_Post4_stacks(JNIEnv *env, jobject self)
{
	return getStacks(env, getCtx(env, self));
}

JNIEXPORT void JNICALL
Java_post4_jni_Post4_evalFile(JNIEnv *env, jobject self, jstring fpath)
{
	P4_Ctx *ctx = getCtx(env, self);
	ctx->jenv = env;
	const char *path = (*env)->GetStringUTFChars(env, fpath, NULL);
	int rc = p4EvalFile(ctx, path);
	(*env)->ReleaseStringUTFChars(env, fpath, path);
	if (rc != P4_THROW_OK) {
		(*env)->Throw(env, post4Exception(env, rc));
	}
}

JNIEXPORT void JNICALL
Java_post4_jni_Post4_evalString(JNIEnv *env, jobject self, jstring string)
{
	P4_Ctx *ctx = getCtx(env, self);
	ctx->jenv = env;
	size_t len = (*env)->GetStringLength(env, string);
	const char *str = (*env)->GetStringUTFChars(env, string, NULL);
	int rc = p4EvalString(ctx, str, len);
	(*env)->ReleaseStringUTFChars(env, string, str);
	if (rc != P4_THROW_OK) {
		(*env)->Throw(env, post4Exception(env, rc));
	}
}

#ifdef HAVE_HOOKS
# define BEGIN		{
# define END		}

/*
 * Return the arity given a function signature.
 */
static int
post4Arity(const char *sig)
{
	int arity = 0;

	if (*sig++ != '(') {
		return -1;
	}
	for ( ; *sig != ')'; sig++) {
		switch (*sig) {
		case 'Z': case 'B': case 'C':  case 'S': case 'I': case 'J': case 'D':
			break;
		case 'L':
			sig += strcspn(sig, ";");
			break;
		case '[':
			switch (*++sig) {
			case 'Z': case 'B': case 'C':  case 'S': case 'I': case 'J': case 'D':
				break;
			case 'L':
				sig += strcspn(sig, ";");
				break;
			default:
				return -1;
			}
			break;
		default:
			return -1;
		}
		arity++;
	}

	return arity;
}

/*
 * jSetLocalCapacity ( n -- )
 *
 *	Before it enters a native method, the VM automatically ensures
 *	that at least 16 local references can be created.
 */
static void
jSetLocalCapacity(P4_Ctx *ctx)
{
	JNIEnv *env = ctx->jenv;
	jint n = P4_POP(ctx->ds).n;
	(*env)->EnsureLocalCapacity(env, n);
}

/*
 * jDeleteLocalRef ( obj -- )
 */
static void
jDeleteLocalRef(P4_Ctx *ctx)
{
	JNIEnv *env = ctx->jenv;
	jobject obj = P4_POP(ctx->ds).v;
	(*env)->DeleteLocalRef(env, obj);
}

/*
 * jFindClass ( class_name u -- cls )
 */
static void
jFindClass(P4_Ctx *ctx)
{
	JNIEnv *env = ctx->jenv;
	P4_DROP(ctx->ds, 1);
	const char *str = P4_TOP(ctx->ds).s;
	jclass cls = (*env)->FindClass(env, str);
	P4_TOP(ctx->ds).v = cls;
}

/*
 * jArrayLength ( jarray -- length )
 */
static void
jArrayLength(P4_Ctx *ctx)
{
	JNIEnv *env = ctx->jenv;
	jarray arr = (jarray) P4_TOP(ctx->ds).v;
	jsize size = (*env)->GetArrayLength(env, arr);
	P4_TOP(ctx->ds).n = size;
}

/*
 * jBoxArray ( x*i i -- jarray )
 */
static void
jBoxArray(P4_Ctx *ctx)
{
	JNIEnv *env = ctx->jenv;
	jsize size = (jsize) P4_POP(ctx->ds).z;
	jlongArray arr = (*env)->NewLongArray(env, size);
	if (arr == NULL) {
		LONGJMP(ctx->longjmp, P4_THROW_ALLOCATE);
	}
	for (jsize i = 0; i < size; i++) {
		jlong item = (jlong) P4_POP(ctx->ds).n;
		(*env)->SetLongArrayRegion(env, arr, i, 1, &item);

	}
	P4_PUSH(ctx->ds, (void *) arr);
}

static int
p4StackResize(P4_Stack *stk, unsigned newsize)
{
	if (newsize <= stk->size) {
		/* Nothing but growth potential. */
		return 0;
	}
	size_t depth = P4_PLENGTH(stk);
	P4_Cell *newbase = realloc(stk->base - P4_GUARD_CELLS/2, (newsize + P4_GUARD_CELLS) * sizeof (*stk->base));
	if (newbase == NULL) {
		return -1;
	}
	/* Set stack just above lower guards. */
	stk->base = newbase + P4_GUARD_CELLS/2;
	/* Clear new memory to make Valgrind happy. */
	(void) memset(stk->base + depth, 0, (newsize - stk->size) * sizeof (*stk->base));
	/* Set the guards. */
	stk->base[newsize].u = P4_SENTINEL;
	stk->base[-1].u = P4_SENTINEL;
	stk->base[newsize+1].u = 0;
	stk->base[-2].u = 0;
	stk->size = newsize;
	P4_PSET(stk, depth);
	return 0;
}

/*
 * jUnboxArray ( jarray -- x*i i )
 */
static void
jUnboxArray(P4_Ctx *ctx)
{
	JNIEnv *env = ctx->jenv;
	size_t ds_depth = P4_LENGTH(ctx->ds);
	jarray arr = (jarray) P4_POP(ctx->ds).v;
	jsize size = (*env)->GetArrayLength(env, arr);
	/* Enough stack space to hold array items? */
	if (ctx->ds.size <= ds_depth + size + 1) {
		/* Grow data stack with some extra work space. */
		if (p4StackResize(&ctx->ds, ctx->ds.size + size + P4_STACK_EXTRA)) {
			/* Frick'n'ell. */
			LONGJMP(ctx->longjmp, P4_THROW_DS_OVER);
		}
	}
	/* Get each array item and push. */
	for (jsize i = size - 1; 0 <= i; i--) {
		jlong item;
		(*env)->GetLongArrayRegion(env, arr, i, 1, &item);
		P4_PUSH(ctx->ds, (P4_Int) item);
	}
	P4_PUSH(ctx->ds, (P4_Size) size);
}

/*
 * jStringByteLength ( jstr -- length )
 */
static void
jStringByteLength(P4_Ctx *ctx)
{
	JNIEnv *env = ctx->jenv;
	jstring str = (jstring) P4_TOP(ctx->ds).v;
	jsize size = (*env)->GetStringUTFLength(env, str);
	P4_TOP(ctx->ds).n = size;
}

/*
 * jBoxString ( caddr u -- jstr )
 */
static void
jBoxString(P4_Ctx *ctx)
{
	JNIEnv *env = ctx->jenv;
	P4_DROP(ctx->ds, 1);
	const char *str = P4_TOP(ctx->ds).v;
	jstring jstr = (*env)->NewStringUTF(env, str);
	if (jstr == NULL) {
		(*env)->ExceptionDescribe(env);
		LONGJMP(ctx->longjmp, P4_THROW_EINVAL);
	}
	P4_TOP(ctx->ds).v = jstr;
}

/*
 * jUnboxString ( jstr -- caddr u )
 */
static void
jUnboxString(P4_Ctx *ctx)
{
	JNIEnv *env = ctx->jenv;
	jstring jstr = P4_TOP(ctx->ds).v;
	jsize size = (*env)->GetStringUTFLength(env, jstr);
	const char *str = (*env)->GetStringUTFChars(env, jstr, NULL);
	P4_TOP(ctx->ds).s = strdup(str);
	P4_PUSH(ctx->ds, (P4_Size) size);
	(*env)->ReleaseStringUTFChars(env, jstr, str);
}

/*
 * jPushLocalFrame ( capacity -- )
 */
static void
jPushLocalFrame(P4_Ctx *ctx)
{
	JNIEnv *env = ctx->jenv;
	jint capacity = (jint) P4_POP(ctx->ds).n;
	jint rc = (*env)->PushLocalFrame(env, capacity);
	if (rc != JNI_OK) {
		(*env)->ExceptionDescribe(env);
		LONGJMP(ctx->longjmp, P4_THROW_ALLOCATE);
	}
}

/*
 * jPopLocalFrame ( -- )
 */
static void
jPopLocalFrame(P4_Ctx *ctx)
{
	JNIEnv *env = ctx->jenv;
	(void) (*env)->PopLocalFrame(env, NULL);
}

#ifdef HMM
/*
 * jObjectClass ( obj -- cls )
 */
static void
jObjectClass(P4_Ctx *ctx)
{
	JNIEnv *env = ctx->jenv;
	jobject obj = P4_TOP(ctx->ds).v;
	jclass cls = (*env)->GetObjectClass(env, obj);
	if (cls == NULL) {
		(*env)->ExceptionDescribe(env);
		LONGJMP(ctx->longjmp, P4_THROW_EINVAL);
	}
	P4_TOP(ctx->ds).v = cls;
}

/*
 * jMethodID ( cls method u signature u -- mid is_static )
 */
static void
jMethodID(P4_Ctx *ctx)
{
	JNIEnv *env = ctx->jenv;

	P4_DROP(ctx->ds, 1);
	const char *sig = P4_POP(ctx->ds).s;
	P4_DROP(ctx->ds, 1);
	const char *method = P4_POP(ctx->ds).s;
	jclass cls = P4_POP(ctx->ds).v;

	int is_static = 0;
	jmethodID mid = (*env)->GetMethodID(env, cls, method, sig);
	if (mid == NULL) {
		(*env)->ExceptionClear(env);
		mid = (*env)->GetStaticMethodID(env, cls, method, sig);
		if (mid == NULL) {
			(*env)->ExceptionDescribe(env);
			LONGJMP(ctx->longjmp, P4_THROW_EINVAL);
		}
		is_static = 1;
	}

	P4_PUSH(ctx->ds, (P4_Uint) mid);
	P4_PUSH(ctx->ds, (P4_Int) is_static);
}

/*
 * jFieldID ( cls field f signature s -- fid is_static )
 */
static void
jFieldID(P4_Ctx *ctx)
{
	JNIEnv *env = ctx->jenv;

	P4_DROP(ctx->ds, 1);
	const char *sig = P4_POP(ctx->ds).s;
	P4_DROP(ctx->ds, 1);
	const char *field = P4_POP(ctx->ds).s;
	jclass cls = P4_POP(ctx->ds).v;

	int is_static = 0;
	jfieldID fid = (*env)->GetFieldID(env, cls, field, sig);
	if (fid == NULL) {
		(*env)->ExceptionClear(env);
		fid = (*env)->GetStaticFieldID(env, cls, field, sig);
		if (fid == NULL) {
			(*env)->ExceptionDescribe(env);
			LONGJMP(ctx->longjmp, P4_THROW_EINVAL);
		}
		is_static = 1;
	}

	P4_PUSH(ctx->ds, (P4_Uint) fid);
	P4_PUSH(ctx->ds, (P4_Int) is_static);
}
#endif

/*
 * jCall ( x*i cls_or_obj method m signature s -- x | )
 */
static void
jCall(P4_Ctx *ctx)
{
	int is_static = 0;
	JNIEnv *env = ctx->jenv;

	size_t len = P4_POP(ctx->ds).z;
	const char *sig = P4_POP(ctx->ds).s;
	P4_DROP(ctx->ds, 1);
	const char *method = P4_POP(ctx->ds).s;
	jobject cls_or_obj = P4_POP(ctx->ds).v;
	if (cls_or_obj == NULL) {
		goto error0;
	}

	// Assert we have a class.
	jclass cls;
	if (isClassClass(env, cls_or_obj)) {
		// Already a class.
		cls = (*env)->NewLocalRef(env, cls_or_obj);
	} else {
		// Class of instance.
		cls = (*env)->GetObjectClass(env, cls_or_obj);
	}

	jmethodID mid = (*env)->GetMethodID(env, cls, method, sig);
	if (mid == NULL) {
		(*env)->ExceptionClear(env);
		mid = (*env)->GetStaticMethodID(env, cls, method, sig);
		if (mid == NULL) {
			(*env)->ExceptionDescribe(env);
			goto error1;
		}
		is_static = 1;
	}

	BEGIN;
	int arity = post4Arity(sig);
	if (arity < 0) {
		goto error1;
	}

	jvalue jargs[arity];
	for (int i = 0; i < arity; i++) {
		jargs[i].j = P4_PICK(ctx->ds, i).n;
	}
	P4_DROP(ctx->ds, arity);

	P4_Cell ret;
	if (is_static) {
		// Static methods come from a class.
		switch (sig[len-1]) {
		case 'V':
			(*env)->CallStaticVoidMethodA(env, cls, mid, jargs);
			break;
		case 'Z':
			ret.n = (P4_Int)(*env)->CallStaticBooleanMethodA(env, cls, mid, jargs);
			break;
		case 'B':
			ret.u = (P4_Uint)(*env)->CallStaticByteMethodA(env, cls, mid, jargs);
			break;
		case 'C':
			ret.u = (P4_Uint)(*env)->CallStaticCharMethodA(env, cls, mid, jargs);
			break;
		case 'S':
			ret.n = (P4_Int)(*env)->CallStaticShortMethodA(env, cls, mid, jargs);
			break;
		case 'I':
			ret.n = (P4_Int)(*env)->CallStaticIntMethodA(env, cls, mid, jargs);
			break;
		case 'J':
			ret.n = (P4_Int)(*env)->CallStaticLongMethodA(env, cls, mid, jargs);
			break;
		case 'D':
			ret.f = (P4_Float)(*env)->CallStaticDoubleMethodA(env, cls, mid, jargs);
			break;
		case ';': // (...)Lsome/class/path;
			ret.v = (void *)(*env)->CallStaticObjectMethodA(env, cls, mid, jargs);
			break;
		default:
			goto error1;
		}
	} else {
		// Instance methods come from an object.
		switch (sig[len-1]) {
		case 'V':
			(*env)->CallVoidMethodA(env, cls_or_obj, mid, jargs);
			break;
		case 'Z':
			ret.n = (P4_Int)(*env)->CallBooleanMethodA(env, cls_or_obj, mid, jargs);
			break;
		case 'B':
			ret.u = (P4_Uint)(*env)->CallByteMethodA(env, cls_or_obj, mid, jargs);
			break;
		case 'C':
			ret.u = (P4_Uint)(*env)->CallCharMethodA(env, cls_or_obj, mid, jargs);
			break;
		case 'S':
			ret.n = (P4_Int)(*env)->CallShortMethodA(env, cls_or_obj, mid, jargs);
			break;
		case 'I':
			ret.n = (P4_Int)(*env)->CallIntMethodA(env, cls_or_obj, mid, jargs);
			break;
		case 'J':
			ret.n = (P4_Int)(*env)->CallLongMethodA(env, cls_or_obj, mid, jargs);
			break;
		case 'D':
			ret.f = (P4_Float)(*env)->CallDoubleMethodA(env, cls_or_obj, mid, jargs);
			break;
		case ';': // (...)Lsome/class/path;
			ret.v = (void *)(*env)->CallObjectMethodA(env, cls_or_obj, mid, jargs);
			break;
		default:
			goto error1;
		}
	}
	(*env)->DeleteLocalRef(env, cls);
	if (sig[len-1] != 'V') {
		P4_PUSH(ctx->ds, ret);
	}
	return;
	END;
error1:
	(*env)->DeleteLocalRef(env, cls);
error0:
	LONGJMP(ctx->longjmp, P4_THROW_EINVAL);
}

/*
 * jField ( cls_or_obj field f signature s -- x )
 */
static void
jField(P4_Ctx *ctx)
{
	int is_static = 0;
	JNIEnv *env = ctx->jenv;

	size_t len = P4_POP(ctx->ds).z;
	char *sig = P4_POP(ctx->ds).s;
	P4_DROP(ctx->ds, 1);
	char *field = P4_POP(ctx->ds).s;
	jobject cls_or_obj = P4_POP(ctx->ds).v;
	if (cls_or_obj == NULL) {
		goto error0;
	}

	// Assert we have a class.
	jclass cls;
	if (isClassClass(env, cls_or_obj)) {
		// Already a class.
		cls = (*env)->NewLocalRef(env, cls_or_obj);
	} else {
		// Class of instance.
		cls = (*env)->GetObjectClass(env, cls_or_obj);
	}

	jfieldID fid = (*env)->GetFieldID(env, cls, field, sig);
	if (fid == NULL) {
		(*env)->ExceptionClear(env);
		fid = (*env)->GetStaticFieldID(env, cls, field, sig);
		if (fid == NULL) {
			(*env)->ExceptionDescribe(env);
			goto error1;
		}
		is_static = 1;
	}

	P4_Cell ret;
	if (is_static) {
		// Static fields come from a class.
		switch (*sig) {
		case 'Z':
			ret.n = (P4_Int)(*env)->GetStaticBooleanField(env, cls, fid);
			break;
		case 'B':
			ret.u = (P4_Uint)(*env)->GetStaticByteField(env, cls, fid);
			break;
		case 'C':
			ret.u = (P4_Uint)(*env)->GetStaticCharField(env, cls, fid);
			break;
		case 'S':
			ret.n = (P4_Int)(*env)->GetStaticShortField(env, cls, fid);
			break;
		case 'I':
			ret.n = (P4_Int)(*env)->GetStaticIntField(env, cls, fid);
			break;
		case 'J':
			ret.n = (P4_Int)(*env)->GetStaticLongField(env, cls, fid);
			break;
		case 'D':
			ret.f = (P4_Float)(*env)->GetStaticDoubleField(env, cls, fid);
			break;
		case 'L':
			ret.v = (void *)(*env)->GetStaticObjectField(env, cls, fid);
			break;
		default:
			goto error1;
		}
	} else {
		// Instance fields come from an object.
		switch (*sig) {
		case 'Z':
			ret.n = (P4_Int)(*env)->GetBooleanField(env, cls_or_obj, fid);
			break;
		case 'B':
			ret.u = (P4_Uint)(*env)->GetByteField(env, cls_or_obj, fid);
			break;
		case 'C':
			ret.u = (P4_Uint)(*env)->GetCharField(env, cls_or_obj, fid);
			break;
		case 'S':
			ret.n = (P4_Int)(*env)->GetShortField(env, cls_or_obj, fid);
			break;
		case 'I':
			ret.n = (P4_Int)(*env)->GetIntField(env, cls_or_obj, fid);
			break;
		case 'J':
			ret.n = (P4_Int)(*env)->GetLongField(env, cls_or_obj, fid);
			break;
		case 'D':
			ret.f = (P4_Float)(*env)->GetDoubleField(env, cls_or_obj, fid);
			break;
		case 'L':
			ret.v = (void *)(*env)->GetObjectField(env, cls_or_obj, fid);
			break;
		default:
			goto error1;
		}
	}
	(*env)->DeleteLocalRef(env, cls);
	P4_PUSH(ctx->ds, ret);
	return;
error1:
	(*env)->DeleteLocalRef(env, cls);
error0:
	LONGJMP(ctx->longjmp, P4_THROW_EINVAL);
}

/*
 * jSetfield ( x cls_or_obj field f signature s -- )
 */
static void
jSetField(P4_Ctx *ctx)
{
	int is_static = 0;
	JNIEnv *env = ctx->jenv;

	size_t len = P4_POP(ctx->ds).z;
	char *sig = P4_POP(ctx->ds).s;
	P4_DROP(ctx->ds, 1);
	char *field = P4_POP(ctx->ds).s;
	jobject cls_or_obj = P4_POP(ctx->ds).v;
	if (cls_or_obj == NULL) {
		goto error0;
	}

	// Assert we have a class.
	jclass cls;
	if (isClassClass(env, cls_or_obj)) {
		// Already a class.
		cls = (*env)->NewLocalRef(env, cls_or_obj);
	} else {
		// Class of instance.
		cls = (*env)->GetObjectClass(env, cls_or_obj);
	}

	jfieldID fid = (*env)->GetFieldID(env, cls, field, sig);
	if (fid == NULL) {
		(*env)->ExceptionClear(env);
		fid = (*env)->GetStaticFieldID(env, cls, field, sig);
		if (fid == NULL) {
			(*env)->ExceptionDescribe(env);
			goto error1;
		}
		is_static = 1;
	}

	P4_Cell value = P4_POP(ctx->ds);
	if (is_static) {
		// Static fields come from a class.
		switch (*sig) {
		case 'Z':
			(*env)->SetStaticBooleanField(env, cls, fid, (jboolean) value.u);
			break;
		case 'B':
			(*env)->SetStaticByteField(env, cls, fid, (jbyte) value.n);
			break;
		case 'C':
			(*env)->SetStaticCharField(env, cls, fid, (jchar) value.u);
			break;
		case 'S':
			(*env)->SetStaticShortField(env, cls, fid, (jshort) value.n);
			break;
		case 'I':
			(*env)->SetStaticIntField(env, cls, fid, (jint) value.n);
			break;
		case 'J':
			(*env)->SetStaticLongField(env, cls, fid, (jlong) value.n);
			break;
		case 'D':
			(*env)->SetStaticDoubleField(env, cls, fid, (jdouble) value.f);
			break;
		case 'L':
			(*env)->SetStaticObjectField(env, cls, fid, (jobject) value.v);
			break;
		default:
			goto error1;
		}
	} else {
		// Instance fields come from an object.
		switch (*sig) {
		case 'Z':
			(*env)->SetBooleanField(env, cls_or_obj, fid, (jboolean) value.u);
			break;
		case 'B':
			(*env)->SetByteField(env, cls_or_obj, fid, (jbyte) value.n);
			break;
		case 'C':
			(*env)->SetCharField(env, cls_or_obj, fid, (jchar) value.u);
			break;
		case 'S':
			(*env)->SetShortField(env, cls_or_obj, fid, (jshort) value.n);
			break;
		case 'I':
			(*env)->SetIntField(env, cls_or_obj, fid, (jint) value.n);
			break;
		case 'J':
			(*env)->SetLongField(env, cls_or_obj, fid, (jlong) value.n);
			break;
		case 'D':
			(*env)->SetDoubleField(env, cls_or_obj, fid, (jdouble) value.f);
			break;
		case 'L':
			(*env)->SetObjectField(env, cls_or_obj, fid, (jobject) value.v);
			break;
		default:
			goto error1;
		}
	}
	(*env)->DeleteLocalRef(env, cls);
	return;
error1:
	(*env)->DeleteLocalRef(env, cls);
error0:
	LONGJMP(ctx->longjmp, P4_THROW_EINVAL);
}

static P4_Hook jHooks[] = {
	{ "jSetLocalCapacity", jSetLocalCapacity },
	{ "jDeleteLocalRef", jDeleteLocalRef },
	{ "jFindClass", jFindClass },
#ifdef HMM
	{ "jObjectClass", jObjectClass },
	{ "jMethodID", jMethodID },
	{ "jFieldID", jFieldID },
#endif
	{ "jPushLocalFrame", jPushLocalFrame },
	{ "jPopLocalFrame", jPopLocalFrame },
	{ "jStringByteLength", jStringByteLength },
	{ "jUnboxString", jUnboxString },
	{ "jArrayLength", jArrayLength },
	{ "jUnboxArray", jUnboxArray },
	{ "jBoxString", jBoxString },
	{ "jBoxArray", jBoxArray },
	{ "jSetField", jSetField },
	{ "jField", jField },
	{ "jCall", jCall },
	{ NULL, NULL }
};

static int
post4HookInit(P4_Ctx *ctx)
{
	int rc;
	P4_Hook *h;

	for (h = jHooks; h->name != NULL; h++) {
		if ((rc = p4HookAdd(ctx, h->name, h->func)) != P4_THROW_OK) {
			errx(EXIT_FAILURE, "hook %s fail %d", h->name, rc);
		}
	}

	return 0;
}
#endif

JNIEXPORT jlong JNICALL
Java_post4_jni_Post4_p4Create(JNIEnv *env, jobject self, jobject opts)
{
	P4_Ctx *ctx;
	jfieldID fid;
	P4_Options p4_opts;

	/* Map from object to struct. */
	jclass clazz = (*env)->GetObjectClass(env, opts);

	fid = (*env)->GetFieldID(env, clazz, "trace", "I");
	p4_opts.trace = (int)(*env)->GetIntField(env, opts, fid);
	fid = (*env)->GetFieldID(env, clazz, "ds_size", "I");
	p4_opts.ds_size = (unsigned)(*env)->GetIntField(env, opts, fid);
	fid = (*env)->GetFieldID(env, clazz, "fs_size", "I");
	p4_opts.fs_size = (unsigned)(*env)->GetIntField(env, opts, fid);
	fid = (*env)->GetFieldID(env, clazz, "rs_size", "I");
	p4_opts.rs_size = (unsigned)(*env)->GetIntField(env, opts, fid);
	fid = (*env)->GetFieldID(env, clazz, "mem_size", "I");
	p4_opts.mem_size = (unsigned)(*env)->GetIntField(env, opts, fid);

	fid = (*env)->GetFieldID(env, clazz, "core_file", "Ljava/lang/String;");
	jstring jcore_file = (*env)->GetObjectField(env, opts, fid);
	p4_opts.core_file = (*env)->GetStringUTFChars(env, jcore_file, NULL);
	if (p4_opts.core_file == NULL) {
		p4_opts.core_file = P4_CORE_FILE;
	}

	fid = (*env)->GetFieldID(env, clazz, "block_file", "Ljava/lang/String;");
	jstring jblock_file = (*env)->GetObjectField(env, opts, fid);
	p4_opts.block_file = (*env)->GetStringUTFChars(env, jblock_file, NULL);

	fid = (*env)->GetFieldID(env, clazz, "argv", "[Ljava/lang/String;");
	jobjectArray jargv = (*env)->GetObjectField(env, opts, fid);
	p4_opts.argc = (int) (*env)->GetArrayLength(env, jargv);
	if ((p4_opts.argv = malloc((p4_opts.argc + 1) * sizeof (*p4_opts.argv))) == NULL) {
		p4_opts.argv = (char **) empty_argv;
		p4_opts.argc = 0;
	} else {
		p4_opts.argv[p4_opts.argc] = NULL;
		for (int argi = 0; argi < p4_opts.argc; argi++) {
			jstring jstr = (*env)->GetObjectArrayElement(env, jargv, argi);
			const char *str = (*env)->GetStringUTFChars(env, jstr, NULL);
			p4_opts.argv[argi] = strdup(str);
			(*env)->ReleaseStringUTFChars(env, jstr, str);
			(*env)->DeleteLocalRef(env, jstr);
		}
	}
	(*env)->DeleteLocalRef(env, jargv);

	/* Create Post4 context. */
	ctx = p4Create(&p4_opts);

	(*env)->ReleaseStringUTFChars(env, jcore_file, p4_opts.core_file);
	(*env)->ReleaseStringUTFChars(env, jblock_file, p4_opts.block_file);
	(*env)->DeleteLocalRef(env, clazz);

	if (ctx == NULL) {
		(*env)->Throw(env, (*env)->FindClass(env, "java/lang/OutOfMemory"));
	}

#ifdef HAVE_HOOKS
	(void) post4HookInit(ctx);
#endif

	// https://stackoverflow.com/questions/1632367/passing-pointers-between-c-and-java-through-jni
	return (jlong) ctx;
}
