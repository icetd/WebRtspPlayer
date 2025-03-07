#ifndef _DECODER_H_
#define _DECODER_H_


#include "libswscale/swscale.h"
#include "libavutil/opt.h"
#include "libavutil/imgutils.h"
#include "libavcodec/avcodec.h"

#define QUEUE_SIZE	20

typedef struct _imgdata{
	int width;
	int height;
	uint8_t* data;
}Imgdata;

/**1: h265  0: h264 */
extern int init_decoder(int codeid);
extern Imgdata *decoder_raw(char *framedata, int framesize);
extern uint8_t *process(AVFrame *frame, uint8_t *buff);
extern void free_decoder();

#endif
