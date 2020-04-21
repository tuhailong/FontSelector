#include <jni.h>
#include <stdio.h>

extern int register_com_tuhailong_fontselector_SystemFontHelper(JNIEnv*);

jint JNI_OnLoad(JavaVM* vm, void* /*reserved*/) {
    JNIEnv* env = nullptr;
    if (vm->GetEnv((void**)&env, JNI_VERSION_1_4) != JNI_OK)
        return JNI_ERR;
    if (register_com_tuhailong_fontselector_SystemFontHelper(env))
        return JNI_ERR;
    return JNI_VERSION_1_4;
}

