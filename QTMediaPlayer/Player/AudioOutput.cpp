#include "AudioOutput.h"
#include "MediaPlayer.h"
#include "FrameQueue.h"
#include "PacketQueue.h"
#include "Clock.h"
#include "sonic.h"
#include "logger.h"
using namespace toolkit;
#ifdef __cplusplus
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswresample/swresample.h"
#include "SDL.h"
#include "libavutil/imgutils.h"
#include "libavutil/time.h"
#include "libavutil/common.h"
}
#endif

#define CHANGE_AUDIO_SPEED 1
// ��ʼ�����֣��ȴ���һ����
static sonicStream audio_speed_convert;

FILE *pcm_file = NULL;
static void sdl_audio_callback(void *opaque, Uint8 *stream, int len)
{
	MediaPlayer *player_ = (MediaPlayer *)opaque;
	int64_t dec_channel_layout;
	int     wanted_nb_samples;
	int data_size = 0;
	int resampled_data_size;
	Frame *af = NULL;
	int actual_out_samples;
	int copy_size = 0;
#ifdef CONVERT_AUDIO_FMT_BY_USER
	int16_t samples_buf[1024 * 2];            // ֻ�������Լ�ת����ʽʱ��
	int16_t *pcm = (int16_t *)samples_buf;
#endif
	int temp_len = len;

	//    LOG_DEBUG(DEBUG_AUDIO_DECODE | DBG_TRACE, "\ninto len = %d",len);
	player_->audio_callback_time = av_gettime_relative();
	while (len > 0)
	{
		if (player_->audio_buf_index >= player_->audio_buf_size)
		{   // ֡����������Ѿ���������ϣ�����Ҫ�����������
			// �Ӷ����л�ȡ���ݰ������н���, ����ͣ��ȥ��ȡ����
			retry:
			if (player_->sampq->frame_queue_nb_remaining() > 0 && !player_->paused)
			{
				af = player_->sampq->frame_queue_peek();       // ����ͺܼ򵥣�ȡ��ǰ��һ֡�����Ƿ��Լ�������ʾ��ʱ���
				if (af->serial != player_->audioq->serial_)
				{
					player_->sampq->frame_queue_next();
					goto retry;			// seekʱ��Ҫ������е�����
				}
				player_->audio_clock = af->pts / player_->speed_;
				player_->audio_clock_serial = af->serial;
				player_->audio_buf_index = 0;            // �µ�һ֡����
													// ����ÿһ֡�����ݳ���  AV_SAMPLE_FMT_FLTP
				data_size = av_samples_get_buffer_size(NULL, af->frame->channels,
					af->frame->nb_samples,
					(AVSampleFormat)af->frame->format, 1);
				actual_out_samples = af->frame->nb_samples; // �ز�����ʵ������ĵ�ͨ����������
				dec_channel_layout =
					(af->frame->channel_layout
						&& af->frame->channels == av_get_channel_layout_nb_channels(af->frame->channel_layout)) ?
					af->frame->channel_layout : av_get_default_channel_layout(af->frame->channels);
				wanted_nb_samples = af->frame->nb_samples;      // ��audio master��ʱ���õ���

																// �ԱȽ�����֡��ʽ���ѱ����֡��ʽ�Ƿ�һ�£������һ������������swr_ctx
				if ((af->frame->format != player_->audio_src->fmt)
					|| (dec_channel_layout != player_->audio_src->channel_layout)
					|| (af->frame->sample_rate != player_->audio_src->freq)
					|| (wanted_nb_samples != af->frame->nb_samples && !player_->swr_ctx))
				{
					// LOG_DEBUG(DEBUG_AUDIO_DECODE | DBG_TRACE, "swr_free");

					// ����Ƶ���������仯ʱ�����³�ʼ��
					swr_free(&player_->swr_ctx);
					player_->swr_ctx = swr_alloc_set_opts(NULL,
						player_->audio_tgt->channel_layout,
						player_->audio_tgt->fmt,
						player_->audio_tgt->freq,
						dec_channel_layout,
						(AVSampleFormat)af->frame->format,
						af->frame->sample_rate,
						0,
						NULL);
					if (!player_->swr_ctx || swr_init(player_->swr_ctx) < 0)
					{
						av_log(NULL, AV_LOG_ERROR,
							"Cannot create sample rate converter for conversion of %d Hz %s %d channels to %d Hz %s %d channels!\n",
							af->frame->sample_rate,
							av_get_sample_fmt_name((AVSampleFormat)af->frame->format),
							af->frame->channels,
							player_->audio_tgt->freq,
							av_get_sample_fmt_name((AVSampleFormat)player_->audio_tgt->fmt),
							player_->audio_tgt->channels);
						swr_free(&player_->swr_ctx);
						return;
					}
					player_->audio_src->channel_layout = dec_channel_layout;
					player_->audio_src->channels = af->frame->channels;
					player_->audio_src->freq = (AVSampleFormat)af->frame->sample_rate;
					player_->audio_src->fmt = (AVSampleFormat)af->frame->format;
				}

				if (player_->swr_ctx)
				{
					const uint8_t **in = (const uint8_t **)af->frame->extended_data;
					uint8_t **out = &player_->audio_buf1;
					// �����������ͨ���Ĳ���������+256��Ϊ�˱������
					int out_count = (int64_t)wanted_nb_samples *
						player_->audio_tgt->freq / af->frame->sample_rate + 256;
					// ����out_count��������*channels���Լ�������ʽ�����������Ҫ��buffer�ռ�
					int out_size = av_samples_get_buffer_size(NULL,
						player_->audio_tgt->channels,
						out_count,
						player_->audio_tgt->fmt, 0);

					if (out_size < 0)
					{
						av_log(NULL, AV_LOG_ERROR, "av_samples_get_buffer_size() failed\n");
						return;
					}

					av_fast_malloc(&player_->audio_buf1, &player_->audio_buf1_size, out_size);
					if (!player_->audio_buf1)
					{
						ErrorL <<"av_fast_malloc failed";
						return;//AVERROR(ENOMEM);
					}
					actual_out_samples = swr_convert(player_->swr_ctx, out, out_count, in, af->frame->nb_samples);
					if (actual_out_samples < 0) {
						ErrorL << "swr_convert() failed";
						return;
					}
					if (actual_out_samples == out_count) {
						ErrorL << "audio buffer is probably too small";
						if (swr_init(player_->swr_ctx) < 0)
							swr_free(&player_->swr_ctx);
					}
					// ָ�򻺴���
					resampled_data_size = actual_out_samples * player_->audio_tgt->channels
						* av_get_bytes_per_sample(player_->audio_tgt->fmt);
					player_->audio_buf = player_->audio_buf1; //  player_->audio_buf1_size
					player_->audio_buf_size = resampled_data_size;   // Ϊʲô����out_size
				}
				else
				{
					av_fast_malloc(&player_->audio_buf1, &player_->audio_buf1_size, data_size);
					memcpy(player_->audio_buf1, af->frame->data[0], data_size);
					player_->audio_buf = player_->audio_buf1;
					player_->audio_buf_size = data_size;
				}

#ifdef CHANGE_AUDIO_SPEED
				// ���ٲ���
				static int s_in_cout = 0;
				static int s_out_cout = 0;

				int sonic_samples;
				// ���㴦���ĵ���
				int numSamples = actual_out_samples / player_->speed_;
				data_size = av_samples_get_buffer_size(NULL, player_->audio_tgt->channels,
					(numSamples + 1),
					(AVSampleFormat)player_->audio_tgt->fmt, 1);
				// �������
				av_fast_malloc(&player_->audio_buf2, &player_->audio_buf2_size,
					data_size);
				//LOG_DEBUG(DEBUG_AUDIO_DECODE | DBG_TRACE, "%p, %d, %d", player_->audio_buf2, player_->audio_buf2_size, data_size);
				// ���ñ���ϵ��
				sonicSetSpeed(audio_speed_convert, player_->speed_);
				// д��ԭʼ����
				int out_ret = sonicWriteShortToStream(audio_speed_convert,
					(short *)player_->audio_buf,
					actual_out_samples);
				s_in_cout += actual_out_samples;
				if (out_ret)
				{
					// �����ж�ȡ����õ�����
					sonic_samples = sonicReadShortFromStream(audio_speed_convert,
						(int16_t *)player_->audio_buf2,
						numSamples);
				}
				s_out_cout += sonic_samples;
				//                    LOG_DEBUG(DEBUG_AUDIO_DECODE | DBG_TRACE, "sonic_samples = %d, in=%d, out=%d, aq_size = %d",
				//                           sonic_samples, s_in_cout, (int)(s_out_cout*speed),
				//                           player_->audioq->size);
				player_->audio_buf = player_->audio_buf2;
				player_->audio_buf_size = sonic_samples * 2 * 2;
#endif
				if (len >= (player_->audio_buf_size - player_->audio_buf_index))
				{
					copy_size = player_->audio_buf_size - player_->audio_buf_index;
				}
				else
				{
					copy_size = len;
				}
				memset(stream, 0, copy_size);
				/* ����������ʵ�����Ǹı�PCM���ݵķ�ֵ */
				SDL_MixAudioFormat(stream, (uint8_t *)player_->audio_buf + player_->audio_buf_index,
					AUDIO_S16SYS, copy_size, player_->audio_volume);
				player_->audio_buf_index += copy_size;
				len -= copy_size;
				stream += copy_size;

				player_->sampq->frame_queue_next();    // �ͷ�֡
				if (len <= 0)
					break;
			}
			else
			{
				memset(stream, 0, len);
				len = 0;
				return;
			}
			//                LOG_DEBUG(DEBUG_AUDIO_DECODE | DBG_TRACE, "audio_clock = %lf", player_->audio_clock);
		}
		else        // ��һ�ο�����ʣ������
		{
			if (len >= (player_->audio_buf_size - player_->audio_buf_index))
			{
				copy_size = player_->audio_buf_size - player_->audio_buf_index;
			}
			else
			{
				copy_size = len;
			}
			memset(stream, 0, copy_size);
			/* ����������ʵ�����Ǹı�PCM���ݵķ�ֵ */
			SDL_MixAudioFormat(stream, (uint8_t *)player_->audio_buf + player_->audio_buf_index,
				AUDIO_S16SYS, copy_size, player_->audio_volume);
			player_->audio_buf_index += copy_size;
			len -= copy_size;
			stream += copy_size;
		}
	}


// 	//set_clock(player_->audclk,  player_->audio_clock, 1);
// 	LogDebug( "audio clock = %lf",
// 		player_->audio_clock - temp_len / player_->audio_tgt->bytes_per_sec);
// 	player_->audclk->set_clock(player_->audio_clock - temp_len / player_->audio_tgt->bytes_per_sec, 
// 		player_->audio_clock_serial);
	DebugL << "audio clock =" << player_->audio_clock - temp_len / player_->audio_tgt->bytes_per_sec;
	if (!isnan(player_->audio_clock)) 
	{
		player_->audclk->set_clock_at(player_->audio_clock - temp_len / player_->audio_tgt->bytes_per_sec,
			player_->audio_clock_serial, player_->audio_callback_time / 1000000.0);
		player_->extclk->sync_clock_to_slave(player_->audclk);
	}
	//    LOG_DEBUG(DEBUG_AUDIO_DECODE | DBG_TRACE, "reamain = %d, %d, %d\n",
	//              player_->audio_buf_size - player_->audio_buf_index, player_->audio_buf_size, player_->audio_buf_index);
}
AudioOutput::AudioOutput(MediaPlayer *player)
	:player_(player)
{

}


AudioOutput::~AudioOutput()
{
}

int AudioOutput::audio_open(int wanted_channel_layout, int wanted_nb_channels, int wanted_sample_rate, AudioParams * audio_hw_params)
{
	SDL_AudioSpec wanted_spec; // ����Ҫ������
	SDL_AudioSpec spec;        //ʵ�ʷ�����������
	const char *env;
	static const int next_nb_channels[] = { 0, 0, 1, 6, 2, 6, 4, 6 };
	static const int next_sample_rates[] = { 0, 44100, 48000, 96000, 192000 };
	int next_sample_rate_idx = FF_ARRAY_ELEMS(next_sample_rates) - 1;
	if (SDL_Init(SDL_INIT_AUDIO) < 0)
	{
		ErrorL << "Could not initialize SDL: ", SDL_GetError();
		return -1;
	}
	env = SDL_getenv("SDL_AUDIO_CHANNELS");
	if (env)
	{
		wanted_nb_channels = atoi(env);
		wanted_channel_layout = av_get_default_channel_layout(wanted_nb_channels);
	}
	if (!wanted_channel_layout || wanted_nb_channels != av_get_channel_layout_nb_channels(wanted_channel_layout))
	{
		wanted_channel_layout = av_get_default_channel_layout(wanted_nb_channels);
		wanted_channel_layout &= ~AV_CH_LAYOUT_STEREO_DOWNMIX;
	}
	wanted_nb_channels = av_get_channel_layout_nb_channels(wanted_channel_layout);
	wanted_spec.channels = wanted_nb_channels;
	wanted_spec.freq = wanted_sample_rate;
	if (wanted_spec.freq <= 0 || wanted_spec.channels <= 0) {
		av_log(NULL, AV_LOG_ERROR, "Invalid sample rate or channel count!\n");
		return -1;
	}
	while (next_sample_rate_idx && next_sample_rates[next_sample_rate_idx] >= wanted_spec.freq)
		next_sample_rate_idx--;
	wanted_spec.format = AUDIO_S16SYS;
	wanted_spec.silence = 0;
	wanted_spec.samples = FFMAX(SDL_AUDIO_MIN_BUFFER_SIZE,
		2 << av_log2(wanted_spec.freq / SDL_AUDIO_MAX_CALLBACKS_PER_SEC));
	wanted_spec.callback = sdl_audio_callback;
	wanted_spec.userdata = player_;
	while (!(player_->audio_dev = SDL_OpenAudioDevice(NULL, 0, &wanted_spec, &spec,
		SDL_AUDIO_ALLOW_FREQUENCY_CHANGE | SDL_AUDIO_ALLOW_CHANNELS_CHANGE)))
	{
		av_log(NULL, AV_LOG_WARNING, "SDL_OpenAudio (%d channels, %d Hz): %s\n",
			wanted_spec.channels, wanted_spec.freq, SDL_GetError());
		wanted_spec.channels = next_nb_channels[FFMIN(7, wanted_spec.channels)];
		if (!wanted_spec.channels)
		{
			wanted_spec.freq = next_sample_rates[next_sample_rate_idx--];
			wanted_spec.channels = wanted_nb_channels;
			if (!wanted_spec.freq)
			{
				av_log(NULL, AV_LOG_ERROR,
					"No more combinations to try, audio open failed\n");
				return -1;
			}
		}
		wanted_channel_layout = av_get_default_channel_layout(wanted_spec.channels);
	}
	if (spec.format != AUDIO_S16SYS)
	{
		av_log(NULL, AV_LOG_ERROR,
			"SDL advised audio format %d is not supported!\n", spec.format);
		return -1;
	}
	if (spec.channels != wanted_spec.channels)
	{
		wanted_channel_layout = av_get_default_channel_layout(spec.channels);
		if (!wanted_channel_layout)
		{
			av_log(NULL, AV_LOG_ERROR,
				"SDL advised channel count %d is not supported!\n", spec.channels);
			return -1;
		}
	}

	audio_hw_params->fmt = AV_SAMPLE_FMT_S16;           // AV_SAMPLE_FMT_S16ֻ��SDL����ĸ�ʽ
	audio_hw_params->freq = spec.freq;
	audio_hw_params->channel_layout = wanted_channel_layout;
	audio_hw_params->channels = spec.channels;
	audio_hw_params->frame_size = av_samples_get_buffer_size(NULL, audio_hw_params->channels, 1, audio_hw_params->fmt, 1);
	audio_hw_params->bytes_per_sec = av_samples_get_buffer_size(NULL, audio_hw_params->channels, audio_hw_params->freq, audio_hw_params->fmt, 1);
	if (audio_hw_params->bytes_per_sec <= 0 || audio_hw_params->frame_size <= 0)
	{
		av_log(NULL, AV_LOG_ERROR, "av_samples_get_buffer_size failed\n");
		return -1;
	}

	SDL_PauseAudioDevice(player_->audio_dev, 0);     // �����������

												// ����Ϊ�����ʺ�������
	audio_speed_convert = sonicCreateStream(audio_hw_params->freq, audio_hw_params->channels);

	return spec.size;
}

void AudioOutput::audio_stop()
{
	SDL_PauseAudioDevice(player_->audio_dev, 1);     // ��ͣ�������
	SDL_CloseAudioDevice(player_->audio_dev);        // �ر���Ƶ�豸
}

void AudioOutput::update_volume(int sign, double step)
{
	player_->audio_volume += sign * step;
	if (player_->audio_volume < 0)
		player_->audio_volume = 0;
	if (player_->audio_volume > 100)
		player_->audio_volume = 100;
}
