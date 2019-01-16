#include "QTMediaPlayer.h"
#include <QtWidgets/QApplication>
#include <iostream>
#include "logger.h"
using namespace std;
using namespace toolkit;
int main(int argc, char *argv[])
{
	toolkit::Logger::Instance().add(std::make_shared<FileChannel>("FileChannel", "test.log"));
	QApplication a(argc, argv);
	QTMediaPlayer w;
	w.show();
	return a.exec();
}


class TestLog
{
public:
	template<typename T>
	TestLog(const T &t) {
		_ss << t;
	};
	~TestLog() {};

	//ͨ������Ԫ���������Դ�ӡ�Զ�����������
	friend ostream& operator<<(ostream& out, const TestLog& obj) {
		return out << obj._ss.str();
	}
private:
	stringstream _ss;
};

int main2() 
{
	{
	//��ʼ����־ϵͳ
	toolkit::Logger::Instance().add(std::make_shared<ConsoleChannel>());
	toolkit::Logger::Instance().add(std::make_shared<FileChannel>("FileChannel", "test.log"));
	toolkit::Logger::Instance().add(std::make_shared<FileChannel>("FileChannel2", "test2.log"));
	toolkit::Logger::Instance().setWriter(std::make_shared<AsyncLogWriter>());

	//ostream֧�ֵ��������Ͷ�֧��,����ͨ����Ԫ�ķ�ʽ��ӡ�Զ�����������
	TraceL << "object int" << TestLog((int)1) << endl;
	DebugL << "object short:" << TestLog((short)2) << endl;
//	toolkit::InfoL << "object float:" << TestLog((float)3.12345678) << endl;
	}
	system("pause");
	return 0;
}