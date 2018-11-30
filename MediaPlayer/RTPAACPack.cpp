#include "RTPAACPack.h"
#include <string.h>


/**
 * rtp����aac�ĸ�ʽ�����������:
* 2���ֽڵ�AU-headers-length
* n��AU-header��ÿ��2�ֽ�
* n��AU����aacȥ��adts���غ�
 */

RTPAACPack::RTPAACPack(const uint32_t ssrc, const uint8_t payload_type, 
	const int rtp_packet_max_size, const int max_frames_per_packet):
	rtp_packet_max_size_(rtp_packet_max_size),
	max_frames_per_packet_(max_frames_per_packet),
	seq_num_(0)
{
	if (rtp_packet_max_size_ > RTP_PACKET_MAX_SIZE)
	{
		rtp_packet_max_size_ = RTP_PACKET_MAX_SIZE;		// �������
	}
	memset(&rtp_hdr, 0, sizeof(rtp_hdr));
	rtp_hdr.set_type(payload_type);
	rtp_hdr.set_version(RTP_VERSION);
}

RTPAACPack::~RTPAACPack()
{

}

// Ŀǰֻ֧��ÿ��packet ��frame�����
bool RTPAACPack::Pack(uint8_t * buf, uint32_t buf_size, uint32_t timestamp, bool end_of_frame)
{
	// max_frames_per_packet_��ǰΪ1��ֻ֧�ֵ�֡ÿpacket�����
	const int max_au_headers_size = 2 + 2 * max_frames_per_packet_;
	uint8_t au_header_len[2];
	uint8_t au_header[2];

	// ����ADTS
	buf_size -= 7;
	buf += 7;
	if (RTP_HEADER_LEN + max_au_headers_size + buf_size > rtp_packet_max_size_)
	{
		return false;			// һ���������ռ䷢��һ֡AAC����
	}

	// max_frames_per_packet_ = 1�� ����Ŀǰֻ����һ֡��������Ӵ���Ч�ʵĽǶȿ�
	// ���Զ��aac����װһ���ٴ���
	au_header_len[0] = (((AU_HEADER_SIZE * max_frames_per_packet_) * 8) >> 8) & 0xff;
	au_header_len[1] = ((AU_HEADER_SIZE * max_frames_per_packet_) * 8) & 0xff;
	// ��13bit�洢
	au_header[0] = (buf_size & 0x1fe0) >> 5;	// �洢��8λ
	au_header[1] = (buf_size & 0x1f) << 3;		// �洢��5λ

	sendbuf_len_ = RTP_HEADER_LEN + max_au_headers_size + buf_size;

	rtp_hdr.set_mark(end_of_frame);
	rtp_hdr.set_seq_num(seq_num_++);
	rtp_hdr.set_time_stamp(timestamp);


	memcpy(sendbuf_, &rtp_hdr, RTP_HEADER_LEN);
	sendbuf_[RTP_HEADER_LEN + 0] = au_header_len[0];
	sendbuf_[RTP_HEADER_LEN + 1] = au_header_len[1];
	sendbuf_[RTP_HEADER_LEN + 2] = au_header[0];
	sendbuf_[RTP_HEADER_LEN + 3] = au_header[1];

	return true;
}

uint8_t * RTPAACPack::GetPacket(int & out_packet_size)
{
	if(0 == sendbuf_len_)
	{
		return nullptr;
	}
	out_packet_size = sendbuf_len_;
	sendbuf_len_ = 0;
	return sendbuf_;
}
