#ifndef PACKET_QUEUE_H
#define PACKET_QUEUE_H

#ifdef __cplusplus
extern "C"
{
#include "libavformat/avformat.h"
#include "SDL.h"
}
#endif

// ���������ݻ������
#define MAX_QUEUE_SIZE (5*1024 * 1024)
#define MIN_FRAMES 25
#define EXTERNAL_CLOCK_MIN_FRAMES 2
#define EXTERNAL_CLOCK_MAX_FRAMES 10

typedef struct MyAVPacketList 
{
	AVPacket		pkt;
	struct MyAVPacketList	*next;          // ָ����һ��Ԫ��
	int			serial;
} MyAVPacketList;

class PacketQueue
{
public:
	PacketQueue();
	~PacketQueue();
	int packet_queue_init(const char *name);
	void packet_queue_start();
	int packet_queue_put(AVPacket *pkt);
	/**
	* @brief ��ȡ���ݰ�
	* @param q
	* @param pkt
	* @param block �Ƿ�����ʽ��ȡ���ݰ�
	* @return ����״̬
	*  @retval -1  �����˳�����
	*  @retval 0   û�����ݰ��ɻ�ȡ
	*  @retval 1   ��ȡ�����ݰ�
	*/
	int packet_queue_get(AVPacket *pkt, int block, int *serial);
	void packet_queue_flush(void);
	int packet_queue_put_nullpacket(int stream_index);
	void packet_queue_abort(void);
	void packet_queue_destroy(void);
	int get_abort_request() const
	{
		return abort_request_;
	}
	int get_serial() const
	{
		return serial_;
	}
	int get_size() const
	{
		return size_;
	}
	int get_nb_packets() const
	{
		return nb_packets_;
	}
	
	static AVPacket *GetFlushPacket();
	
public:
	int packet_queue_put_private(AVPacket * pkt);


	MyAVPacketList *first_pkt_ = nullptr;
	MyAVPacketList *last_pkt_ = nullptr;
	int nb_packets_ = 0;             // ������
	int64_t		duration_ = 0;       // ��������Ԫ�ص����ݲ��ų���ʱ��
	int		size_ = 0;                   // ���д洢��ý�������ܴ�С
	int		serial_ = 0;
	SDL_mutex *mutex_ = nullptr;
	SDL_cond *cond_ = nullptr;
	int abort_request_ = 0;          // �Ƿ��˳��ȴ�
	char		name_[20];       // �洢���������Է���debug
};
#endif
