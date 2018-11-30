#pragma once
#include <cstdint>
#include "RTP.h"

#define AU_HEADER_LEN     2
#define AU_HEADER_SIZE    2

#define AAC_SINGLE_AU     1

// �ο� ffmpeg rtpenc_aac.c�� �����RFC3640
/**
 * ��Ҫ��aac��ADTSȥ����
 * 1.���12�ֽڵ�rtp��ͷ��
 * 2.���2�ֽڵ�AU_HEADER_LENGTH��
 * 3.���2�ֽڵ�AU_HEADER��
 * 3.�ӵ�17�ֽڿ�ʼ����payload��ȥ��ADTS��aac���ݣ�������
 */
class RTPAACPack
{
public:
	
// 		const uint16_t rtp_packet_max_size
	RTPAACPack(const uint32_t ssrc = 1,
				const uint8_t payload_type = RTP_PAYLOAD_TYPE_AAC,
				const int rtp_packet_max_size = RTP_PACKET_MAX_SIZE, 
				const int max_frames_per_packet = 1);
	~RTPAACPack();
	bool Pack(uint8_t *buf, uint32_t buf_size,
		uint32_t timestamp, bool end_of_frame);

	//ѭ������Get��ȡRTP����ֱ������ֵΪNULL
	uint8_t* GetPacket(int &out_packet_size);
private:
	RTP_HDR_T	rtp_hdr;    // RTP header is assembled here
	uint8_t sendbuf_[RTP_PACKET_MAX_SIZE];
	uint32_t sendbuf_len_ = 0;
	uint16_t seq_num_ = 0;
	 
	int max_frames_per_packet_ = 1;	// Ĭ��Ϊ1��ÿ��IP����һ֡AAC����
	int rtp_packet_max_size_ = RTP_PACKET_MAX_SIZE;
};

// AAC���
class RTPAACUnpack
{
};