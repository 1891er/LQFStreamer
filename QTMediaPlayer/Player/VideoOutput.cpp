#include "VideoOutput.h"
#include "MediaPlayer.h"
#include "FrameQueue.h"
#include "Clock.h"
#include "PacketQueue.h"
#include "VideoWidget.h"
#include "MediaPlayer.h"
#include "Decoder.h"
#include "logger.h"
using namespace toolkit;
#ifdef __cplusplus
extern "C"
{
#include "libavutil/time.h"
#include "SDL.h"
}
#endif

#define REFRESH_RATE  0.01
int framedrop = 2;

static int sRefreshThread(void *arg)
{
	VideoOutput *video_output = (VideoOutput *)arg;
	return video_output->refreshThread();	
}
/**
* @brief ����������ʾ֡��Ҫ�������ŵ�ʱ�䣬ͨ������·��ص�������֡�����
* @param delay �ò���ʵ�ʴ��ݵ��ǵ�ǰ��ʾ֡��׼��Ҫ����֡�ļ����
* @param is
* @return ���ص�ǰ��ʾ֡Ҫ�������ŵ�ʱ�䡣ΪʲôҪ�������ص�delay��Ϊʲô��֧��ʹ�����ڼ��֡ʱ�䣿
*/
static double compute_target_delay(double delay, MediaPlayer *is)
{
	double sync_threshold, diff = 0;

	/* update delay to follow master synchronisation source */
	/* ������ֵ�ǰ��ClockԴ����video������㵱ǰ��Ƶʱ������ʱ�ӵĲ�ֵ */
	if (Clock::get_master_sync_type(is) != AV_SYNC_VIDEO_MASTER) {
		/* if video is slave, we try to correct big delays by
		duplicating or deleting a frame */
		diff = is->vidclk->get_clock() - Clock::get_master_clock(is);

		/* skip or repeat frame. We take into account the
		delay to compute the threshold. I still don't know
		if it is the best guess */
		sync_threshold = FFMAX(AV_SYNC_THRESHOLD_MIN, FFMIN(AV_SYNC_THRESHOLD_MAX, delay));
		if (!isnan(diff) && fabs(diff) < is->max_frame_duration) {
			if (diff <= -sync_threshold)
				delay = FFMAX(0, delay + diff);
			else if (diff >= sync_threshold && delay > AV_SYNC_FRAMEDUP_THRESHOLD)
				delay = delay + diff;
			else if (diff >= sync_threshold)
				delay = 2 * delay;
		}
	}

	av_log(NULL, AV_LOG_TRACE, "video: delay=%0.3f A-V=%f\n",
		delay, -diff);

	return delay;
}

static void update_video_pts(MediaPlayer *is, double pts, int64_t pos, int serial) 
{
	/* update current video pts */
	is->vidclk->set_clock(pts, serial);
	is->extclk->sync_clock_to_slave(is->vidclk);
}

VideoOutput::VideoOutput(MediaPlayer *player)
	:player_(player)
{
}


VideoOutput::~VideoOutput()
{
}

bool VideoOutput::video_open()
{
	abort_request = false;
	refresh_thread_tid_ = SDL_CreateThread(sRefreshThread, "refresh_thread", this);
	if (!refresh_thread_tid_)
	{
		av_log(NULL, AV_LOG_FATAL, "SDL_CreateThread(): %s\n", SDL_GetError());
		return false;
	}
	return true;
}

bool VideoOutput::video_stop()
{
	abort_request = true;
	if (refresh_thread_tid_)
		SDL_WaitThread(refresh_thread_tid_, NULL);	// �ȴ����ݶ�ȡ�߳��˳�

	if (is_pic_init_)
	{
		avpicture_free(&picture);
		is_pic_init_ = false;
	}
	return true;
}

int VideoOutput::refreshThread()
{
	double remaining_time = 0.0; /* ���ߵȴ���remaining_time�ļ�����video_refresh�� */

	while (true)
	{
		if (abort_request)
		{
			break;
		}
		if (remaining_time > 0.0)
		{
			av_usleep((int64_t)(remaining_time * 1000000.0));    /* ת��ΪΪ΢�� */
		}

		remaining_time = REFRESH_RATE;      // �������������ÿ���������ʱ�䣬��������ȱʡ��0.01�룬����10ms

											/* ����video_refresh������ˢ����Ƶ֡ */
		video_refresh(&remaining_time);                   /* ����ͣ����ǿ��ˢ��ʱ���� */
	}
	return 0;
}
double VideoOutput::vp_duration(Frame *vp, Frame *nextvp) 
{
	if (vp->serial == nextvp->serial) 
	{
		double duration = nextvp->pts - vp->pts;
		if (isnan(duration) || duration <= 0 || duration > player_->max_frame_duration)
			return vp->duration / player_->speed_;	 /* �쳣ʱ��֡ʱ��Ϊ��׼(1��/֡��) */
		else
			return duration / player_->speed_;
	}
	else {
		return 0.0;
	}
}
void VideoOutput::video_refresh(double * remaining_time)
{
	MediaPlayer *is = player_;
	double time;

	Frame *sp, *sp2;

	if (!is->paused && Clock::get_master_sync_type(player_) == AV_SYNC_EXTERNAL_CLOCK && is->realtime)
		Clock::check_external_clock_speed(player_);

	if (is->video_st) {
	retry:
		if (is->pictq->frame_queue_nb_remaining() == 0) 
		{
			// nothing to do, no picture to display in the queue
// 			DebugL << "no frame";
		}
		else 
		{
			double last_duration, duration, delay;
			Frame *vp, *lastvp;

			/* dequeue the picture */
			lastvp = is->pictq->frame_queue_peek_last();
			vp = is->pictq->frame_queue_peek();

			if (vp->serial != is->videoq->serial_) 
			{
				is->pictq->frame_queue_next();
				DebugL << "retry vpts =" << vp->pts;
				goto retry;
			}

			if (lastvp->serial != vp->serial)
				is->frame_timer = av_gettime_relative() / 1000000.0;

			if (is->paused)
			{
				goto display;
			}
			/* compute nominal last_duration */
			/*
			* last_duration ���ݵ�ǰ֡����һ֡��pts���������һ֡����ʾ��Ҫ������ʱ��
			* �����video master��ģʽ����ֻ��Ҫ�����ÿ֡�������ŵ�ʱ�䣬��֡�ʲ��ż���
			*/
			last_duration = vp_duration(lastvp, vp);
			/* ����compute_target_delay�����������������ʾ��֡Ҫ�������ŵ�ʱ�� */
			// �����video clockΪmaster����delayֱ�ӵ���last_duration��
			// �������audio/external clockΪmaster������Ҫ�ȶ���ʱ�ӵ�����ǰ֡Ҫ�������ŵ�ʱ�䡣
			delay = compute_target_delay(last_duration, is);
// 			DebugL << "delay = " << delay << ", vpts = " << vp->pts;
			time = av_gettime_relative() / 1000000.0;
			if (time < is->frame_timer + delay) {
				*remaining_time = FFMIN(is->frame_timer + delay - time, *remaining_time);
				goto display;
			}

			is->frame_timer += delay;
			if (delay > 0 && time - is->frame_timer > AV_SYNC_THRESHOLD_MAX)
				is->frame_timer = time;

			SDL_LockMutex(is->pictq->mutex_);
			if (!isnan(vp->pts))
				update_video_pts(is, vp->pts / player_->speed_, vp->pos, vp->serial);
			SDL_UnlockMutex(is->pictq->mutex_);

			if (is->pictq->frame_queue_nb_remaining() > 1) 
			{
				Frame *nextvp = is->pictq->frame_queue_peek_next();
				duration = vp_duration(vp, nextvp);
				if (!is->step && (framedrop>0 || (framedrop && Clock::get_master_sync_type(is) != AV_SYNC_VIDEO_MASTER)) && time > is->frame_timer + duration) {
					is->frame_drops_late++;
					is->pictq->frame_queue_next();
					goto retry;
				}
			}

			is->pictq->frame_queue_next();
			is->force_refresh = 1;	 /* ˵����Ҫˢ����Ƶ֡ */

			if (is->step && !is->paused)
				is->stream_toggle_pause();
		}
	display:
		/* display picture */
		if (is->force_refresh &&  is->pictq->rindex_shown_)
		{
			video_display();
		}
	}
	is->force_refresh = 0;
}

void VideoOutput::video_display()
{
	if (player_->video_st)
		video_image_display();
}

// �Ƚ�ռ��cpu�ĺ�������
void VideoOutput::video_image_display()
{
	Frame *vp;
	Frame *sp = NULL;
	SDL_Rect rect;
	vp = player_->pictq->frame_queue_peek_last();
	DebugL << "video_display pts = " << vp->pts;
	AVFrame		*frame = vp->frame;
	width_ = frame->width;
	height_ = frame->height;
	if (is_first_frame_)
	{
		is_first_frame_ = false;
		player_->video_dev_->InitShader(width_, height_);
	}
	
	convert_ctx = sws_getCachedContext(convert_ctx, width_, height_, 
		 player_->viddec->avctx_->pix_fmt, width_, height_, AV_PIX_FMT_YUV420P, 
		SWS_FAST_BILINEAR, NULL, NULL, NULL);

	if (!is_pic_init_)
	{
		int ret = avpicture_alloc(&picture, AV_PIX_FMT_YUV420P, width_, height_);
		is_pic_init_ = true;
	}

	int rr = sws_scale(convert_ctx, frame->data, frame->linesize,
		0, frame->height, picture.data, picture.linesize);

	player_->video_dev_->UpdateData(picture.data);
}
