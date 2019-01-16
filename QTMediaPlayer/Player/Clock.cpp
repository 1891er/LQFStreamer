#include "Clock.h"

#include "MediaPlayer.h"
#include "PacketQueue.h"

Clock::Clock(MediaPlayer *player)
	:player_(player)
{
}


Clock::~Clock()
{
}
/**
* ��ȡ����ʵ������:���һ֡��pts ���� �Ӵ������һ֡��ʼ�����ڵ�ʱ��,����ο�set_clock_at ��get_clock�Ĵ���
* pts_drift_=���һ֡��pts-�Ӵ������һ֡ʱ��
* clock=pts_drift_+���ڵ�ʱ��
* get_clock(&player_->vidclk) ==player_->vidclk.pts, av_gettime_relative() / 1000000.0 -player_->vidclk.last_updated  +player_->vidclk.pts
*/
double Clock::get_clock()
{
	if (*queue_serial_ != serial_)
	{
		return(NAN);
	}
	if (paused_)
	{
		return pts_;
	}
	/* �����ǰ�����ڲ���״̬���򷵻ص�ʱ��Ϊ���µ�pts + ����pts֮�����ŵ�ʱ�� */
	double time = av_gettime_relative() / 1000000.0;

	return(pts_drift_ + time - (time - last_updated_) * (1.0 - speed_));
}

void Clock::set_clock_at(double pts, int serial, double time)
{
	pts_ = pts;                      /* ��ǰ֡��pts */
	pts_drift_ = pts_ - time;        /* ��ǰ֡pts��ϵͳʱ��Ĳ�ֵ������������������ߵĲ�ֵӦ���ǱȽϹ̶��ģ���Ϊ���߶�����ʱ��Ϊ��׼������������ */
	last_updated_ = time;
	serial_ = serial;                     // ��Դ��packet serial
}

void Clock::set_clock(double pts, int serial)
{
	double time = av_gettime_relative() / 1000000.0;
	set_clock_at(pts, serial, time); /* �����ⲿʱ�� */
}

void Clock::set_clock_speed(double speed)
{
	set_clock(get_clock(), serial_);
	speed_ = speed;
}

int Clock::get_master_sync_type(MediaPlayer *player)
{
	if (player->av_sync_type == AV_SYNC_VIDEO_MASTER) {
		if (player->video_st)
			return AV_SYNC_VIDEO_MASTER;
		else
			return AV_SYNC_AUDIO_MASTER;	 /* ���û����Ƶ�ɷ���ʹ�� audio master */
	}
	else if (player->av_sync_type == AV_SYNC_AUDIO_MASTER) {
		if (player->audio_st)
			return AV_SYNC_AUDIO_MASTER;
		else
			return AV_SYNC_EXTERNAL_CLOCK;	 /* û����Ƶ��ʱ���Ǿ����ⲿʱ�� */
	}
	else {
		return AV_SYNC_EXTERNAL_CLOCK;
	}
}

double Clock::get_master_clock(MediaPlayer *player)
{
	double val;

	switch (get_master_sync_type(player)) 
	{
	case AV_SYNC_VIDEO_MASTER:
		val = player->vidclk->get_clock();
		break;
	case AV_SYNC_AUDIO_MASTER:
		val = player->audclk->get_clock();
		break;
	default:
		val = player->extclk->get_clock();
		break;
	}
	return val;
}

void Clock::set_master_sync_type(MediaPlayer *player, int sync_type)
{
	player->av_sync_type = sync_type;
}

void Clock::check_external_clock_speed(MediaPlayer *player)
{
	if (player->video_stream >= 0 && player->videoq->nb_packets_ <= EXTERNAL_CLOCK_MIN_FRAMES ||
		player->audio_stream >= 0 && player->audioq->nb_packets_ <= EXTERNAL_CLOCK_MIN_FRAMES)
	{
		player->extclk->set_clock_speed(FFMAX(EXTERNAL_CLOCK_SPEED_MIN, player->extclk->speed_ - EXTERNAL_CLOCK_SPEED_STEP));
	}
	else if ((player->video_stream < 0 || player->videoq->nb_packets_ > EXTERNAL_CLOCK_MAX_FRAMES) &&
		(player->audio_stream < 0 || player->audioq->nb_packets_ > EXTERNAL_CLOCK_MAX_FRAMES)) {
		player->extclk->set_clock_speed(FFMIN(EXTERNAL_CLOCK_SPEED_MAX, player->extclk->speed_ + EXTERNAL_CLOCK_SPEED_STEP));
	}
	else {
		double speed = player->extclk->speed_;
		if (speed != 1.0)
			player->extclk->set_clock_speed(speed + EXTERNAL_CLOCK_SPEED_STEP * (1.0 - speed) / fabs(1.0 - speed));
	}
}

void Clock::sync_clock_to_slave(Clock * slave)
{
	double	clock = get_clock();                                                               /* �ⲿʱ�ӵ�ʱ�� */
	double	slave_clock = slave->get_clock();                                                           /* slaveʱ�ӵ�ʱ�� */
	if (!isnan(slave_clock) && (isnan(clock) || fabs(clock - slave_clock) > AV_NOSYNC_THRESHOLD))   /* �������no sync */
	{
		set_clock(slave_clock, slave->serial_);
	}
}

void Clock::init_clock(int * queue_serial, const char * name)
{
	speed_ = 1.0;
	if (queue_serial)
		queue_serial_ = queue_serial;
	else
		queue_serial_ = &(serial_);
	paused_ = 0;
	if (name)
	{
		strcpy(name_, name);
	}
	set_clock( NAN, -1);
}

