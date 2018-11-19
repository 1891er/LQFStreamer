#ifndef _X264_ENCODER_H_
#define _X264_ENCODER_H_
// ��׼���ļ�
#include <iostream>
#include <stdint.h>
// ��������ͷ�ļ�
#include <x264.h>
// �û�ͷ�ļ�

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
	int b_repeat_headers;	// �Ƿ�ÿ��I֡����SPS,PPS,SEI	
	int b_annexb;		// �Ƿ��start code
	int preset;			// Ԥ�� ultrafast, superfast, veryfast, faster, fast, medium, slow, slower, veryslow, placebo.
}VideoEncoderConfig;

typedef enum encode_ret
{
	kRetEncEagain,		// ����������
	kRetEncEof,			// ��ȡ���б���֡���
	kRetEncOk,			// ��ȡ������֡
	kRetEncError,		
}VideoEncodeRet; 

enum MediaPictureType {
	MD_PICTURE_TYPE_NONE = 0, ///< Undefined
	MD_PICTURE_TYPE_I,     ///< Intra
	MD_PICTURE_TYPE_IDR,
	MD_PICTURE_TYPE_P,     ///< Predicted
	MD_PICTURE_TYPE_B,     ///< Bi-dir predicted
	MD_PICTURE_TYPE_S,     ///< S(GMC)-VOP MPEG-4
	MD_PICTURE_TYPE_SI,    ///< Switching Intra
	MD_PICTURE_TYPE_SP,    ///< Switching Predicted
	MD_PICTURE_TYPE_BI,    ///< BI type
};

typedef struct MediaPacket
{
	int64_t pts;
	int64_t dts;
	uint8_t *data;
	int   size;
	int pkt_type;
}MediaPacket;

typedef struct MediaFrame 
{
#define MEDIA_NUM_DATA_POINTERS 8
	uint8_t *data[MEDIA_NUM_DATA_POINTERS];
	int linesize[MEDIA_NUM_DATA_POINTERS];
	/**
	* Presentation timestamp in time_base units (time when frame should be shown to user).
	*/
	int64_t pts;
	enum MediaPictureType pict_type;	//Picture type of the frame.
}MediaFrame;

typedef enum
{
	kH264_TYPE_B,
	kH264_TYPE_P,
	kH264_TYPE_I,
	kH264_TYPE_IDR,
	kH264_TYPE_AUTO
}H264_TYPE_E;

class  X264Encoder
{
public:
	
	X264Encoder();
	~X264Encoder();
	int Init(VideoEncoderConfig &encoder_config);
	void Deinit();
	int EncodeFrame(MediaPacket * pkt, const MediaFrame * frame);
	int UpdateEncoderInfo();
	int GetSPS(char *sps_buf, int &sps_len);
	int GetSPS(std::string &sps);
	std::string GetSPS() { return sps_; }
	int GetPPS(char *pps_buf, int &pps_len);
	int GetPPS(std::string &pps);
	std::string GetPPS() { return pps_; }
	int GetSEI(char *sei_buf, int &sei_len);
	int GetSEI(std::string &sei);
	std::string GetSEI() { return sei_; }
	// �ο�x264_param_parseԴ��
	int SetParam(const char *name, const char *value);
	// ר�������޸�����
	int SetBitrate(int bitrate);

	static const char *GetPresetString(int preset);
	static const char *GetProfileString(int profile);
	static const char *GetErrorString(int ret);
private:
	void encodeNALs(MediaPacket * pkt, const x264_nal_t * nals, int nnal);
private:
	//VideoEncoderConfig encoder_config;
	x264_param_t x264_params_;
	x264_picture_t pic_in_;
	x264_t *x264_handle_;
	x264_nal_t *p_nal_;
	int luma_size_;
	int chroma_size_;
	int width_;
	int height_;
	int fps_;
	int bitrate_;
	int gop_;					//ͼ�����д�С��Ҳ��I֡���
	int qp_;

	std::string sps_;
	std::string pps_;
	std::string sei_;
};
#endif

