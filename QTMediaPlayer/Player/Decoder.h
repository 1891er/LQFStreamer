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
}
#endif

class PacketQueue; 
class FrameQueue;
class Decoder
{
public:
	Decoder();
	~Decoder();
	int decoder_init(AVCodecContext *avctx, PacketQueue *queue, SDL_cond *empty_queue_cond);
	int decoder_start(int(*fn)(void *), void *arg);
	void decoder_abort(FrameQueue *fq);
	void decoder_destroy(void);
public:

	AVPacket pkt_;
	PacketQueue	*queue_ = NULL;         // ���ݰ�����
	AVCodecContext	*avctx_ = NULL;     // ������������
	int		pkt_serial = -1;         // ������
	int		finished = 0;           // =0�����������ڹ���״̬��=��0�����������ڿ���״̬
	int		packet_pending = 0;     // =0�������������쳣״̬����Ҫ�������ý�������=1����������������״̬
	SDL_cond	*empty_queue_cond_ = NULL;  // ��鵽packet���п�ʱ���� signal����read_thread��ȡ����
	int64_t		start_pts = 0;          // ��ʼ��ʱ��stream��start time
	AVRational	start_pts_tb;       // ��ʼ��ʱ��stream��time_base
	int64_t		next_pts = 0;           // ��¼���һ�ν�����frame��pts����������Ĳ���֡û����Ч��ptsʱ��ʹ��next_pts��������
	AVRational	next_pts_tb;        // next_pts�ĵ�λ
	SDL_Thread	*decoder_tid = NULL;       // �߳̾��
};

