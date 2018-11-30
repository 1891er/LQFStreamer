#ifndef PACKET_QUEUE_H
#define PACKET_QUEUE_H

#ifdef __cplusplus
extern "C"
{
#include "libavformat/avformat.h"
#include "SDL.h"
}
#endif

typedef struct MyAVPacketList {
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
		return abort_request;
	}
	int get_serial() const
	{
		return serial;
	}
	int get_size() const
	{
		return size;
	}
	int get_nb_packets() const
	{
		return nb_packets;
	}
	
	static AVPacket *GetFlushPacket();
	
private:
	int packet_queue_put_private(AVPacket * pkt);


	MyAVPacketList *first_pkt;
	MyAVPacketList *last_pkt;
	int nb_packets;             // ������
	int64_t		duration;       // ��������Ԫ�ص����ݲ��ų���ʱ��
	int size;                   // ���д洢��ý�������ܴ�С
	int		serial;
	SDL_mutex *mutex;
	SDL_cond *cond;
	int abort_request;          // �Ƿ��˳��ȴ�
	char		name_[20];       // �洢���������Է���debug
};
#endif
