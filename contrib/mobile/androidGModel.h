/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class org_geuz_onelab_Gmsh */

#ifndef _Included_org_geuz_onelab_Gmsh
#define _Included_org_geuz_onelab_Gmsh
void requestRender();
void getBitmapFromString(const char *text, int textsize, unsigned char **map, int *height, int *width, int *realWidth=NULL);
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     org_geuz_onelab_Gmsh
 * Method:    init
 * Signature: (Ljava/lang/String;)J
 */
JNIEXPORT jlong JNICALL Java_org_geuz_onelab_Gmsh_init
  (JNIEnv *, jobject, jstring);

/*
 * Class:     org_geuz_onelab_Gmsh
 * Method:    loadFile
 * Signature: (JLjava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_org_geuz_onelab_Gmsh_loadFile
  (JNIEnv *, jobject, jlong, jstring);

/*
 * Class:     org_geuz_onelab_Gmsh
 * Method:    initView
 * Signature: (JII)V
 */
JNIEXPORT void JNICALL Java_org_geuz_onelab_Gmsh_initView
  (JNIEnv *, jobject, jlong, jint, jint);

/*
 * Class:     org_geuz_onelab_Gmsh
 * Method:    drawView
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_org_geuz_onelab_Gmsh_drawView
  (JNIEnv *, jobject, jlong);

/*
 * Class:     org_geuz_onelab_Gmsh
 * Method:    eventHandler
 * Signature: (JIFF)V
 */
JNIEXPORT void JNICALL Java_org_geuz_onelab_Gmsh_eventHandler
  (JNIEnv *, jobject, jlong, jint, jfloat, jfloat);

/*
 * Class:     org_geuz_onelab_Gmsh
 * Method:    setStringOption
 * Signature: (Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_org_geuz_onelab_Gmsh_setStringOption
  (JNIEnv *, jobject, jstring, jstring, jstring);

/*
 * Class:     org_geuz_onelab_Gmsh
 * Method:    setDoubleOption
 * Signature: (Ljava/lang/String;Ljava/lang/String;D)I
 */
JNIEXPORT jint JNICALL Java_org_geuz_onelab_Gmsh_setDoubleOption
  (JNIEnv *, jobject, jstring, jstring, jdouble);

/*
 * Class:     org_geuz_onelab_Gmsh
 * Method:    setIntegerOption
 * Signature: (Ljava/lang/String;Ljava/lang/String;I)I
 */
JNIEXPORT jint JNICALL Java_org_geuz_onelab_Gmsh_setIntegerOption
  (JNIEnv *, jobject, jstring, jstring, jint);

/*
 * Class:     org_geuz_onelab_Gmsh
 * Method:    getStringOption
 * Signature: (Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_org_geuz_onelab_Gmsh_getStringOption
  (JNIEnv *, jobject, jstring, jstring);

/*
 * Class:     org_geuz_onelab_Gmsh
 * Method:    getDoubleOption
 * Signature: (Ljava/lang/String;Ljava/lang/String;)D
 */
JNIEXPORT jdouble JNICALL Java_org_geuz_onelab_Gmsh_getDoubleOption
  (JNIEnv *, jobject, jstring, jstring);

/*
 * Class:     org_geuz_onelab_Gmsh
 * Method:    getIntegerOption
 * Signature: (Ljava/lang/String;Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_org_geuz_onelab_Gmsh_getIntegerOption
  (JNIEnv *, jobject, jstring, jstring);

/*
 * Class:     org_geuz_onelab_Gmsh
 * Method:    getParams
 * Signature: ()[Ljava/lang/String;
 */
JNIEXPORT jobjectArray JNICALL Java_org_geuz_onelab_Gmsh_getParams
  (JNIEnv *, jobject);

/*
 * Class:     org_geuz_onelab_Gmsh
 * Method:    setParam
 * Signature: (Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_org_geuz_onelab_Gmsh_setParam
  (JNIEnv *, jobject, jstring, jstring, jstring);

/*
 * Class:     org_geuz_onelab_Gmsh
 * Method:    getPView
 * Signature: ()[Ljava/lang/String;
 */
JNIEXPORT jobjectArray JNICALL Java_org_geuz_onelab_Gmsh_getPView
  (JNIEnv *, jobject);

/*
 * Class:     org_geuz_onelab_Gmsh
 * Method:    setPView
 * Signature: (IIIIF)V
 */
JNIEXPORT void JNICALL Java_org_geuz_onelab_Gmsh_setPView
  (JNIEnv *, jobject, jint, jint, jint, jint, jfloat);

/*
 * Class:     org_geuz_onelab_Gmsh
 * Method:    onelabCB
 * Signature: (Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_org_geuz_onelab_Gmsh_onelabCB
  (JNIEnv *, jobject, jstring);

/*
 * Class:     org_geuz_onelab_Gmsh
 * Method:    animationNext
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_org_geuz_onelab_Gmsh_animationNext
  (JNIEnv *, jobject);

#ifdef __cplusplus
}
#endif
#endif
