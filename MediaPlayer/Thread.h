#pragma once
#include <thread>
#include <Windows.h>
using namespace std;

class Thread
{
public:
	Thread();
	virtual ~Thread();
	virtual bool Start();		// �����߳�
	virtual bool Stop();		// ֹͣ�߳�
	virtual bool IsRunning();		// �Ƿ�������״̬
	
	virtual void Run(void);
	protected:
	thread *thread_ = nullptr;
	bool running_ = false;	// �߳��Ƿ�������̬
	bool requet_abort_ = false;

};

