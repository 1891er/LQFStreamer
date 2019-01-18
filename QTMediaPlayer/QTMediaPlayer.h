#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_QTMediaPlayer.h"

#include <QCloseEvent>

#include "PlayerInfo.h"
class MediaPlayer;
class QTMediaPlayer : public QMainWindow
{
	Q_OBJECT

public:
	QTMediaPlayer(QWidget *parent = Q_NULLPTR);
	void HandlePlayerEvent(PlayerEvent e, void *param);
protected:       
	void closeEvent(QCloseEvent *event);
private:
	void initUI();
	void initPlayer();
	void handlePlay();
	void handleStop();
	void handleDuration(void *param);
	void handleProgress(void *param);
	void handlePlayFinish();
signals:
	void signalChangePlayButtonIcon(int);
	void signalChangeTotalDuration(QString);
	void signalChangeProgressSlider(int);
private slots:
	void slotStop();
	void slotPlayOrPause();
	void setProgressValue(int);
	void slotChangePlayButtonIcon(int);
	void slotChangeTime(QString);
	void slotChangeProgressSlider(int);
	void slotSeekProgress(int);
	void slotSeekProgress();
	void slotProgressPress();
	void slotOpenFile();		//���ļ�
	void slotSlow();			// ����
	void slotFast();			// ���
	void slotVolume(int);
private:
	Ui::QTMediaPlayerClass ui;

	MediaPlayer *player_ = nullptr;

	int progress_max_value_ = 1000;
	int volume_max_value_ = 20;
	QString total_time_;	// ������ʱ�䳤��
	QString cur_time_;		// ��ǰʱ��
	std::string url_ = "shahai45.mp4";			// ���ļ�����
	bool is_progress_press_ = false;
	float volume_default_ratio_ = 0.5; // ��Χ��0~1.0 ��Ӧ��С~�������
};
