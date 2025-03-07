#include "decoder.h"
#include <errno.h>
#include <stdio.h>

AVCodec *codec; // 记录编码详细信息
AVCodecParserContext *parser = NULL;
AVCodecContext *c = NULL; // 记录码流编码信息
AVPacket *pkt = NULL;
AVFrame *frame = NULL; // 存储视频中的帧
Imgdata *imgbuff = NULL;
static int codeId = 0;

int init_decoder(int codeid)
{
    codeId = codeid;
    switch (codeid) {
    case 0:
        codec = avcodec_find_decoder(AV_CODEC_ID_H264);
        break;

    case 1:
        codec = avcodec_find_decoder(AV_CODEC_ID_H265);
        break;

    default:
        printf("In C language no choose h.264/h.265 decoder\n");
        return -1;
    }

    if (NULL == codec) {
        printf("codec find decoder failed\n");
        return -1;
    }

    parser = av_parser_init(codec->id);
    if (NULL == parser) {
        printf("can't jsparser init!\n");
        return -1;
    }

    c = avcodec_alloc_context3(codec);
    if (NULL == c) {
        printf("can't allocate video jscodec context!\n");
        return -1;
    }

    if (avcodec_open2(c, codec, NULL) < 0) {
        printf("can't open codec\n");
        return -1;
    }

    frame = av_frame_alloc();
    if (NULL == frame) {
        printf("can't allocate video frame\n");
        return -1;
    }

    pkt = av_packet_alloc();
    if (NULL == pkt) {
        printf("can't allocate video packet\n");
        return -1;
    }

    imgbuff = (Imgdata *)malloc(sizeof(Imgdata));
    if (NULL == imgbuff) {
        printf("malloc imgbuff failed\n");
        return -1;
    }

    printf("In C language init decoder success!\n");
    return 0;
}

Imgdata *decoder_raw(char *framedata, int framesize)
{
    if (NULL == framedata || framesize <= 0) {
        printf("In C language framedata is null or framesize is zero\n");
        return NULL;
    }
    /*****************************************************
     *	过滤，第一次检测首帧是否是VPS/SPS，若不是跳过不解码
        第一次检测到首帧是VPS/SPS，后面不用检测，flag=0 --> flag=1
     *
     * **************************************************/
    int nut = 0;
    static int flag = 0;
    if (codeId == 0) { // h264
        nut = (char)framedata[4] & 0x1f;
        while (!flag) {
            if (7 == nut) { // PPS
                flag = 1;
                break;
            } else if (8 == nut) { // PPS
                return NULL;
            } else if (5 == nut) { // I
                return NULL;
            } else if (1 == nut) { //P
                return NULL;
            } else if (9 == nut) { // AUD
                return NULL;
            } else if (6 == nut) { //SEI
                return NULL;
            } else { // Unknow
                return NULL;
            }
        }
    } else {
        // h.265裸流
        nut = (char)(framedata[4] >> 1) & 0x3f;
        while (!flag) {
            if (nut == 32) { // VPS
                flag = 1;
                break;
            } else if (nut == 33) { // SPS
                return NULL;
            } else if (nut == 34) { // PPS
                return NULL;
            } else if (nut == 19) { // I帧
                return NULL;
            } else if (nut == 1) { // P帧
                return NULL;
            } else if (nut == 35) { // AUD
                return NULL;
            } else if (nut == 39) {// SEI
                return NULL;
            } else { // Unknown
                return NULL;
            }
        }
    }

    int ret = 0;
    int size = 0;
    while (framesize > 0) {
        size = av_parser_parse2(parser, c, &pkt->data, &pkt->size, (const uint8_t *)framedata, framesize,
                                AV_NOPTS_VALUE, AV_NOPTS_VALUE, AV_NOPTS_VALUE);
        if (size < 0) {
            printf("Error while parsing\n");
            return NULL;
        }
        framedata += size;
        framesize -= size;
        if (pkt->size) {
            ret = avcodec_send_packet(c, pkt);
            if (ret < 0) {
                printf("In C language error sending a packet for decoding\n");
                return NULL;
            } else {
                while (ret >= 0) {
                    ret = avcodec_receive_frame(c, frame);
                    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                        break;
                    } else if (ret < 0) {
                        printf("Error during decoding\n");
                        return NULL;
                    }

                    // yuv数据处理，保存在buff中,在网页中调用yuv420p数据交给webgl渲染
                    int yuvBytes = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, frame->width, frame->height, 1);
                    uint8_t *buff = (uint8_t *)malloc(yuvBytes);
                    if (buff == NULL) {
                        printf("malloc yuvBytes failed!\n");
                        return NULL;
                    }

                    imgbuff->width = frame->width;
                    imgbuff->height = frame->height;
                    imgbuff->data = process(frame, buff);
                    free(buff);
                }
            }
        }
    }
    return imgbuff;
}

uint8_t *process(AVFrame *frame, uint8_t *buff)
{
    int i, j, k;
    for (i = 0; i < frame->height; i++) {
        memcpy(buff + frame->width * i, frame->data[0] + frame->linesize[0] * i, frame->width);
    } for (j = 0; j < frame->height / 2; j++) {
        memcpy(buff + frame->width * i + frame->width / 2 * j, frame->data[1] + frame->linesize[1] * j, frame->width / 2);
    } for (k = 0; k < frame->height / 2; k++) {
        memcpy(buff + frame->width * i + frame->width / 2 * j + frame->width / 2 * k, frame->data[2] + frame->linesize[2] * k, frame->width / 2);
    }
    return buff;
}

void free_decoder()
{
    av_parser_close(parser);
    avcodec_free_context(&c);
    av_frame_free(&frame);
    av_packet_free(&pkt);
    if (imgbuff != NULL) {
        if (imgbuff->data != NULL) {
            free(imgbuff->data);
            imgbuff->data = NULL;
        }
        free(imgbuff);
        imgbuff = NULL;
    }
    printf("In C language free all requested memory! \n");
}