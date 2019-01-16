#include "Demuxer.h"
#include "logger.h"
#include "MediaPlayer.h"

using namespace toolkit;
static int is_realtime(AVFormatContext *s)
{
	if (!strcmp(s->iformat->name, "rtp")
		|| !strcmp(s->iformat->name, "rtsp")
		|| !strcmp(s->iformat->name, "sdp"))
	{
		return 1;
	}
	if (s->pb
		&& (!strncmp(s->url, "rtp:", 4) || !strncmp(s->url, "udp:", 4)))
	{
		return 1;
	}
	return 0;
}

static char err_buf[128] = { 0 };
static char* av_get_err(int errnum)
{
	av_strerror(errnum, err_buf, 128);
	return err_buf;
}
Demuxer::Demuxer(MediaPlayer *player)
{
	player_ = player;
	ic = avformat_alloc_context();
}

int Demuxer::Open(const char * url)
{
	int ret;
	ret = avformat_open_input(&ic, url, NULL, NULL);
	if (ret != 0)
	{
		ErrorL << "[error] avformat_open_input: " <<  av_get_err(ret);
		goto failed;
	}

	// ��ȡý���ļ��Ĳ������ݰ��Ի�ȡ������Ϣ
	ret = avformat_find_stream_info(ic, NULL);
	if (ret < 0)
	{
		ErrorL <<  "[error] avformat_find_stream_info: " <<  av_get_err(ret);
		goto failed;
	}
	player_->max_frame_duration = (ic->iformat->flags & AVFMT_TS_DISCONT) ? 10.0 : 3600.0;
	/* �Ƿ�Ϊʵʱ��ý�� */
	player_->realtime = is_realtime(ic);

	// ���ҳ��ĸ�������video/audio/subtitles
	player_->audio_stream = -1;
	player_->video_stream = -1;
	player_->video_stream = av_find_best_stream(ic, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
	if (player_->video_stream == -1)
	{
		DebugL  << "Didn't find a video stream";
	}

	player_->audio_stream = av_find_best_stream(ic, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
	if (player_->audio_stream == -1)
	{
		DebugL  << "Didn't find a audio stream";
	}

	player_->format_context_ = ic;

	return ret;
failed:
	// ����رճ���
	if (ic)
		avformat_close_input(&ic);
	return ret;
}


Demuxer::~Demuxer()
{
}
