#ifndef __RTMP_PUSHER_H__
#define __RTMP_PUSHER_H__


#include <memory>
#include <mutex>
#include "Media.h"
#include "RingBuffer.h"
#include "Pusher.h"

// ��������
#include "librtmp/rtmp.h"
using namespace LQF;
//#pragma comment(lib,"ws2_32.lib")



class RtmpPusher : public Pusher
{
public:
	static const int AAC_PROFILE_MAIN = 1;
	static const int AAC_PROFILE_LC = 2;
	static const int AAC_PROFILE_SSR = 3;
public:
	RtmpPusher(const int audio_samplerate = 44100, const int video_fps = 15);
	virtual ~RtmpPusher();
	void SetAudioConfig(int profile, int sample_rate, int channel);
	void SetVideoConfig(std::string &sps, std::string &pps, std::string &sei)
	{
		sps_ = sps;
		pps_ = pps;
		sei_ = sei;
	}
	// Connect����RTMP���ӣ�������RTMP�������߳�
	bool Connect(const char* url, int timeout);
	// Disconnectȡ������RTMP���ӣ���ֹͣRTMP�����߳�
	bool Disconnect();
	virtual void Run() override;
private:
	RTMP *rtmp_handle_ = nullptr;
};

#endif // !__RTMP_PUSHER_H__