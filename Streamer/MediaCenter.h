#pragma once
#include <iostream>
#include "AACEncoder.h"
#include "AudioCapture.h"
#include "X264Encoder.h"
#include "VideoCapture.h"
#include "RtmpPusher.h"
#include "RecoderPusher.h"
#include "Media.h"
using namespace std;


// Ӧ������
class MediaCenter
{
public:
	MediaCenter();
	virtual ~MediaCenter();
	uint32_t VideoGetTimeStamp();
	uint32_t AudioGetTimeStamp(uint32_t sampleRate);
	bool StartRtmpPush(const string url, int timeout);
	bool StopRtmpPush(void);

	bool StartRecord(string url);
	bool StopRecord(void);

	bool ConfigAudioEncoder(AudioEncoderConfig &config);	// ����audio����������
	void GetAudioEncoderConfig(AudioEncoderConfig &config);	// ��ȡaudio����������
	bool ConfigVideoEncoder(VideoEncoderConfig &config);//����video����������
	void GetVideoEncoderConfig(VideoEncoderConfig &config);// ��ȡvideo����������

														   // ���ò������
	void ConfigAudioCapture(AudioCaptureConfig &config);
	void ConfigVideoCapture(VideoCaptureConfig &config);

	void InitAVTimer();
	bool StartAudio();			// ����audioԴ
	bool StopAudio();			// ֹͣaudioԴ
	bool StartVideo();			// ����videoԴ
	bool StopVideo();			// ֹͣvideoԴ
private:
	friend void TriggerEncodeVideoCallback(void *user)
	{
		MediaCenter *media_center = (MediaCenter *)user;
		media_center->TriggerEncodeVideo();
	}
	void TriggerEncodeVideo();
	void WaitTriggerEncodeVideo();
	void PushAudio();
	void PushVideo();
	bool pushVideoMetadata();	// ����video��sps pps sei��

	// ���ò���
	AudioEncoderConfig audio_encoder_config_;
	VideoEncoderConfig video_encoder_config_;
	AudioCaptureConfig audio_capture_config_;
	VideoCaptureConfig video_capture_config_;
	// 
	std::shared_ptr<std::thread> video_thread_;
	std::shared_ptr<std::thread> audio_thread_;
	bool vid_thread_req_abort_ = false;
	bool aud_thread_req_abort_ = false;
	// ��Ƶ��Ϣ
	std::string sps_;
	std::string pps_;
	std::string sei_;

	std::shared_ptr<AudioCapture> audio_capture_ = nullptr;
	std::shared_ptr<VideoCapture> video_capture_ = nullptr;

	std::shared_ptr<AACEncoder>  audio_encoder_ = nullptr;
	std::shared_ptr<X264Encoder> video_encoder_ = nullptr;
	std::mutex mutex_vid_enc_trigger_;
	std::condition_variable cv_vid_enc_trigger_; //��������

	// RTMP
	std::shared_ptr<RtmpPusher> rtmp_pusher_ = nullptr;
	string rtmp_push_url_;		// ��������
	bool rtmp_push_enable_ = false;	

	// Recoder
	std::shared_ptr<RecoderPusher> recod_pusher_ = nullptr;
	string recod_push_url_;		// ��������
	bool record_push_enable_ = false;			// �Ƿ�������
};