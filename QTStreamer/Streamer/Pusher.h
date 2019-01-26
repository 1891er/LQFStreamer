#pragma once
#include <mutex>
#include "Media.h"
#include "LQFRingBuffer.h"
#include "Thread.h"

typedef enum __PUSH_STATE_T
{
	PUSH_STATE_CONNECTING = 1,			/* ������ */
	PUSH_STATE_CONNECTED,				/* ���ӳɹ� */
	PUSH_STATE_CONNECT_FAILED,			/* ����ʧ�� */
	PUSH_STATE_CONNECT_ABORT,			/* �����쳣�ж� */
	PUSH_STATE_PUSHING,					/* ������ */
	PUSH_STATE_DISCONNECTED,			/* �Ͽ����� */
	PUSH_STATE_ERROR
}PUSH_STATE_T;

class Pusher : public Thread
{
public:
	Pusher(const int audio_samplerate = 44100, const int video_fps = 15);
	virtual ~Pusher();
	void SetAudioConfig(int profile, int samplerate, int channel);
	void SetVideoConfig(std::string &sps, std::string &pps, std::string &sei)
	{
		sps_ = sps;
		pps_ = pps;
		sei_ = sei;
	}

	virtual void Run() override;
	bool PushPacket(LQF::AVPacket &pkt);
	int GetAudioCachePackets() const;
	int GetVideoCachePackets() const;
	// ֻȥ�б�ǰ����ǰ���ֽ��Ƿ�����start code��ʣ���������Ϊ��nalu����
	bool readNaluFromBuf(const uint8_t * buffer, uint32_t nBufferSize, LQF::NaluUnit & nalu);

protected:
	// ����Ƶ���зֿ����
	std::shared_ptr<LQFRingBuffer<LQF::AVPacket>> audio_pkt_queue_;
	std::shared_ptr<LQFRingBuffer<LQF::AVPacket>> video_pkt_queue_;
	mutex mutex_;

	uint8_t aac_spec_[2];

	bool is_audio_metadata_send_ = false;
	bool is_video_metadata_send_ = false;

	// ��Ƶ��Ϣ
	int audio_profile_ = 2;
	int audio_samplerate_ = 44100;
	int audio_channel_ = 2;
	// ��Ƶ��Ϣ
	std::string sps_;
	std::string pps_;
	std::string sei_;

};

