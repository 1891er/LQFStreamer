#include "RtmpPusher.h"
#include "H264RTMPPackager.h"
#include "AACRTMPPackager.h"
#include "LogUtil.h"
#include "TimeUtil.h"
#include <iostream>

#include "librtmp/log.h"

static const char start_code_4bytes[4] = { 0x00, 0x00, 0x00, 0x01 };
static const char start_code_3bytes[3] = { 0x00, 0x00, 0x01 };

RtmpPusher::RtmpPusher(const int audio_samplerate, const int video_fps):
	Pusher(audio_samplerate, video_fps)
{
	//RTMP_LogLevel loglvl = RTMP_LOGALL;
	//RTMP_LogSetLevel(loglvl);
	rtmp_handle_ = RTMP_Alloc();
	RTMP_Init(rtmp_handle_);

	// ��ʱ��Ϊһ֡�Ĳ��������̶�Ϊ1024
	audio_pkt_queue_.reset(new RingBuffer<AVPacket>(1000.0 * 1024 / audio_samplerate));		// ֻ�洢һ֡������
	// 
	video_pkt_queue_.reset(new RingBuffer<AVPacket>(video_fps));		// ֻ�洢һ֡������
}

RtmpPusher::~RtmpPusher()
{
	Disconnect();

	if(rtmp_handle_)
	{ 
		RTMP_Free(rtmp_handle_);
		rtmp_handle_ = NULL;
	}
}

void RtmpPusher::SetAudioConfig(int profile, int samplerate, int channel)
{
	audio_profile_ = profile;
	audio_samplerate_ = samplerate;
	audio_channel_ = channel;
}

bool RtmpPusher::Connect(const char * url,int timeout)
{
	rtmp_handle_->Link.timeout = timeout;
	if (!RTMP_SetupURL(rtmp_handle_, (char *)url)) 
	{
		return false;
	}

	RTMP_EnableWrite(rtmp_handle_);

	if (!RTMP_Connect(rtmp_handle_, NULL)) 
	{
		RTMP_Free(rtmp_handle_);
		return false;
	}

	if (!RTMP_ConnectStream(rtmp_handle_, 0))
	{
		RTMP_Close(rtmp_handle_);
		return false;
	}

	return Thread::Start();
}

bool RtmpPusher::Disconnect()
{
	if (rtmp_handle_)
	{
		if (RTMP_IsConnected(rtmp_handle_))
		{
			RTMP_Close(rtmp_handle_);
		}
	}
	if (Thread::IsRunning())
		return Thread::Stop();
	else
		return true;
}



const int kMaxLen = 1024 * 1024;
void RtmpPusher::Run()
{
	RTMPPacket rtmp_packet;
	H264RTMPPackager h264_packager;
	AACRTMPPackager  aac_packager;
	char *rtmp_pkt_buf = new char[kMaxLen];
	int offset;			// h264 nal start code ���ȣ�����00 00 01Ϊ3�ֽڣ�00 00 00 01Ϊ4�ֽ�

	is_audio_metadata_send_ = false;
	is_video_metadata_send_ = false;

	FILE *file_h264 = fopen("video.h264", "wb+");
	if (!file_h264)
	{
		printf("open testVideoCaptureAndEncodeH264.h264 failed\n");
	}
	FILE *file_aac = fopen("audio.aac", "wb+");
	while (true)
	{
		if (requet_abort_)
		{
			break;
		}
		AVPacket av_pkt;
		bool b_get_packet = false;

		// ������Ƶ��
		if (video_pkt_queue_->Pop(av_pkt))
		{
			b_get_packet = true;
			// ��ȥ�ж��ǲ���sps pps
			// ��װ
			NaluUnit nalu;
			if (av_pkt.size >= kMaxLen)
			{
				LogError("NAL̫����");
				continue;
			}
			fwrite((uint8_t *)av_pkt.buffer.get(), av_pkt.size, 1, file_h264);
			fflush(file_h264);
			if(readNaluFromBuf((uint8_t *)av_pkt.buffer.get(), av_pkt.size, nalu))
			{
				if (nalu.type == 0x07)
				{
					sps_.clear();
					sps_.append(nalu.data, nalu.data + nalu.size);
					continue;		// ��Ҫ�ȴ�pps��������Ҫ���¶�ȡpacket
				}
				else if (nalu.type == 0x08)
				{
					pps_.clear();
					pps_.append(nalu.data, nalu.data + nalu.size);
					h264_packager.Metadata(&rtmp_packet, rtmp_pkt_buf, (uint8_t *)sps_.c_str(), sps_.size(),
						(uint8_t *)pps_.c_str(), pps_.size());
					is_video_metadata_send_ = true;
				}
				else
				{
					h264_packager.Pack(&rtmp_packet, rtmp_pkt_buf, (const char *)nalu.data, nalu.size);
				}
				rtmp_packet.m_nInfoField2 = rtmp_handle_->m_stream_id;
				// ʱ�������ʱ�����ᵼ�²����ӳٷǳ���
				rtmp_packet.m_nTimeStamp = av_pkt.timestamp;
				if (!RTMP_IsConnected(rtmp_handle_))
				{
					LogError("rtmp is not connect\n");
				}
				//LogDebug("video stamp = %u", rtmp_packet.m_nTimeStamp);
				//LogDebug("video stamp = %u, body_size = %d, nal_type = %d", 
				//	rtmp_packet.m_nTimeStamp, rtmp_packet.m_nBodySize, nalu.type);					
				if (!RTMP_SendPacket(rtmp_handle_, &rtmp_packet, 0))
				{
					LogError("fail to send rtmp_packet");
				}
			}
		}

		// ������Ƶ��
		if (audio_pkt_queue_->Pop(av_pkt))
		{
			b_get_packet = true;
			//fwrite((uint8_t *)frame.buffer.get(), frame.size, 1, file_aac);
			//fflush(file_aac);

			if (!is_audio_metadata_send_)
			{
				AACRTMPPackager::GetAudioSpecificConfig(aac_spec_, audio_profile_,
					audio_samplerate_, audio_channel_);
				//h264_packager.Pack(&rtmp_packet, rtmp_pkt_buf, av_pkt.buffer.get(), av_pkt.size);
				aac_packager.Metadata(&rtmp_packet, rtmp_pkt_buf, (const char *)aac_spec_, 2);
				is_audio_metadata_send_ = true;
			}
			else
			{
				aac_packager.Pack(&rtmp_packet, rtmp_pkt_buf, av_pkt.buffer.get(), av_pkt.size);
			}
			rtmp_packet.m_nInfoField2 = rtmp_handle_->m_stream_id;
			rtmp_packet.m_nTimeStamp = av_pkt.timestamp;

			//LogDebug("audio stamp = %u", rtmp_packet.m_nTimeStamp);
			if (!RTMP_IsConnected(rtmp_handle_))
			{
				LogError("rtmp is not connect\n");
			}
			if (!RTMP_SendPacket(rtmp_handle_, &rtmp_packet, 0))
			{
				LogError("fail to send rtmp_packet");
			}
		}
		if(!b_get_packet)			// �������Ƶ���ж�Ϊ��������
			Sleep(5);
	}
	if (rtmp_pkt_buf)
	{
		delete[] rtmp_pkt_buf;
		rtmp_pkt_buf = NULL;
	}
}

