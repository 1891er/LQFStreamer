#ifndef _PLAYER_INFO_H_
#define _PLAYER_INFO_H_

// �����ⲿ����ṩ�ص�����
typedef enum player_event
{
	PLAYER_EVENT_UNKNOWN,			// δ֪״̬
	PLAYER_EVENT_PLAYBACK_START,	// ����״̬
	PLAYER_EVENT_PLAYBACK_STOPED,	// ֹͣ״̬
	PLAYER_EVENT_PLAYBACK_FINISHED,	// �����������
	PLAYER_EVENT_PLAYLIST_FINISHED,	// Ŀ¼�������
	PLAYER_EVENT_PLAYBACK_PAUSE,	// ��ͣ״̬
	PLAYER_EVENT_PLAYBACK_RESUME,	// �ָ�����״̬
	PLAYER_EVENT_FRAME_BY_FRAME,	// ��֡����
	PLAYER_EVENT_UPDATE_PROGRESS,	// ���µ�ǰ����״̬���������ٷֱȣ���ǰʱ��
	PLAYER_EVENT_UPDATE_DURATION,	// ��ǰ�ļ�����ʱ��
} PlayerEvent;

// ��ǰ���ŵ�ʱ���Լ��ٷֱ�
typedef struct player_progress
{
	int hour;	// ��ǰʱ
	int minute;	// ��ǰ��
	int second;	// ��ǰ��
	float percentage;		// �ٷֱ�
}PlayerProgress;

// ����
typedef struct player_volume
{
	int volume;			// ����
	int percentage;		// �ٷֱ�
}PlayerVolume;

// �ļ�����
typedef struct player_duration
{
	int hour;	// ʱ
	int minute;	// ��
	int second;	// ��
	int64_t total_seconds;
}PlayerDuration;
#endif
