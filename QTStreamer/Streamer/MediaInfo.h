#pragma once
#if 0
#include <stdint.h>

#define H264_PRESET_DEFAULT		0
#define H264_PRESET_ULTRAFAST	1
#define H264_PRESET_SUPERFAST	2
#define H264_PRESET_VERYFAST	3
#define H264_PRESET_FASTER		4
#define H264_PRESET_FAST		5
#define H264_PRESET_MEDIUM		6
#define H264_PRESET_SLOW		7
#define H264_PRESET_SLOWER		8
#define H264_PRESET_VERYSLOW	9
#define H264_PRESET_PLACEBO		10

#define H264_PROFILE_AUTO		0		// x264Ĭ��
#define H264_PROFILE_BASELINE	66
#define H264_PROFILE_MAIN		77
#define H264_PROFILE_HIGH		100
#define H264_PROFILE_HIGH10		110
#define H264_PROFILE_HIGH422	122
#define H264_PROFILE_HIGH444_PREDICTIVE 244


#define H264_ENCODE_EOF			(2)			// ����
#define H264_ENCODE_EAGAIN		(1)			// 
#define H264_ENCODE_GOT_PACKET	(0)
#define H264_ENCODE_ERROR		(-1)
typedef struct VideoEncoderConfig_
{
	int fps;			// ֡��
	int bitrate;		// �����ʣ���λkbps��
	int width;			// ��
	int height;			// ��
	int slice_max_size;	// ����slice�����ֽ���
	int gop;			// gop���
	int profile;		// ���뼶��
	int qp;				// quantization parameter, ȡֵ��Χ 0(�ϸ)~51(��ֲ�)��
	int use_qp;			// 0 ƽ�����ʣ�1 �㶨������2 �㶨����
	int bframes;		// �ο�֡����
	int preset;			// Ԥ�� ultrafast, superfast, veryfast, faster, fast, medium, slow, slower, veryslow, placebo.
}VideoEncoderConfig;

typedef struct MediaFrame
{
#define MEDIA_NUM_DATA_POINTERS 8
	uint8_t *data[MEDIA_NUM_DATA_POINTERS];
	int linesize[MEDIA_NUM_DATA_POINTERS];
	/**��
	* Presentation timestamp in time_base units (time when frame should be shown to user).
	*/
	int64_t pts;
	enum MediaPictureType pict_type;	//Picture type of the frame.
}MediaFrame;


typedef struct MediaPacket
{
	int64_t pts;
	int64_t dts;
	uint8_t *data;
	int size;
	int pkt_type;
}MediaPacket;
#endif