#pragma once
#include <iostream>

#ifdef __cplusplus
extern "C"
{
#include "libavformat/avformat.h"
#include "SDL.h"
}
#endif

#include "Ringbuffer.h"
#include "PlayerInfo.h"

// ����������
typedef enum player_conmand
{
	PLAYER_CMD_PLAY = 0,
	PLAYER_CMD_STOP,
	PLAYER_CMD_TOGGLE_PAUSE,
	PLAYER_CMD_TOGGLE_MUTE,
	PLAYER_CMD_SEEK,
	PLAYER_CMD_FAST_FORWARD,	// ���ast forward
	PLAYER_CMD_FAST_BACK,		// ����
	PLAYER_CMD_STEP_TO_NEXT_FRAME,
	PLAYER_CMD_TOGGLE_FULL_SCREEN, 
	PLAYER_CMD_UP_VOLUME,			// ��������
	PLAYER_CMD_DOWN_VOLUME,			// ��������
	PLAYER_CMD_CYCLE_CHNANEL,
	PLAYER_CMD_CYCLE_AUDIO_TRACK,	// �л�����
	PLAYER_CMD_CYCLE_VIDEO_TRACK,	// �л���Ƶ��
	PLAYER_CMD_CYCLE_SUBTITLE_TRACK,	// �л���Ļ
}PlayerConmand;

typedef enum player_state
{	
	PLAYER_ST_READY_PALY,	// ׼�������У����²�������ʱ
	PLAYER_ST_PLAYING,	// ���ڲ���		
	PLAYER_ST_PAUSE,	// ��ͣ״̬
	PLAYER_ST_STOPED,	// ֹͣ״̬
}PlayerState;

typedef struct  palyer_play_cmd
{
	std::string url_;				// ��������
	int64_t start_time_;			// ��ʵʱ��
	void setURL(const char *url)
	{
		url_.clear();
		url_.append(url);
	}
	void setStartTime(int64_t start_time)
	{
		start_time_ = start_time;
	}
}PlayerPlayCmd;

// Ŀǰû��ʵ�ʵĲ������յ�����ֱ�ӽ�������
typedef struct  palyer_stop_cmd
{
	bool stop_;
}PlayerStopCmd;

typedef struct  palyer_seek_cmd
{
	double percent;
}PlayerSeekCmd;

typedef struct player_cmd_
{
	void *parm;				// �������ڲ�MediaPlayer����
	PlayerConmand cmd;
}PlayerConmandMessage;


/* Minimum SDL audio buffer size, in samples. */
#define SDL_AUDIO_MIN_BUFFER_SIZE 512
/* Calculate actual buffer size keeping in mind not cause too frequent audio callbacks */
#define SDL_AUDIO_MAX_CALLBACKS_PER_SEC 30
typedef struct AudioParams
{
	int freq;                   // ֡��
	int channels;               // ͨ����
	int64_t channel_layout;     // ͨ������ ����2.0������/2.1������+������/5.1��ͥӰԺ�����������ȵ�
	enum AVSampleFormat fmt;    // ������ʽ
	int frame_size;             // һ��������Ԫռ���ֽ�������һ��������Ԫ��������ͨ���Ĳ����㣩
	int bytes_per_sec;          // һ�������ĵ��ֽ�����
} AudioParams;


// ǰ������
class PacketQueue;
class Decoder;
class FrameQueue;
class Demuxer;
class Clock;
class AudioOutput;
class VideoOutput;
class VideoWidget;

// e�¼���param�¼�������user�û��Լ���ָ��
typedef int (*pEventCallBack) (PlayerEvent e, void *param, void *user);

class MediaPlayer
{
public:
	MediaPlayer(VideoWidget *video_dev);
	// ���ش�״̬��ʱ����Ϣ�����Ž��ȵȵ�
	void RegisterEventCallback(pEventCallBack ecb, void *user);
	~MediaPlayer();
	bool PushCommandMessage(PlayerConmandMessage *cmdMsg);			
	int read_thread();
	int event_loop();		// ��Ӧ�ⲿ�¼�
	
	/**
	* url ����url
	* start_time ��ʼʱ�䣬Ĭ��Ϊ0
	*/
	bool PlayCommand(const char *url, int64_t start_time = 0);
	bool TogglePauseCommand();		// ��ͣ/�ָ�����
	// ֹͣ����
	bool StopCommand();
	// �϶�����, ֻ���ݰٷֱ�(���ݽ������ı�ֵ)
	bool SeekCommand(double seek_percent);
	bool AccelerateCommand();	// ���ٲ���
	bool DecelerateCommand();	// ���ٲ���
	// ratio��Χ[0~1.0]
	bool AdjustVolumeCommand(float ratio);
	float GetCurrentPlaySpeed();
	PlayerState GetPlayerState();
	bool IsStoped();
	bool IsPaused();

	void stream_toggle_pause();

	void UpdateDuration(AVFormatContext *ic);
	void UpdateProgress();
private:
	void checkPlayFinish();							// ����Ƿ��Ѿ��������
	void toggle_pause();
	bool stream_open(const char *url);			// ��ʼ����Դ
	void stream_close();		// �ͷ���Դ
	int  stream_component_open(int stream_index);
	void stream_component_close(int stream_index);
	void stream_seek(int64_t pos, int64_t rel, int seek_by_bytes);
	RingBuffer<PlayerConmandMessage *> *cmd_queue_;

public:
	PlayerState player_state_ = PLAYER_ST_STOPED;	// ��ʼΪֹͣ״̬

	pEventCallBack event_cb_;
	void* event_data_;
	int64_t audio_callback_time = 0;
	Demuxer *demxer_ = nullptr;
	AVFormatContext *format_context_ = nullptr;
	int		abort_request = 0;      // =1ʱ�����˳�����
	int		realtime = 0;           // =1Ϊʵʱ��
	int		read_pause_return = 0;

	// ��Ƶ���
	AVStream    *video_st = nullptr;
	PacketQueue *videoq = nullptr;
	int         video_stream = -1;   // ��Ƶ������
	Decoder     *viddec = nullptr;        // ��Ƶ������,ԭ����vid_codec_ctx�ŵ�viddec
	int         frame_rate = 25;     // ֡��
	FrameQueue	*pictq = nullptr;         // ͼ��֡����

								// ��Ƶ���
	AVStream    *audio_st = nullptr;
	PacketQueue *audioq = nullptr;
	int         audio_stream = -1;   // ��Ƶ������
	Decoder     *auddec = nullptr;        // ��Ƶ��������ԭ����aud_codec_ctx�ŵ�auddec
	int         sample_rate = 44100;    // ������
	FrameQueue	*sampq = nullptr;         // PCM֡����
								// ����������
	SDL_AudioDeviceID audio_dev;
	AudioParams *audio_tgt = nullptr;         // SDL������Ƶ��Ҫ�ĸ�ʽ
	AudioParams *audio_src = nullptr;         // �������Ƶ֡�ĸ�ʽ  ��audio_tgt��audio_src��ͬ����Ҫ�ز���
	struct SwrContext *swr_ctx = nullptr;     // ��Ƶ�ز���

	uint8_t *audio_buf = nullptr;             // ָ����������ݣ���ֻ��һ��ָ�룬ʵ�ʴ洢������������audio_buf1
	uint32_t audio_buf_size = 0;        // audio_bufָ������֡�����ݳ��ȣ����ֽ�Ϊ��λ
	uint32_t audio_buf_index = 0;       // audio_buf_index��ǰ��ȡ��λ�ã����ܳ���audio_buf_size

	uint8_t *audio_buf1 = 0;            // �洢��������Ƶ����֡����̬���룬���������㳤��ʱ�������ͷ��ٷ���
	uint32_t audio_buf1_size = 0;       // �洢�����ݳ��ȣ����ֽ�Ϊ��λ

	uint8_t *audio_buf2 = nullptr;            // �洢���ٺ����Ƶ����֡
	uint32_t audio_buf2_size = 0;       // �洢�����ݳ��ȣ����ֽ�Ϊ��λ

									// ��ʾ���
	int		force_refresh  = 0;      // =1ʱ��Ҫˢ�»��棬��������ˢ�»������˼
								// SDL �ⲿ����Ҫ����ʾ��أ�
	int default_width = 640;
	int default_height = 480;
	int screen_width = 0;
	int screen_height = 0;
	int	width, height, xleft, ytop;
	bool is_display_open = false;

	// ��Ƶ���
	AudioOutput *audio_output_ = nullptr;
	// ��Ƶ���
	VideoOutput *video_output_ = nullptr;

	// ����Ƶͬ�����
	Clock	*audclk = nullptr;            // ��Ƶ����ʱ��
	Clock	*vidclk = nullptr;             // ��Ƶʱ��
	Clock	*extclk = nullptr;             // �ⲿʱ��
	int     audio_clock_serial = -1; // serial
	double  audio_clock = 0;        // �����ڽ�PTS��ֵ��������ֵ���ø�ʱ��
	double  video_clock = 0;        // �����ڽ�PTS��ֵ�����ں�audio pts���ȶ�
	double	frame_timer = 0;        // ����video�������ʱ��
	float speed_ = 1.0;                // �����ٶȿ��ƣ�Ĭ��Ϊ1.0������Ƶ����Ƶ����Ч
	int av_sync_type = 0;                       // ����Ƶͬ������
											/* maximum duration of a frame - above this, we consider the jump a timestamp discontinuity */
											// ���ڸ�ֵ����Ϊ֡���������м�����֡�����
	double		max_frame_duration;
	int			frame_drops_early;      // drop֡���
	int			frame_drops_late;

	// �ļ����
	char filename[1024];
	bool eof = false;        // �Ƿ��Ѿ���ȡ����

	SDL_Thread   *refresh_tid = nullptr;
	SDL_Thread   *read_tid = nullptr;

	bool abort_app = false;
	SDL_Thread   *event_loop_tid = nullptr;

	// �������
	bool quit = false;       // = tureʱ�����˳�
	bool paused = false;      // = ture��ͣ, false h�ָ�����
	int	last_paused;        // �ݴ桰��ͣ��/�����š�״̬
							// ��� ���� ��ת
	int		seek_req = 0;       // seek�����־
							// AVSEEK_FLAG_BACKWARD AVSEEK_FLAG_BYTE AVSEEK_FLAG_ANY  AVSEEK_FLAG_FRAME
	int		seek_flags = 0;     // seek���ͣ����ֽڻ��ǰ�ʱ��
							// ע��seek_pos��seek_rel�����λ������ǰ��ֽ�seek��λΪ�ֽڣ������ʱ����λΪ us΢��
	int64_t	seek_pos;       // ���󲥷ŵ�λ��
	int64_t	seek_rel;       // ����˵��seek��ƫ��λ��
							// ��֡����
	int	step = 0;
	// ����
	int			audio_volume;                   /* ���� */
	int			muted;                      // =1������=0������

	SDL_cond	*continue_read_thread = nullptr;
	VideoWidget *video_dev_ = nullptr;

	// �����
	PlayerPlayCmd play_cmd_;
	PlayerStopCmd stop_cmd_;
	PlayerSeekCmd seek_cmd_;

	PlayerDuration player_duration_;
	PlayerProgress player_progress_ ;
};

