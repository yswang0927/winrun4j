/*******************************************************************************
* This program and the accompanying materials
* are made available under the terms of the Common Public License v1.0
* which accompanies this distribution, and is available at 
* http://www.eclipse.org/legal/cpl-v10.html
* 
* Contributors:
*     Peter Smith
*******************************************************************************/

#include "Native.h"
#include "../common/Log.h"
#include "../java/JNI.h"
#include "../java/VM.h"

bool Native::RegisterNatives(JNIEnv *env)
{
	Log::Info("Registering natives for Native class");

	jclass clazz = JNI::FindClass(env, "org/boris/winrun4j/Native");
	if(clazz == NULL) {
		JNI::ClearException(env);
		Log::Warning("Could not find Native class");
		return false;
	}
	
	JNINativeMethod nm[11];
	nm[0].name = "loadLibrary";
	nm[0].signature = "(Ljava/lang/String;)J";
	nm[0].fnPtr = (void*) LoadLibrary;
	nm[1].name = "freeLibrary";
	nm[1].signature = "(J)V";
	nm[1].fnPtr = (void*) FreeLibrary;
	nm[2].name = "getProcAddress";
	nm[2].signature = "(JLjava/lang/String;)J";
	nm[2].fnPtr = (void*) GetProcAddress;
	nm[3].name = "malloc";
	nm[3].signature = "(I)J";
	nm[3].fnPtr = (void*) Malloc;
	nm[4].name = "free";
	nm[4].signature = "(J)V";
	nm[4].fnPtr = (void*) Free;
	nm[5].name = "fromPointer";
	nm[5].signature = "(JJ)Ljava/nio/ByteBuffer;";
	nm[5].fnPtr = (void*) FromPointer;
	nm[6].name = "call";
	nm[6].signature = "(J[JII)J";
	nm[6].fnPtr = (void*) Call;
	nm[7].name = "bind";
	nm[7].signature = "(Ljava/lang/Class;Ljava/lang/String;Ljava/lang/String;J)Z";
	nm[7].fnPtr = (void*) Bind;
	nm[8].name = "newGlobalRef";
	nm[8].signature = "(Ljava/lang/Object;)J";
	nm[8].fnPtr = (void*) NewGlobalRef;
	nm[9].name = "deleteGlobalRef";
	nm[9].signature = "(J)V";
	nm[9].fnPtr = (void*) DeleteGlobalRef;
	nm[10].name = "getMethodId";
	nm[10].signature = "(Ljava/lang/Class;Ljava/lang/String;Ljava/lang/String;Z)J";
	nm[10].fnPtr = (void*) GetMethodID;

	env->RegisterNatives(clazz, nm, 11);

	if(env->ExceptionCheck()) {
		JNI::PrintStackTrace(env);
		return false;
	}

	return true;
}

jlong Native::LoadLibrary(JNIEnv* env, jobject self, jstring filename)
{
	if(!filename)
		return 0;
	jboolean iscopy;
	const jchar* str = env->GetStringChars(filename, &iscopy);
	jlong res = (jlong) ::LoadLibraryW((LPCWSTR) str);
	env->ReleaseStringChars(filename, str);
	return res;
}

void Native::FreeLibrary(JNIEnv* env, jobject self, jlong handle)
{
	::FreeLibrary((HMODULE) handle);
}

jlong Native::GetProcAddress(JNIEnv* env, jobject self, jlong handle, jstring name)
{
	if(!name)
		return 0;
	jboolean iscopy;
	const char* str = env->GetStringUTFChars(name, &iscopy);
	jlong res = (jlong) ::GetProcAddress((HMODULE) handle, str);
	env->ReleaseStringUTFChars(name, str);
	return res;
}

jlong Native::Malloc(JNIEnv* env, jobject self, jint size)
{
	return (jlong) ::malloc(size);
}

void Native::Free(JNIEnv* env, jobject self, jlong handle)
{
	::free((void*) handle);
}

jobject Native::FromPointer(JNIEnv* env, jobject self, jlong handle, jlong size)
{
	return env->NewDirectByteBuffer((void*) handle, size);
}

#ifndef X64
jlong Native::Call(JNIEnv* env, jobject self, jlong handle, jlongArray stack, jint size, jint mode)
{
	jboolean iscopy;
	jlong* p = !stack ? (jlong*) 0 : (jlong*)env->GetPrimitiveArrayCritical(stack, &iscopy);
	if(!p && size > 0)
		return 0;
	for(int i = 0; i < size; i++) {
		int v = *p;
		__asm {
			push v
		}
		p++;
	}
	int r;
	FARPROC fp = (FARPROC) handle;
	__asm {
		call fp
		mov dword ptr[r], eax
	}
	env->ReleasePrimitiveArrayCritical(stack, p, 0);
	return (jlong) r;
}
#else

typedef int (__fastcall *FP)(...);

jlong Native::Call(JNIEnv* env, jobject self, jlong handle, jlongArray stack, jint size, jint mode)
{
	jboolean iscopy;
	jlong* p = !stack ? (jlong*) 0 : (jlong*) env->GetPrimitiveArrayCritical(stack, &iscopy);
	if(!p && size > 0)
		return 0;
	FP fp = (FP) handle;
	int r = 0;
	switch(size)
	{
	case 0:
		r = fp();
		break;
	case 1:
		r = fp(p[0]);
		break;
	case 2:
		r = fp(p[1], p[0]);
		break;
	case 3:
		r = fp(p[2], p[1], p[0]);
		break;
	case 4:
		r = fp(p[3], p[2], p[1], p[0]);
		break;
	case 5:
		r = fp(p[4], p[3], p[2], p[1], p[0]);
		break;
	case 6:
		r = fp(p[5], p[4], p[3], p[2], p[1], p[0]);
		break;
	case 7:
		r = fp(p[6], p[5], p[4], p[3], p[2], p[1], p[0]);
		break;
	case 8:
		r = fp(p[7], p[6], p[5], p[4], p[3], p[2], p[1], p[0]);
		break;
	case 9:
		r = fp(p[8], p[7], p[6], p[5], p[4], p[3], p[2], p[1], p[0]);
		break;
	case 10:
		r = fp(p[9], p[8], p[7], p[6], p[5], p[4], p[3], p[2], p[1], p[0]);
		break;
	case 11:
		r = fp(p[10], p[9], p[8], p[7], p[6], p[5], p[4], p[3], p[2], p[1], p[0]);
		break;
	case 12:
		r = fp(p[11], p[10], p[9], p[8], p[7], p[6], p[5], p[4], p[3], p[2], p[1], p[0]);
		break;
	case 13:
		r = fp(p[12], p[11], p[10], p[9], p[8], p[7], p[6], p[5], p[4], p[3], p[2], p[1], p[0]);
		break;
	case 14:
		r = fp(p[13], p[12], p[11], p[10], p[9], p[8], p[7], p[6], p[5], p[4], p[3], p[2], p[1], p[0]);
		break;
	case 15:
		r = fp(p[14], p[13], p[12], p[11], p[10], p[9], p[8], p[7], p[6], p[5], p[4], p[3], p[2], p[1], p[0]);
		break;
	case 16:
		r = fp(p[15], p[14], p[13], p[12], p[11], p[10], p[9], p[8], p[7], p[6], p[5], p[4], p[3], p[2], p[1], p[0]);
		break;
	case 17:
		r = fp(p[16], p[15], p[14], p[13], p[12], p[11], p[10], p[9], p[8], p[7], p[6], p[5], p[4], p[3], p[2], p[1], p[0]);
		break;
	case 18:
		r = fp(p[17], p[16], p[15], p[14], p[13], p[12], p[11], p[10], p[9], p[8], p[7], p[6], p[5], p[4], p[3], p[2], p[1], p[0]);
		break;
	case 19:
		r = fp(p[18], p[17], p[16], p[15], p[14], p[13], p[12], p[11], p[10], p[9], p[8], p[7], p[6], p[5], p[4], p[3], p[2], p[1], p[0]);
		break;
	case 20:
		r = fp(p[19], p[18], p[17], p[16], p[15], p[14], p[13], p[12], p[11], p[10], p[9], p[8], p[7], p[6], p[5], p[4], p[3], p[2], p[1], p[0]);
		break;
	}
	if(stack) env->ReleasePrimitiveArrayCritical(stack, p, 0);
	return (jlong) r;
}
#endif

jboolean Native::Bind(JNIEnv* env, jobject self, jclass clazz, jstring fn, jstring sig, jlong ptr)
{
	if(!clazz || !fn || !sig || !ptr)
		return false;

	jboolean iscopy = false;
	const char* cf = env->GetStringUTFChars(fn, &iscopy);
	const char* cs = env->GetStringUTFChars(sig, &iscopy);

	JNINativeMethod nm[1];
	nm[0].name = (char*) cf;
	nm[0].signature = (char*) cs;
	nm[0].fnPtr = (void*) ptr;

	env->RegisterNatives(clazz, nm, 1);

	if(env->ExceptionCheck()) {
		JNI::PrintStackTrace(env);
		env->ReleaseStringUTFChars(fn, cf);
		env->ReleaseStringUTFChars(sig, cs);
		return false;
	}
	env->ReleaseStringUTFChars(fn, cf);
	env->ReleaseStringUTFChars(sig, cs);

	return true;
}

jlong Native::NewGlobalRef(JNIEnv* env, jobject self, jobject obj)
{
	return (jlong) env->NewGlobalRef(obj);
}

void Native::DeleteGlobalRef(JNIEnv* env, jobject self, jlong handle)
{
	env->DeleteGlobalRef((jobject) handle);
}

jlong Native::GetMethodID(JNIEnv* env, jobject self, jclass clazz, jstring name, jstring sig, jboolean isStatic)
{
	const char* ns = env->GetStringUTFChars(name, 0);
	const char* ss = env->GetStringUTFChars(sig, 0);
	jlong res = isStatic ? (jlong) env->GetStaticMethodID(clazz, ns, ss) : 
		(jlong) env->GetMethodID(clazz, ns, ss);
	env->ReleaseStringUTFChars(name, ns);
	env->ReleaseStringUTFChars(sig, ss);
	return res;
}

extern "C" __declspec(dllexport) int __cdecl Native_Is64()
{
#ifdef X64
	return 1;
#else
	return 0;
#endif
}

extern "C" __declspec(dllexport) int __cdecl Native_Callback(jobject obj, jmethodID mid, int stack)
{
	JNIEnv* env = VM::GetJNIEnv(true);
	return env->CallIntMethod(obj, mid, stack+8);
}

