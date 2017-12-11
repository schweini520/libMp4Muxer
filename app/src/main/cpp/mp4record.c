//
//  mp4record.c
//

#import "mp4record.h"
#import <stdlib.h>
#include <android/log.h>
#include<stdio.h>
#include<string.h>

#include <sys/time.h>

//#define LOGI(...) __android_log_print(ANDROID_LOG_INFO  , "TEST", __VA_ARGS__)
#define LOGI(FORMAT,...) __android_log_print(ANDROID_LOG_INFO,"ldw",FORMAT,##__VA_ARGS__);
#define LOGE(FORMAT,...) __android_log_print(ANDROID_LOG_ERROR,"ldw",FORMAT,##__VA_ARGS__);

int firstTime = 0;

unsigned int getTime()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	LOGE("getTimeStamp2 %ld  ",tv.tv_sec*1000);
	LOGE("getTimeStamp3 %ld  ",tv.tv_usec/1000);
	return (tv.tv_sec*1000 + tv.tv_usec/1000);
}




/**
 *
 * 初始化
 *
 * filename 保存文件的路径
 * width    视频的宽
 * height   视频的高
 *
 */
MP4V2_CONTEXT *initMp4Encoder(const char *filename, int width, int height) {
	firstTime = getTime();
    MP4V2_CONTEXT *recordCtx = (MP4V2_CONTEXT *) malloc(sizeof(struct MP4V2_CONTEXT));
    if (!recordCtx) {
        printf("error : malloc context \n");
        return NULL;
    }

    recordCtx->m_vWidth = width;
    recordCtx->m_vHeight = height;
    // recordCtx->m_vFrateR = 25;
    recordCtx->m_vFrateR = 15;
   // recordCtx->m_vTimeScale = 1000;
   recordCtx->m_vTimeScale = 90000;
    recordCtx->m_vFrameDur = 3000;
    recordCtx->m_vTrackId = MP4_INVALID_TRACK_ID;
    recordCtx->m_aTrackId = MP4_INVALID_TRACK_ID;

    recordCtx->m_mp4FHandle = MP4Create(filename, 0);
    if (recordCtx->m_mp4FHandle == MP4_INVALID_FILE_HANDLE) {
        LOGI("error : MP4Create  \n");
        return NULL;
    }
    MP4SetTimeScale(recordCtx->m_mp4FHandle, recordCtx->m_vTimeScale);
    //------------------------------------------------------------------------------------- audio track
    //这里的1024的值可以更改，设置小了会出现音频断断续续的情况 我之前设置的160，合成的音频断断续续
   // recordCtx->m_aTrackId = MP4AddAudioTrack(recordCtx->m_mp4FHandle, 8000, 1024,
    //                                         MP4_MPEG4_AUDIO_TYPE);

    //recordCtx->m_aTrackId = MP4AddAudioTrack(recordCtx->m_mp4FHandle, 22050, 1024,
     //                                        MP4_MPEG4_AUDIO_TYPE);

    recordCtx->m_aTrackId = MP4AddAudioTrack(recordCtx->m_mp4FHandle, 22050, 1024,
                                             MP4_MPEG4_AUDIO_TYPE);
    if (recordCtx->m_aTrackId == MP4_INVALID_TRACK_ID) {
        LOGI("error : MP4AddAudioTrack  \n");
        return NULL;
    }

    MP4SetAudioProfileLevel(recordCtx->m_mp4FHandle, 2);// 2, 1//    uint8_t aacConfig[2] = {18,16};
//    uint8_t aacConfig[2] = {0x14, 0x10};
  //  uint8_t aacConfig[2] = {0x15, 0x88};
    uint8_t aacConfig[2] = {0x13, 0x88};
    MP4SetTrackESConfiguration(recordCtx->m_mp4FHandle, recordCtx->m_aTrackId, aacConfig, 2);
    LOGI("ok  : initMp4Encoder file=%s  \n", filename);

    return recordCtx;
}


/**
 *
 *
 *
 *
 * 视频数据写进文件
 *
 *
 *
 */
int mp4VEncodeNew(MP4V2_CONTEXT *recordCtx,int isIframe,uint8_t *sps, uint8_t *pps ,uint8_t *_naluData, int _naluSize) {

    int index = -1;

    if (_naluData[0] == 0 && _naluData[1] == 0 && _naluData[2] == 0 && _naluData[3] == 1 &&
        (_naluData[4] & 0x1F) == 0x07) {
        index = _NALU_SPS_;
    }
    if (index != _NALU_SPS_ && recordCtx->m_vTrackId == MP4_INVALID_TRACK_ID) {
        return index;
    }
    if (_naluData[0] == 0 && _naluData[1] == 0 && _naluData[2] == 0 && _naluData[3] == 1 &&
        (_naluData[4] & 0x1F) == 0x08) {
        index = _NALU_PPS_;
    }
  /* if (_naluData[0] == 0 && _naluData[1] == 0 && _naluData[2] == 0 && _naluData[3] == 1 &&
        (_naluData[4] & 0x1F) == 0x05) {
        index = _NALU_I_;
    }*/

  // if (  ((_naluData[26] & 0x1F) == 0x65) || ((_naluData[26] & 0x1F) == 0x67)  || ((_naluData[26] & 0x1F) == 0x68)   ) {
  /* if(!recordCtx->m_vTrackId == MP4_INVALID_TRACK_ID)
   {
	   if (  (_naluData[26] == 101) || (_naluData[26] == 103)  || (_naluData[26] == 104)   ) {
	        index = _NALU_I_;
	    }

   }*/

    if(!recordCtx->m_vTrackId == MP4_INVALID_TRACK_ID && isIframe == 1)
     {
  	        index = _NALU_I_;

     }

    if (_naluData[0] == 0 && _naluData[1] == 0 && _naluData[2] == 0 && _naluData[3] == 1 &&
        (_naluData[4] & 0x1F) == 0x01) {
        index = _NALU_P_;
    }
    //
    switch (index) {
        case _NALU_SPS_:
            if (recordCtx->m_vTrackId == MP4_INVALID_TRACK_ID) {
                recordCtx->m_vTrackId = MP4AddH264VideoTrack
                        (recordCtx->m_mp4FHandle,
                         recordCtx->m_vTimeScale,
                         recordCtx->m_vTimeScale / recordCtx->m_vFrateR,
                         recordCtx->m_vWidth,     // width
                         recordCtx->m_vHeight,    // height
                         _naluData[5], // sps[1] AVCProfileIndication
                         _naluData[6], // sps[2] profile_compat
                         _naluData[7], // sps[3] AVCLevelIndication
                         3);           // 4 bytes length before each NAL unit
                if (recordCtx->m_vTrackId == MP4_INVALID_TRACK_ID) {
                    return -1;
                }
                MP4SetVideoProfileLevel(recordCtx->m_mp4FHandle, 0x7F); //  Simple Profile @ Level 3
            }

            // sps pps
            //00 00 00 01 67 42 00 1f e5 40 28 02 dc 80 00 00 00 01 68 ce 31 12
            //MP4AddH264SequenceParameterSet(recordCtx->m_mp4FHandle, recordCtx->m_vTrackId,
            //                               _naluData + 4, _naluSize - 4);
            //sps
           // MP4AddH264SequenceParameterSet(recordCtx->m_mp4FHandle, recordCtx->m_vTrackId,
           //                                _naluData + 4, _naluSize - 8);
           /* char sps [10];
            strncpy(sps, _naluData + 4, 10);

            char pps [4];
            strncpy(pps, _naluData + 18, 4);
            LOGE("sps  %d  ",sps[2]);
            LOGE("pps  %d  ",pps[2]);*/
            //MP4AddH264SequenceParameterSet(recordCtx->m_mp4FHandle, recordCtx->m_vTrackId,
            //                               _naluData + 4, 10);

            //pps
           // MP4AddH264PictureParameterSet(recordCtx->m_mp4FHandle, recordCtx->m_vTrackId,
            //                              _naluData + 18, 4);

            MP4AddH264SequenceParameterSet(recordCtx->m_mp4FHandle, recordCtx->m_vTrackId,
            		sps, 10);

            //pps
            MP4AddH264PictureParameterSet(recordCtx->m_mp4FHandle, recordCtx->m_vTrackId,
                                         pps, 4);
            LOGE("sps  我排第一\n");
            //
            break;
        case _NALU_PPS_:
            MP4AddH264PictureParameterSet(recordCtx->m_mp4FHandle, recordCtx->m_vTrackId,
                                          _naluData + 4, _naluSize - 4);
            LOGE("pps 我排第二\n");
            break;
        case _NALU_I_: {



            MP4AddH264SequenceParameterSet(recordCtx->m_mp4FHandle, recordCtx->m_vTrackId,
            		sps, 10);

            //pps
            MP4AddH264PictureParameterSet(recordCtx->m_mp4FHandle, recordCtx->m_vTrackId,
                                         pps, 4);




            _naluData[0] = (_naluSize - 4) >> 24;
            _naluData[1] = (_naluSize - 4) >> 16;
            _naluData[2] = (_naluSize - 4) >> 8;
            _naluData[3] = (_naluSize - 4) & 0xff;

             if (!MP4WriteSample(recordCtx->m_mp4FHandle, recordCtx->m_vTrackId, _naluData,
            		  _naluSize, MP4_INVALID_DURATION, 0, 1)) {
              	LOGE("Iframe MP4WriteSample error");
                  return -1;
              }
            //总时间变长，且不是按秒为单位在进
            /* int nowTime = getTime();

              if (!MP4WriteSample(recordCtx->m_mp4FHandle, recordCtx->m_vTrackId, _naluData,
            		  _naluSize, (nowTime-firstTime)*90000/1000, 0, 1)) {
              	LOGE("Iframe MP4WriteSample error");
                  return -1;
              }*/


              LOGE("Iframe \n");



















//            uint8_t * IFrameData = (uint8_t*)malloc(_naluSize+1);
//            //
//            IFrameData[0] = (_naluSize-3) >>24;
//            IFrameData[1] = (_naluSize-3) >>16;
//            IFrameData[2] = (_naluSize-3) >>8;
//            IFrameData[3] = (_naluSize-3) &0xff;
//
//            memcpy(IFrameData+4,_naluData+3,_naluSize-3);
////            if(!MP4WriteSample(recordCtx->m_mp4FHandle, recordCtx->m_vTrackId, IFrameData, _naluSize+1, recordCtx->m_vFrameDur/44100*90000, 0, 1)){
////                return -1;
////            }
////            recordCtx->m_vFrameDur = 0;
//            if(!MP4WriteSample(recordCtx->m_mp4FHandle, recordCtx->m_vTrackId, IFrameData, _naluSize+1, MP4_INVALID_DURATION, 0, 1)){
//                return -1;
//            }
//            free(IFrameData);
//            printf("Iframe 我排第三\n");
//            //

            //sps
             // MP4AddH264SequenceParameterSet(recordCtx->m_mp4FHandle, recordCtx->m_vTrackId,
           //                                  _naluData + 4, _naluSize - 8);

              //pps
             // MP4AddH264PictureParameterSet(recordCtx->m_mp4FHandle, recordCtx->m_vTrackId,
             //                               _naluData + 18, _naluSize - 4);

          /*  char sps [10];
            char* retsps = strncpy(sps, _naluData + 4, 10);

            char pps [4];
            strncpy(pps, _naluData + 18, 4);
            LOGE("retsps  %s  ",retsps);

            LOGE("sps  %d  ",sps[3]);
            LOGE("pps  %d  ",pps[3]);*/





            //26 = 22+4
            /*_naluData[22] = (_naluSize - 26) >> 24;
            _naluData[23] = (_naluSize - 26) >> 16;
            _naluData[24] = (_naluSize - 26) >> 8;
            _naluData[25] = (_naluSize - 26) & 0xff;*/

          /*  char iFrame [_naluSize - 21];
            char* ret = strncpy(iFrame, _naluData + 21, _naluSize - 21);
            LOGE("ret  %s  ",ret);
            LOGE("_naluSize  %d  ",_naluSize);
            LOGE("_naluData[24]  %d  ",_naluData[24]);
            LOGE("_naluData[25]  %d  ",_naluData[25]);
            LOGE("iFrame0  %d  ",iFrame[0]);
            LOGE("_naluData  %d  ",_naluData[0+22]);
            LOGE("iFrame1  %d  ",iFrame[1]);
            LOGE("_naluData  %d  ",_naluData[23]);
            LOGE("iFrame2  %d  ",iFrame[2]);
            LOGE("_naluData  %d  ",_naluData[24]);
            LOGE("iFrame3  %d  ",iFrame[9]);
            LOGE("_naluData  %d  ",_naluData[31]);
            LOGE("iFrame4  %d  ",iFrame[20]);
            LOGE("_naluData  %d  ",_naluData[42]);
            LOGE("iFrame5  %d  ",iFrame[5]);
            LOGE("_naluData  %d  ",_naluData[27]);
            LOGE("iFrame6  %d  ",iFrame[56]);
            LOGE("_naluData  %d  ",_naluData[78]);
            LOGE("iFrame7  %d  ",iFrame[7]);
            LOGE("_naluData  %d  ",_naluData[29]);
            LOGE("iFrame8  %d  ",iFrame[8]);
            LOGE("_naluData  %d  ",_naluData[30]);
            LOGE("iFrame_naluSize - 25  %d  ",iFrame[_naluSize - 25]);
            LOGE("_naluSize end  %d  ",_naluData[_naluSize-1]);


            MP4AddH264SequenceParameterSet(recordCtx->m_mp4FHandle, recordCtx->m_vTrackId,
            		sps, 10);

            //pps
            MP4AddH264PictureParameterSet(recordCtx->m_mp4FHandle, recordCtx->m_vTrackId,
                                         pps, 4);


              if (!MP4WriteSample(recordCtx->m_mp4FHandle, recordCtx->m_vTrackId, iFrame,
            		  _naluSize - 22, MP4_INVALID_DURATION, 0, 1)) {
              	LOGE("Iframe MP4WriteSample error");
                  return -1;
              }*/

             // LOGE("Iframe \n");



   /*         {
                _naluData[0] = (_naluSize - 4) >> 24;
                _naluData[1] = (_naluSize - 4) >> 16;
                _naluData[2] = (_naluSize - 4) >> 8;
                _naluData[3] = (_naluSize - 4) & 0xff;

//                if(!MP4WriteSample(recordCtx->m_mp4FHandle, recordCtx->m_vTrackId, _naluData, _naluSize, recordCtx->m_vFrameDur/8000*90000, 0, 1)){
//                    return -1;
//                }
//                recordCtx->m_vFrameDur = 0;
                if (!MP4WriteSample(recordCtx->m_mp4FHandle, recordCtx->m_vTrackId, _naluData,
                                    _naluSize, MP4_INVALID_DURATION, 0, 1)) {
                	LOGE("Iframe MP4WriteSample error");
                    return -1;
                }
                LOGE("Iframe 我排第三\n");
            }*/
            break;
        }
        case _NALU_P_: {
            _naluData[0] = (_naluSize - 4) >> 24;
            _naluData[1] = (_naluSize - 4) >> 16;
            _naluData[2] = (_naluSize - 4) >> 8;
            _naluData[3] = (_naluSize - 4) & 0xff;

//            if(!MP4WriteSample(recordCtx->m_mp4FHandle, recordCtx->m_vTrackId, _naluData, _naluSize, recordCtx->m_vFrameDur/8000*90000, 0, 0)){
//                return -1;
//            }
//            recordCtx->m_vFrameDur = 0;
            if (!MP4WriteSample(recordCtx->m_mp4FHandle, recordCtx->m_vTrackId, _naluData,
                                _naluSize, MP4_INVALID_DURATION, 0, 0)) {
            	LOGE("Pframe MP4WriteSample error");
                return -1;
            }
            //总时间变长，且不是按秒为单位在进
           /* int nowTime = getTime();
            if (!MP4WriteSample(recordCtx->m_mp4FHandle, recordCtx->m_vTrackId, _naluData,
                                _naluSize, (nowTime-firstTime)*90000/1000, 0, 0)) {
            	LOGE("Pframe MP4WriteSample error");
                return -1;
            }*/
            LOGE("Pframe 我排第四\n");
            break;
        }
    }
    return 0;
}




/**
 *
 *
 *
 *
 * 视频数据写进文件
 *
 *
 *
 */
int mp4VEncode(MP4V2_CONTEXT *recordCtx, uint8_t *_naluData, int _naluSize) {

    int index = -1;

    if (_naluData[0] == 0 && _naluData[1] == 0 && _naluData[2] == 0 && _naluData[3] == 1 &&
        (_naluData[4] & 0x1F) == 0x07) {
        index = _NALU_SPS_;
    }
    if (index != _NALU_SPS_ && recordCtx->m_vTrackId == MP4_INVALID_TRACK_ID) {
        return index;
    }
    if (_naluData[0] == 0 && _naluData[1] == 0 && _naluData[2] == 0 && _naluData[3] == 1 &&
        (_naluData[4] & 0x1F) == 0x08) {
        index = _NALU_PPS_;
    }
   /* if (_naluData[0] == 0 && _naluData[1] == 0 && _naluData[2] == 0 && _naluData[3] == 1 &&
        (_naluData[4] & 0x1F) == 0x05) {
        index = _NALU_I_;
    }*/
  // if (  ((_naluData[26] & 0x1F) == 0x65) || ((_naluData[26] & 0x1F) == 0x67)  || ((_naluData[26] & 0x1F) == 0x68)   ) {
   if(!recordCtx->m_vTrackId == MP4_INVALID_TRACK_ID)
   {
	   if (  (_naluData[26] == 101) || (_naluData[26] == 103)  || (_naluData[26] == 104)   ) {
	        index = _NALU_I_;
	    }
   }

    if (_naluData[0] == 0 && _naluData[1] == 0 && _naluData[2] == 0 && _naluData[3] == 1 &&
        (_naluData[4] & 0x1F) == 0x01) {
        index = _NALU_P_;
    }
    //
    switch (index) {
        case _NALU_SPS_:
            if (recordCtx->m_vTrackId == MP4_INVALID_TRACK_ID) {
                recordCtx->m_vTrackId = MP4AddH264VideoTrack
                        (recordCtx->m_mp4FHandle,
                         recordCtx->m_vTimeScale,
                         recordCtx->m_vTimeScale / recordCtx->m_vFrateR,
                         recordCtx->m_vWidth,     // width
                         recordCtx->m_vHeight,    // height
                         _naluData[5], // sps[1] AVCProfileIndication
                         _naluData[6], // sps[2] profile_compat
                         _naluData[7], // sps[3] AVCLevelIndication
                         3);           // 4 bytes length before each NAL unit
                if (recordCtx->m_vTrackId == MP4_INVALID_TRACK_ID) {
                    return -1;
                }
                MP4SetVideoProfileLevel(recordCtx->m_mp4FHandle, 0x7F); //  Simple Profile @ Level 3
            }

            // sps pps
            //00 00 00 01 67 42 00 1f e5 40 28 02 dc 80 00 00 00 01 68 ce 31 12
            //MP4AddH264SequenceParameterSet(recordCtx->m_mp4FHandle, recordCtx->m_vTrackId,
            //                               _naluData + 4, _naluSize - 4);
            //sps
           // MP4AddH264SequenceParameterSet(recordCtx->m_mp4FHandle, recordCtx->m_vTrackId,
           //                                _naluData + 4, _naluSize - 8);
            char sps [10];
            strncpy(sps, _naluData + 4, 10);

            char pps [4];
            strncpy(pps, _naluData + 18, 4);
            LOGE("sps  %d  ",sps[2]);
            LOGE("pps  %d  ",pps[2]);
            //MP4AddH264SequenceParameterSet(recordCtx->m_mp4FHandle, recordCtx->m_vTrackId,
            //                               _naluData + 4, 10);

            //pps
           // MP4AddH264PictureParameterSet(recordCtx->m_mp4FHandle, recordCtx->m_vTrackId,
            //                              _naluData + 18, 4);

            MP4AddH264SequenceParameterSet(recordCtx->m_mp4FHandle, recordCtx->m_vTrackId,
            		sps, 10);

            //pps
            MP4AddH264PictureParameterSet(recordCtx->m_mp4FHandle, recordCtx->m_vTrackId,
                                         pps, 4);
            LOGE("sps  我排第一\n");
            //
            break;
        case _NALU_PPS_:
            MP4AddH264PictureParameterSet(recordCtx->m_mp4FHandle, recordCtx->m_vTrackId,
                                          _naluData + 4, _naluSize - 4);
            LOGE("pps 我排第二\n");
            break;
        case _NALU_I_: {
//            uint8_t * IFrameData = (uint8_t*)malloc(_naluSize+1);
//            //
//            IFrameData[0] = (_naluSize-3) >>24;
//            IFrameData[1] = (_naluSize-3) >>16;
//            IFrameData[2] = (_naluSize-3) >>8;
//            IFrameData[3] = (_naluSize-3) &0xff;
//
//            memcpy(IFrameData+4,_naluData+3,_naluSize-3);
////            if(!MP4WriteSample(recordCtx->m_mp4FHandle, recordCtx->m_vTrackId, IFrameData, _naluSize+1, recordCtx->m_vFrameDur/44100*90000, 0, 1)){
////                return -1;
////            }
////            recordCtx->m_vFrameDur = 0;
//            if(!MP4WriteSample(recordCtx->m_mp4FHandle, recordCtx->m_vTrackId, IFrameData, _naluSize+1, MP4_INVALID_DURATION, 0, 1)){
//                return -1;
//            }
//            free(IFrameData);
//            printf("Iframe 我排第三\n");
//            //

            //sps
             // MP4AddH264SequenceParameterSet(recordCtx->m_mp4FHandle, recordCtx->m_vTrackId,
           //                                  _naluData + 4, _naluSize - 8);

              //pps
             // MP4AddH264PictureParameterSet(recordCtx->m_mp4FHandle, recordCtx->m_vTrackId,
             //                               _naluData + 18, _naluSize - 4);

            char sps [10];
            char* retsps = strncpy(sps, _naluData + 4, 10);

            char pps [4];
            strncpy(pps, _naluData + 18, 4);
            LOGE("retsps  %s  ",retsps);

            LOGE("sps  %d  ",sps[3]);
            LOGE("pps  %d  ",pps[3]);





            //26 = 22+4
            /*_naluData[22] = (_naluSize - 26) >> 24;
            _naluData[23] = (_naluSize - 26) >> 16;
            _naluData[24] = (_naluSize - 26) >> 8;
            _naluData[25] = (_naluSize - 26) & 0xff;*/

            char iFrame [_naluSize - 21];
            char* ret = strncpy(iFrame, _naluData + 21, _naluSize - 21);
            LOGE("ret  %s  ",ret);
            LOGE("_naluSize  %d  ",_naluSize);
            LOGE("_naluData[24]  %d  ",_naluData[24]);
            LOGE("_naluData[25]  %d  ",_naluData[25]);
            LOGE("iFrame0  %d  ",iFrame[0]);
            LOGE("_naluData  %d  ",_naluData[0+22]);
            LOGE("iFrame1  %d  ",iFrame[1]);
            LOGE("_naluData  %d  ",_naluData[23]);
            LOGE("iFrame2  %d  ",iFrame[2]);
            LOGE("_naluData  %d  ",_naluData[24]);
            LOGE("iFrame3  %d  ",iFrame[9]);
            LOGE("_naluData  %d  ",_naluData[31]);
            LOGE("iFrame4  %d  ",iFrame[20]);
            LOGE("_naluData  %d  ",_naluData[42]);
            LOGE("iFrame5  %d  ",iFrame[5]);
            LOGE("_naluData  %d  ",_naluData[27]);
            LOGE("iFrame6  %d  ",iFrame[56]);
            LOGE("_naluData  %d  ",_naluData[78]);
            LOGE("iFrame7  %d  ",iFrame[7]);
            LOGE("_naluData  %d  ",_naluData[29]);
            LOGE("iFrame8  %d  ",iFrame[8]);
            LOGE("_naluData  %d  ",_naluData[30]);
            LOGE("iFrame_naluSize - 25  %d  ",iFrame[_naluSize - 25]);
            LOGE("_naluSize end  %d  ",_naluData[_naluSize-1]);


            MP4AddH264SequenceParameterSet(recordCtx->m_mp4FHandle, recordCtx->m_vTrackId,
            		sps, 10);

            //pps
            MP4AddH264PictureParameterSet(recordCtx->m_mp4FHandle, recordCtx->m_vTrackId,
                                         pps, 4);


              if (!MP4WriteSample(recordCtx->m_mp4FHandle, recordCtx->m_vTrackId, iFrame,
            		  _naluSize - 22, MP4_INVALID_DURATION, 0, 1)) {
              	LOGE("Iframe MP4WriteSample error");
                  return -1;
              }

             // LOGE("Iframe \n");



   /*         {
                _naluData[0] = (_naluSize - 4) >> 24;
                _naluData[1] = (_naluSize - 4) >> 16;
                _naluData[2] = (_naluSize - 4) >> 8;
                _naluData[3] = (_naluSize - 4) & 0xff;

//                if(!MP4WriteSample(recordCtx->m_mp4FHandle, recordCtx->m_vTrackId, _naluData, _naluSize, recordCtx->m_vFrameDur/8000*90000, 0, 1)){
//                    return -1;
//                }
//                recordCtx->m_vFrameDur = 0;
                if (!MP4WriteSample(recordCtx->m_mp4FHandle, recordCtx->m_vTrackId, _naluData,
                                    _naluSize, MP4_INVALID_DURATION, 0, 1)) {
                	LOGE("Iframe MP4WriteSample error");
                    return -1;
                }
                LOGE("Iframe 我排第三\n");
            }*/
            break;
        }
        case _NALU_P_: {
            _naluData[0] = (_naluSize - 4) >> 24;
            _naluData[1] = (_naluSize - 4) >> 16;
            _naluData[2] = (_naluSize - 4) >> 8;
            _naluData[3] = (_naluSize - 4) & 0xff;

//            if(!MP4WriteSample(recordCtx->m_mp4FHandle, recordCtx->m_vTrackId, _naluData, _naluSize, recordCtx->m_vFrameDur/8000*90000, 0, 0)){
//                return -1;
//            }
//            recordCtx->m_vFrameDur = 0;
            if (!MP4WriteSample(recordCtx->m_mp4FHandle, recordCtx->m_vTrackId, _naluData,
                                _naluSize, MP4_INVALID_DURATION, 0, 0)) {
            	LOGE("Pframe MP4WriteSample error");
                return -1;
            }
            LOGE("Pframe 我排第四\n");
            break;
        }
    }
    return 0;
}


/**
 *
 *
 *
 *
 * 音频数据写进文件
 *
 *
 *
 */
int mp4AEncode(MP4V2_CONTEXT *recordCtx, uint8_t *data, int len) {
    if (recordCtx->m_vTrackId == MP4_INVALID_TRACK_ID) {
        return -1;
    }
    MP4WriteSample(recordCtx->m_mp4FHandle, recordCtx->m_aTrackId, data, len, MP4_INVALID_DURATION,
                  0, 1);//这里的最后一个参数表示音频是否同步，1表示同步

    /*int  nowTime = getTime();
    MP4WriteSample(recordCtx->m_mp4FHandle, recordCtx->m_aTrackId, data, len, (nowTime-firstTime)*22050/1000,
                   0, 1);//这里的最后一个参数表示音频是否同步，1表示同步*/
    recordCtx->m_vFrameDur += 1024;
    return 0;
}



/**
 *
 * 停止合成
 *
 */
void closeMp4Encoder(MP4V2_CONTEXT *recordCtx) {
    if (recordCtx) {
        if (recordCtx->m_mp4FHandle != MP4_INVALID_FILE_HANDLE) {
            MP4Close(recordCtx->m_mp4FHandle, 0);
            recordCtx->m_mp4FHandle = NULL;
        }

        free(recordCtx);
        recordCtx = NULL;
    }
    printf("ok  : closeMp4Encoder  \n");


}

