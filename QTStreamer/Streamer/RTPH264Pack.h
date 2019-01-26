#pragma once
#include "RTP.h"
#include "RTPPack.h"
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

typedef enum
{
	RTP_H264_IDR = 5,
	RTP_H264_SPS = 7,
	RTP_H264_PPS = 8,
	RTP_H264_FU_A = 28,
}RTP_H264_NAL_TYPE_T;

class RTPH264Pack
{
public:

	typedef struct nal_msg_
	{
		bool b_end_of_frame;
		uint8_t type;		// NAL type
		uint8_t *start;	// pointer to first location in the send buffer
		uint8_t *end;	// pointer to last location in send buffer
		uint32_t size;
	} NAL_MSG_T;
	

	typedef struct rtp_info_
	{
		NAL_MSG_T	nal;	    // NAL information
		RTP_HDR_T	rtp_hdr;    // RTP header is assembled here
		int hdr_len;			// length of RTP header

		uint8_t *p_rtp;		// pointer to where RTP packet has beem assembled
		uint8_t *start;		// pointer to start of payload
		uint8_t *end;		// pointer to end of payload

		uint32_t s_bit;		// bit in the FU header
		uint32_t e_bit;		// bit in the FU header
		bool FU_flag;		// fragmented NAL Unit flag
	} RTP_INFO_T;
public:
	RTPH264Pack(const uint32_t ssrc = 0,
		const uint8_t payload_type = RTP_PAYLOAD_TYPE_H264,
		const uint16_t rtp_packet_max_size = RTP_PACKET_MAX_SIZE);
	~RTPH264Pack();

	bool Pack(uint8_t *nal_buf, uint32_t nal_size,
		uint32_t timestamp, bool end_of_frame);

	//ѭ������Get��ȡRTP����ֱ������ֵΪNULL
	uint8_t* GetPacket(int &out_packet_size);

	RTP_INFO_T GetRtpInfo() const;
	void SetRtpInfo(const RTP_INFO_T &RTP_Info);
private:
	unsigned int StartCode(uint8_t *cp);

	RTP_INFO_T rtp_info_;
	bool b_begin_nal_;
	uint16_t rtp_packet_max_size_;
};

/**
 * ���͵�һ֡���������ݲŻ᷵��
 */
class RTPH264Unpack
{

#define RTP_VERSION 2
#define BUF_SIZE (1024 * 500)

	typedef struct
	{
		//LITTLE_ENDIAN
		uint16_t   cc : 4;		/* CSRC count                 */
		uint16_t   x : 1;		/* header extension flag      */
		uint16_t   p : 1;		/* padding flag               */
		uint16_t   v : 2;		/* packet type                */
		uint16_t   pt : 7;		/* payload type               */
		uint16_t   m : 1;		/* marker bit                 */

		uint16_t    seq;		/* sequence number            */
		uint32_t     ts;		/* timestamp                  */
		uint32_t     ssrc;		/* synchronization source     */
	} RTP_HDR_T;
public:
	RTPH264Unpack() {}
	RTPH264Unpack(uint8_t h264_playload_type = 96);
	~RTPH264Unpack(void);

	//pBufΪH264 RTP��Ƶ���ݰ���nSizeΪRTP��Ƶ���ݰ��ֽڳ��ȣ�outSizeΪ�����Ƶ����֡�ֽڳ��ȡ�
	//����ֵΪָ����Ƶ����֡��ָ�롣�������ݿ��ܱ��ƻ���
	uint8_t* ParseRtpPacket(uint8_t * p_buf, uint16_t buf_size, int &out_size, uint32_t &timestamp);

	void resetPacket();

private:
	RTP_HDR_T rtp_header_;

	uint8_t *receive_buf_data_;

	bool b_sps_found_;
	bool b_found_key_frame_;
	bool m_bAssemblingFrame;
	bool b_pre_frame_finish_;
	uint8_t *receive_buf_start_;
	uint8_t *receive_buf_end_;
	uint32_t cur_receive_size_;

	uint16_t seq_num_;
	bool resync_ = true;

	uint8_t h264_playload_type_ = 96;
	uint32_t ssrc_;
};


