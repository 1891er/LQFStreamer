#pragma once

#ifdef __cplusplus
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "SDL.h"
#include "libavutil/imgutils.h"
#include "libavutil/time.h"
#include "libavutil/common.h"
}
#endif

/* no AV sync correction is done if below the minimum AV sync threshold */
#define AV_SYNC_THRESHOLD_MIN 0.01              /* 40ms */
/* AV sync correction is done if above the maximum AV sync threshold */
#define AV_SYNC_THRESHOLD_MAX 0.1               /* 100ms */
/* If a frame duration is longer than this, it will not be duplicated to compensate AV sync */
#define AV_SYNC_FRAMEDUP_THRESHOLD 0.1
/* no AV correction is done if too big error */
#define AV_NOSYNC_THRESHOLD 10.0                /* AV���ز�ͬ��ʱ */

/* external clock speed adjustment constants for realtime sources based on buffer fullness */
#define EXTERNAL_CLOCK_SPEED_MIN  0.900
#define EXTERNAL_CLOCK_SPEED_MAX  1.010
#define EXTERNAL_CLOCK_SPEED_STEP 0.001

/**
*����Ƶͬ����ʽ��ȱʡ����ƵΪ��׼
*/
enum {
	AV_SYNC_AUDIO_MASTER,                   // ����ƵΪ��׼
	AV_SYNC_VIDEO_MASTER,                   // ����ƵΪ��׼
	AV_SYNC_EXTERNAL_CLOCK,                 // ���ⲿʱ��Ϊ��׼��synchronize to an external clock */
};

class MediaPlayer;
class Clock
{
public:
	Clock(MediaPlayer *player);
	virtual ~Clock();

	double get_clock();
	void set_clock_at(double pts, int serial, double time);
	void set_clock(double pts, int serial);
	void init_clock(int *queue_serial, const char *name);
	void set_clock_speed(double speed);
	static int get_master_sync_type(MediaPlayer *player);
	static double get_master_clock(MediaPlayer *player);
	static void set_master_sync_type(MediaPlayer *player, int sync_type);
	static void check_external_clock_speed(MediaPlayer *player);
	void sync_clock_to_slave(Clock *slave);


	double	pts_;                        // clock base ʱ�ӻ���
	int	paused_;                         // = 1 ˵������ͣ״̬
	double	last_updated_;               // ���һ�θ��µ�ϵͳʱ��
	double	speed_;                      // �����speed��Ҫ�����ⲿʱ��ͬ��
										// ���ʱ�ӵĽ��⣬��Ҫ��͸��
	double	pts_drift_;                  // clock base minus time at which we updated the clock����ǰpts�뵱ǰϵͳʱ�ӵĲ�ֵ
										// serial������
	int	serial_;                         // clock is based on a packet with this serial
										// *queue_serial�����ã�ʵ��ָ�����packet queue ��serial
	int	*queue_serial_;                  // pointer to the current packet queue serial, used for obsolete clock detection
	char	name_[20];                   // �洢ʱ�������Է���debug */

private:
	MediaPlayer *player_;
};

