#pragma once
#include "RTP.h"
#include <rtppacket.h>
#include <vector>
#include <memory>
using namespace std;

using namespace jrtplib;

/*
1��RTP Header����
1)	V��RTPЭ��İ汾�ţ�ռ2λ����ǰЭ��汾��Ϊ2

2)	P������־��ռ1λ�����P=1�����ڸñ��ĵ�β�����һ����������İ�λ�飬���ǲ�����Ч�غɵ�һ���֡�

3)	X����չ��־��ռ1λ�����X=1������RTP��ͷ�����һ����չ��ͷ

4)	CC��CSRC��������ռ4λ��ָʾCSRC ��ʶ���ĸ���

5)	M: ��ǣ�ռ1λ����ͬ����Ч�غ��в�ͬ�ĺ��壬������Ƶ�����һ֡�Ľ�����������Ƶ����ǻỰ�Ŀ�ʼ��

6)	PT: ��Ч�������ͣ�ռ7λ������˵��RTP��������Ч�غɵ����ͣ���GSM��Ƶ��JPEMͼ���,����ý���д󲿷�������������Ƶ������Ƶ���ģ��������ڿͻ��˽��н�����

7)	���кţ�ռ16λ�����ڱ�ʶ�����������͵�RTP���ĵ����кţ�ÿ����һ�����ģ����к���1������ֶε��²�ĳ���Э����UDP��ʱ������״�����õ�ʱ�����������鶪����ͬʱ�������綶��������������������ݽ��������������кŵĳ�ʼֵ������ģ�ͬʱ��Ƶ������Ƶ����sequence�Ƿֱ�����ġ�

8)	ʱ��(Timestamp)��ռ32λ������ʹ��90 kHz ʱ��Ƶ�ʡ�ʱ����ӳ�˸�RTP���ĵĵ�һ����λ��Ĳ���ʱ�̡�������ʹ��ʱ���������ӳٺ��ӳٶ�����������ͬ�����ơ�

9)	ͬ����Դ(SSRC)��ʶ����ռ32λ�����ڱ�ʶͬ����Դ���ñ�ʶ�������ѡ��ģ��μ�ͬһ��Ƶ���������ͬ����Դ��������ͬ��SSRC��

10)	��Լ��Դ(CSRC)��ʶ����ÿ��CSRC��ʶ��ռ32λ��������0��15����ÿ��CSRC��ʶ�˰����ڸ�RTP������Ч�غ��е�������Լ��Դ��
*/

// �����rfc6184

typedef struct _RTP_H264_FRAM_T
{
	_RTP_H264_FRAM_T()
	{
		size = 0;
		flags = 0;
		timestamp = 0;
	}

	_RTP_H264_FRAM_T(uint32_t size)
		:data(new uint8_t[size])
	{
		this->size = size;
		flags = 0;
		timestamp = 0;
	}

	std::shared_ptr<uint8_t> data; /* ֡���� */
	int		size;
	int		flags;
	uint32_t timestamp;
}RTP_H264_FRAM_T;


/**
 * ���͵�һ֡���������ݲŻ᷵��
 */
class RTPH264Unpack
{
public:
	RTPH264Unpack(void);
	~RTPH264Unpack(void);
	int RTPH264UnpackInput(std::vector<RTP_H264_FRAM_T> &rtp_h264_frames, const void* packet, int bytes);
	///@return 0-ok, other-error
	int RTPPacketDeserialize(RTP_PACKET_T *pkt, const void* data, int bytes);
private:
	int rtpH264UnpackSTAP(std::vector<RTP_H264_FRAM_T> &rtp_h264_frames, const uint8_t* ptr,
		int bytes, uint32_t timestamp, int stap_b);
	int rtpH264UnpackMTAP(std::vector<RTP_H264_FRAM_T> &rtp_h264_frames,
		const uint8_t* ptr, int bytes, uint32_t timestamp, int n);
	int rtpH264UnpackFU(std::vector<RTP_H264_FRAM_T> &rtp_h264_frames,
		const uint8_t* ptr, int bytes, uint32_t timestamp, int fu_b);
	void pack(std::vector<RTP_H264_FRAM_T> &rtp_h264_frames, 
		const uint8_t *packet, int bytes, uint32_t timestamp, int flags);

	uint16_t seq_ = 0; // rtp seq
	uint8_t* ptr_ = NULL;
	int size_ = 0;
	int capacity_ = 0;

	int flags_ = -1;
};


