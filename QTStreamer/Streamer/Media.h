#ifndef __MEDIA_H__
#define __MEDIA_H__


#include <memory>
#include <cstdint>

namespace LQF
{

#define VIDEO_PACKET_MASK	0x00ff
#define AUDIO_PACKET_MASK	0xff00

#define VIDEO_PACKET		0x00ff
#define VIDEO_PACKET_I		0x0001		// I֡ 
#define VIDEO_PACKET_P		0x0002		// P֡ 
#define VIDEO_PACKET_B		0x0004		// B֡
#define VIDEO_PACKET_SEI	0x0008		// SEI֡

#define AUDIO_PACKET		0xff00		// ��Ƶ���Ҳ���	

typedef struct __AVPacket
{
	__AVPacket()
	{
		size = 0;
		type = 0;
		timestamp = 0;
	}

	__AVPacket(uint32_t size)
		:buffer(new char[size])
	{
		this->size = size;
		type = 0;
		timestamp = 0;
	}

	std::shared_ptr<char> buffer; /* ֡���� */
	uint32_t size;				  /* ֡��С */
	uint32_t type;				  /* ֡���� */
	uint32_t timestamp;		  	  /* ʱ��� */
} AVPacket;

#define MAX_MEDIA_CHANNEL 2

typedef  enum __MediaChannel_Id
{
	channel_0,
	channel_1
} MediaChannelId;

typedef uint32_t MediaSessionId;

// NALU��Ԫ    
typedef struct _NaluUnit
{
	int type;
	int size;
	uint8_t *data;
}NaluUnit;
}


typedef enum _StreamId
{
	stream_0 = 0,
	stream_1
} StreamId;



#endif // !__MEDIA_H__