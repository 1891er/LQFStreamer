#include "QTStreamer.h"
#include <QtWidgets/QApplication>

int main2(int argc, char *argv[])
{
	QApplication a(argc, argv);
	QTStreamer w;
	w.show();
	return a.exec();
}

extern int test_server(int argc, char *argv[]);

int main(int argc, char *argv[])
{
	int ret = test_server(argc, argv);
	//cout << "ret = " << ret<< endl;
	system("pause");
	return 0;
}
#include <iostream>
#include "Util/logger.h"

using namespace std;
using namespace toolkit;

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

int main3() {
	//��ʼ����־ϵͳ
	Logger::Instance().add(std::make_shared<ConsoleChannel>());
	Logger::Instance().add(std::make_shared<FileChannel>("FileChannel", "test.log"));
	Logger::Instance().setWriter(std::make_shared<AsyncLogWriter>());

	//ostream֧�ֵ��������Ͷ�֧��,����ͨ����Ԫ�ķ�ʽ��ӡ�Զ�����������
	TraceL << "object int" << TestLog((int)1) << endl;
	DebugL << "object short:" << TestLog((short)2) << endl;
	InfoL << "object float:" << TestLog((float)3.12345678) << endl;
	WarnL << "object double:" << TestLog((double)4.12345678901234567) << endl;
	ErrorL << "object void *:" << TestLog((void *)0x12345678) << endl;
	ErrorL << "object string:" << TestLog("test string") << endl;


	//����ostreamԭ��֧�ֵ���������
	TraceL << "int" << (int)1 << endl;
	DebugL << "short:" << (short)2 << endl;
	InfoL << "float:" << (float)3.12345678 << endl;
	WarnL << "double:" << (double)4.12345678901234567 << endl;
	ErrorL << "void *:" << (void *)0x12345678 << endl;
	//����RAII��ԭ���˴�����Ҫ���� endl��Ҳ���ڱ�����ջpopʱ��ӡlog
	ErrorL << "without endl!";
	system("pause");
	return 0;
}
