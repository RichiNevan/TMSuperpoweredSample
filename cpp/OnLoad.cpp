#include <jni.h>
#include <fbjni/fbjni.h>
#include "NativeSpecsModule.h" // if needed for internal setup
#include <android/log.h>

using namespace facebook;

// If you have any init logic or logging
extern "C" jint JNI_OnLoad(JavaVM* vm, void*) {
  __android_log_print(ANDROID_LOG_INFO, "NativeSpecs", "JNI_OnLoad called");
  return fbjni::initialize(vm, [] {});
}