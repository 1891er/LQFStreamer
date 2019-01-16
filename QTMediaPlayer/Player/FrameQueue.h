#ifndef FRAME_QUEUE_H
#define FRAME_QUEUE_H

#ifdef __cplusplus
extern "C"
{
#include "libavformat/avformat.h"
#include "SDL.h"
}
#endif

#define VIDEO_PICTURE_QUEUE_SIZE	3       // ͼ��֡��������
#define SUBPICTURE_QUEUE_SIZE		16      // ��Ļ֡��������
#define SAMPLE_QUEUE_SIZE           9       // ����֡��������
#define FRAME_QUEUE_SIZE		FFMAX( SAMPLE_QUEUE_SIZE, FFMAX( VIDEO_PICTURE_QUEUE_SIZE, SUBPICTURE_QUEUE_SIZE ) )

typedef struct Frame
{
	AVFrame		*frame;         // ָ������֡
	AVSubtitle	sub;            // ������Ļ
	int		serial;             // ֡���У���seek�Ĳ���ʱserial��仯
	double		pts;            // ʱ�������λΪ��
	double		duration;       // ��֡����ʱ�䣬��λΪ��
	int64_t		pos;            // ��֡�������ļ��е��ֽ�λ��
	int		width;              // ͼ����
	int		height;             // ͼ��߶�
	int		format;             // ����ͼ��Ϊ(enum AVPixelFormat)������������Ϊ(enum AVSampleFormat)
	AVRational	sar;            // ͼ��Ŀ�߱ȣ����δ֪��δָ����Ϊ0/1
	int		uploaded;           // ������¼��֡�Ƿ��Ѿ���ʾ����
	int		flip_v;             // =1����ת180�� = 0����������
} Frame;

class PacketQueue;

class FrameQueue
{
public:
	FrameQueue();
	~FrameQueue();

	// ���г�ʼ��
	int frame_queue_init(PacketQueue *pktq, int max_size, int keep_last = 0);
	// ���ٶ���
	void frame_queue_destory(void);
	// �ͷ��źŻ�������
	void frame_queue_signal(void);
	// �Ӷ���ȡ����ǰ��Ҫ��ʾ��һ֡
	Frame *frame_queue_peek(void);
	// �Ӷ���ȡ����ǰ��Ҫ��ʾ����һ֡
	Frame *frame_queue_peek_next(void);
	// �Ӷ���ȡ����������ŵ�һ֡
	Frame *frame_queue_peek_last(void);
	// �������Ƿ��д�ռ�
	Frame *frame_queue_peek_writable(void);
	// �������Ƿ������ݿɶ�
	Frame *frame_queue_peek_readable(void);
	// ������������һ֡��frame_queue_peek_writableֻ�ǻ�ȡһ����д��λ�ã���������û���޸�
	void frame_queue_push(void);
	// ��������һ֡���ݣ���֡������ȡ��֮֡��Ĳ�����������rindex_showΪ0��ʱ��ʹ���Ϊ1�����������һ֡
	int frame_queue_next(void);
	// ʣ��֡��
	int frame_queue_nb_remaining(void);
	// �����ʾ��һ֡����ý���ļ��е�ƫ��λ��
	int64_t frame_queue_last_pos(void);

public:
	void frame_queue_unref_item(Frame * vp);

	Frame	queue_[FRAME_QUEUE_SIZE];        // FRAME_QUEUE_SIZE  ���size, ����̫��ʱ��ռ�ô������ڴ棬��Ҫע���ֵ������
	int		rindex_ = 0;                         // ��ʾѭ�����еĽ�β��
	int		windex_ = 0;                         // ��ʾѭ�����еĿ�ʼ��
	int		size_ = 0;                           // ��ǰ���е���Ч֡��
	int		max_size_ = 0;                       // ��ǰ��������֡������
	int		keep_last_ = 0;                      // = 1˵��Ҫ�ڶ������汣�����һ֡�����ݲ��ͷţ�ֻ�����ٶ��е�ʱ��Ž��������ͷ�
	int		rindex_shown_ = 0;                   // ��ʼ��Ϊ0�����keep_last=1ʹ��
	SDL_mutex	*mutex_ = NULL;                     // ������
	SDL_cond	*cond_ = NULL;                      // ��������
	PacketQueue	*pktq_ = NULL;                      // ���ݰ��������
};

#endif