#ifdef __cplusplus
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/imgutils.h"
#include "libswresample/swresample.h"
#include <portaudio.h>
}
#endif

#include "RTPReceiver.h"
#include "LogUtil.h"

#define AVCODE_MAX_AUDIO_FRAME_SIZE	192000  // �ز������ܹ���������ռ�

#define PORT_AUDIO_LATENCY  (0.1);  // 0.1�룬Ҳ����˵PortAudio����Ҫ�ۻ�0.1������ݺ����������

// ��Ƶ���� ��Ƶ¼�� PortAudio
typedef struct audio_param
{
	PaStreamParameters *output_parameters;
	PaStream *pa_stream;
	int sample_rate;
	int format;

	AVFrame want_frame;			// ָ��PortAudio�����ʽ
	int quit;                   // �Ƿ��˳��߳�
	SwrContext	*swr_ctx;       // ��ƵPCM��ʽת��
	int sample_size;			// һ��sampleռ�õ��ֽڣ����ͨ����
}AUDIO_PARAM_T;

static AUDIO_PARAM_T s_audio_param;

int AudioInit(int sample_rate, int channels)
{
	FunEntry();
	int ret;

	//1 ��ʼ����Ƶ����豸
	//memset(&sAudioParam, 0, sizeof(sAudioParam));
	// ��ʼ����Ƶ����豸������PortAudio��
	if (Pa_Initialize() != paNoError)
	{
		LogError("Pa_Initialize failed");
		return -1;
	}
	memset(&s_audio_param, 0, sizeof(s_audio_param));
	// ����PaStreamParameters
	s_audio_param.output_parameters = (PaStreamParameters *)malloc(sizeof(PaStreamParameters));
	s_audio_param.output_parameters->suggestedLatency = PORT_AUDIO_LATENCY;			// ����latencyΪ0.3��
	s_audio_param.output_parameters->sampleFormat = paInt16;						// PCM��ʽ
	s_audio_param.output_parameters->hostApiSpecificStreamInfo = NULL;
	// ������Ƶ��Ϣ, ��������Ƶ�豸
	s_audio_param.output_parameters->channelCount =channels;      // ͨ������
	// ��Ӧffmpeg�ĸ�ʽ
	if (s_audio_param.output_parameters->sampleFormat == paFloat32)                 // ĿǰPortAudio����ֻ����paFloat32��paInt16���ָ�ʽ
	{
		s_audio_param.format = AV_SAMPLE_FMT_FLT;		// ��Ӧffmpeg�ĸ�ʽ
	}
	else if (s_audio_param.output_parameters->sampleFormat == paInt16)
	{
		s_audio_param.format = AV_SAMPLE_FMT_S16;
	}
	else
	{
		s_audio_param.format = AV_SAMPLE_FMT_S16;
	}
	s_audio_param.sample_rate = sample_rate;

	// ��ȡ��Ƶ����豸
	s_audio_param.output_parameters->device = Pa_GetDefaultOutputDevice();
	if (s_audio_param.output_parameters->device < 0)
	{
		LogError("Pa_GetDefaultOutputDevice failed, index = %d", s_audio_param.output_parameters->device);
		return -1;
	}
	// ��һ�������
	if ((ret = Pa_OpenStream(&(s_audio_param.pa_stream), NULL, 
		s_audio_param.output_parameters, sample_rate, 0, 0, 
		NULL, NULL)) != paNoError)
	{
		LogError("Pa_OpenStream open failed, ret = %d", ret);
		return -1;
	}

	// ����PortAudio��Ҫ�ĸ�ʽ
	s_audio_param.want_frame.format = s_audio_param.format;
	s_audio_param.want_frame.sample_rate = s_audio_param.sample_rate;
	s_audio_param.want_frame.channel_layout = av_get_default_channel_layout(s_audio_param.output_parameters->channelCount);
	s_audio_param.want_frame.channels = s_audio_param.output_parameters->channelCount;

	// ��ʼ����ƵPCM��ʽת����
	s_audio_param.swr_ctx = NULL;

	s_audio_param.sample_size = s_audio_param.want_frame.channels  * av_get_bytes_per_sample((enum AVSampleFormat) (s_audio_param.want_frame.format));

	FunExit();
	return 0;
}

int AudioPlay(const uint8_t *data, const uint32_t frames)
{
	if (!Pa_IsStreamActive(s_audio_param.pa_stream))
		Pa_StartStream(s_audio_param.pa_stream);

	if (Pa_WriteStream(s_audio_param.pa_stream, data, frames) == paUnanticipatedHostError)
	{
		LogError("Pa_WriteStream failed");
		return -1;
	}
	return 0;
}

void AudioClose(void)
{
	FunEntry();
	if (s_audio_param.output_parameters)
	{
		if (s_audio_param.pa_stream)
		{
			Pa_StopStream(s_audio_param.pa_stream);
			Pa_CloseStream(s_audio_param.pa_stream);
			s_audio_param.pa_stream = NULL;
		}
		Pa_Terminate();
		free(s_audio_param.output_parameters);
	}
	if (s_audio_param.swr_ctx)
	{
		swr_free(&s_audio_param.swr_ctx);
	}
	FunExit();
}


int RTP_AAC_Receiver_Test(int argc, char *argv[])
{
	RTPReceiver rtp_receiver(10);
	RTP_CONNECT_PARAM_T connect_param;
	connect_param.enable_rtp_recv = true;		// �������
	connect_param.listen_ip[0] = 127;			// �ػ�����
	connect_param.listen_ip[1] = 0;
	connect_param.listen_ip[2] = 0;
	connect_param.listen_ip[3] = 1;
	connect_param.listen_port = 9004;
	connect_param.timestamp_unit = 1000;
	connect_param.payload_type = RTP_PAYLOAD_TYPE_AAC;
	connect_param.audio_param.profile = 2;		// ����LC AAC
	connect_param.audio_param.frequency_index = 4;
	connect_param.audio_param.channel_configuration = 2;
	connect_param.enable_rtp_send = true;		// 
	connect_param.dest_ip[0] = 127;			// �ػ�����
	connect_param.dest_ip[1] = 0;
	connect_param.dest_ip[2] = 0;
	connect_param.dest_ip[3] = 1;
	connect_param.dest_port = 8000;
	if (!rtp_receiver.Init(connect_param))
	{
		LogError("RTPReceiver init failed");
		return -1;
	}
	

	AVCodecID codec_id = AV_CODEC_ID_AAC;
	LogDebug("avcodec_find_decoder");
	AVCodec* pCodec = avcodec_find_decoder(codec_id);
	if (!pCodec)
	{
		LogError("avcodec_find_decoder failed");
		return -1;
	}
	LogDebug("avcodec_alloc_context3");
	AVCodecContext *pCodecCtx = avcodec_alloc_context3(pCodec);
	if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0)
	{
		LogError("avcodec_open2 failed");
		return -1;
	}

	// ����һ��Frame
	AVFrame *pFrame = av_frame_alloc();   /* av_frame_free(&pFrame); */
	AVPacket *packet = av_packet_alloc();
	av_init_packet(packet);
	int ret = 0;

	int is_display_open = 0;
	LogDebug("wait rtp packet.........");

	// ��Ƶ���
	uint8_t *audio_sample_buf = (uint8_t *)malloc(AVCODE_MAX_AUDIO_FRAME_SIZE);
	int nb_samples = 0;

	// AudioInit�е㻨ʱ�� ����2��ĳ�ʼ��
	if (AudioInit(44100, 2) < 0)
	{
		LogError("AudioInit failed");
		return -1;
	}

	if (!rtp_receiver.Start())
	{
		LogError("RTPReceiver start failed");
		return -1;
	}

	while (true)      // ��ѭ��
	{
send_packet_again:
		LQF::AVPacket rtp_pkt;
		if (rtp_receiver.PopPacket(rtp_pkt))
		{
			// ȡ֡�ɹ������·�װ��ffmpeg AVPacket���н���
			packet->data = (uint8_t*)rtp_pkt.buffer.get();
			packet->size = rtp_pkt.size;
			packet->pts = rtp_pkt.timestamp;
			ret = avcodec_send_packet(pCodecCtx, packet);
			if (ret == AVERROR_EOF)
			{
				LogError("avcodec_send_packet failed, ret = %d", ret);
			}
			else if (ret == EAGAIN)
			{
				goto send_packet_again;
			}
			ret = avcodec_receive_frame(pCodecCtx, pFrame);
			if (ret == 0)		// ��ȡ������֡�򲥷�
			{ 
				
				if(!s_audio_param.swr_ctx)
				{ 
					s_audio_param.swr_ctx = swr_alloc_set_opts(NULL,
						s_audio_param.want_frame.channel_layout,
						(enum AVSampleFormat) (s_audio_param.want_frame.format),
						s_audio_param.want_frame.sample_rate,
						pFrame->channel_layout,
						(enum AVSampleFormat) (pFrame->format),
						pFrame->sample_rate,
						0, NULL);
					if (s_audio_param.swr_ctx == NULL || swr_init(s_audio_param.swr_ctx) < 0)
					{
						LogError("swr_init error");
						break;
					}
				}
				// ת��ΪPortAudio�����Ҫ�����ݸ�ʽ
				nb_samples = swr_convert(s_audio_param.swr_ctx,
					&audio_sample_buf,
					1024/2,//AVCODE_MAX_AUDIO_FRAME_SIZE / s_audio_param.sample_size,
					(const uint8_t * *)pFrame->data,
					pFrame->nb_samples);

				if (AudioPlay(audio_sample_buf, nb_samples) < 0)
				{
					LogError("failed");
					break;
				}
			}
			else if (ret == AVERROR(EAGAIN))
			{
				// û��֡�ɶ����ȴ���һ��������ٶ�ȡ
				continue;
			}
		}

		Sleep(5);
	}
	// ������
	rtp_receiver.Stop();
	av_packet_free(&packet);
	av_frame_free(&pFrame);
	avcodec_close(pCodecCtx);
	avcodec_free_context(&pCodecCtx);
	AudioClose();
	if (audio_sample_buf)
	{
		free(audio_sample_buf);
	}
	return 0;
}