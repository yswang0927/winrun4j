/*******************************************************************************
 * This program and the accompanying materials
 * are made available under the terms of the Common Public License v1.0
 * which accompanies this distribution, and is available at 
 * http://www.eclipse.org/legal/cpl-v10.html
 * 
 * Contributors:
 *     Peter Smith
 *******************************************************************************/

#ifndef NATIVE_H
#define NATIVE_H

#include "../common/Runtime.h"
#include <jni.h>

class Native {
public:
	static bool RegisterNatives(JNIEnv* env);

private:
	static jlong LoadLibrary(JNIEnv* env, jobject self, jstring filename);
	static void FreeLibrary(JNIEnv* env, jobject self, jlong handle);
	static jlong GetProcAddress(JNIEnv* env, jobject self, jlong handle, jstring name);
	static jlong Malloc(JNIEnv* env, jobject self, jint size);
	static void Free(JNIEnv* env, jobject self, jlong handle);
	static jobject FromPointer(JNIEnv* env, jobject self, jlong hanedle, jlong size);
	static jlong Call(JNIEnv* env, jobject self, jlong handle, jlongArray stack, jint size, jint mode);
	static jboolean Bind(JNIEnv* env, jobject self, jclass clazz, jstring fn, jstring sig, jlong ptr);
	static jlong NewGlobalRef(JNIEnv* env, jobject self, jobject obj);
	static void DeleteGlobalRef(JNIEnv* env, jobject self, jlong handle);
	static jlong GetMethodID(JNIEnv* env, jobject self, jclass clazz, jstring name, jstring sig, jboolean isStatic);
};

#endif // EVENTLOG_H