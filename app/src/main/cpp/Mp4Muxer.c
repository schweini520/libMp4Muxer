//
// Created by Administrator on 2017/4/12.
//
#include <com_winstars_launcher_chat_codec_Mp4v2Helper.h>
#include "mp4record.h"
#include "mp4record.c"

MP4V2_CONTEXT *_mp4Handle;

/**
 * 初始化
 * @param env
 * @param jclass
 * @param path
 * @param width
 * @param height
 * @return
 */
jint JNICALL Java_com_winstars_launcher_chat_codec_Mp4v2Helper_initMp4Encoder
        (JNIEnv *env, jclass jclass, jstring path, jint width, jint height) {
    const char *local_title = (*env)->GetStringUTFChars(env, path, NULL);
    int m_width = width;
    int m_height = height;
    _mp4Handle = initMp4Encoder(local_title, m_width, m_height);
    return 0;
}
/**
 * 添加视频帧的方法
 */
JNIEXPORT jint JNICALL Java_com_winstars_launcher_chat_codec_Mp4v2Helper_mp4VEncode
        (JNIEnv *env, jclass clz, jbyteArray data, jint size) {
    unsigned char *buf = (unsigned char *) (*env)->GetByteArrayElements(env, data, JNI_FALSE);
    int nalsize = size;
   int reseult = mp4VEncode(_mp4Handle, buf, nalsize);
   // int reseult = mp4VEncodeNew(_mp4Handle, buf, nalsize);
    (*env)->ReleaseByteArrayElements(env, data, (jbyte *)buf, 0);
    return reseult;
}
/*
 * Class:     com_winstars_launcher_chat_codec_Mp4v2Helper
 * Method:    mp4VideoEncodeFrame
 * Signature: (I[B[B[BI)I
 */
JNIEXPORT jint JNICALL Java_com_winstars_launcher_chat_codec_Mp4v2Helper_mp4VideoEncodeFrame
(JNIEnv *env, jclass clz, jint isIFrame, jbyteArray sps, jbyteArray pps, jbyteArray data, jint size)
{
    unsigned char *buf = (unsigned char *) (*env)->GetByteArrayElements(env, data, JNI_FALSE);
    int nalsize = size;
    int isI = isIFrame;
    //int reseult = mp4VEncode(_mp4Handle, buf, nalsize);

    unsigned char *spsData = (unsigned char *) (*env)->GetByteArrayElements(env, sps, JNI_FALSE);
    unsigned char *ppsData = (unsigned char *) (*env)->GetByteArrayElements(env, pps, JNI_FALSE);

    int reseult = mp4VEncodeNew(_mp4Handle,isI, spsData,ppsData,buf, nalsize);


    (*env)->ReleaseByteArrayElements(env, data, (jbyte *)buf, 0);
    (*env)->ReleaseByteArrayElements(env, sps, (jbyte *)spsData, 0);
    (*env)->ReleaseByteArrayElements(env, pps, (jbyte *)ppsData, 0);
    return reseult;
}



/**
 * 添加音频数据
 */
JNIEXPORT jint JNICALL Java_com_winstars_launcher_chat_codec_Mp4v2Helper_mp4AEncode
        (JNIEnv *env, jclass clz, jbyteArray data, jint size) {
    unsigned char *buf = (unsigned char *) (*env)->GetByteArrayElements(env, data, JNI_FALSE);
    int nalsize = size;
    int reseult = mp4AEncode(_mp4Handle, &buf[7], nalsize-7);
    //减去7为了删除adts头部的7个字节
    (*env)->ReleaseByteArrayElements(env, data, (jbyte *)buf, 0);
    return reseult;
}
/**
 * 释放
 */
JNIEXPORT void JNICALL Java_com_winstars_launcher_chat_codec_Mp4v2Helper_closeMp4Encoder
        (JNIEnv *env, jclass clz) {
    closeMp4Encoder(_mp4Handle);
}




