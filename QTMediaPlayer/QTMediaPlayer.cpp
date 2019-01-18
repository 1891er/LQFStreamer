#include "QTMediaPlayer.h"

#include <QDebug>
#include <QMessageBox>
#include <QFileDialog>

#include "PlayerStyleSheet.h"
#include "MediaPlayer.h"


static int mediaPlayerCallback(PlayerEvent e, void *param, void *user)
{
	QTMediaPlayer *qmedia_player = (QTMediaPlayer *)user;
	qmedia_player->HandlePlayerEvent(e, param);
	return 0;
}
QTMediaPlayer::QTMediaPlayer(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	initUI();

	player_ = new MediaPlayer(ui.openGLWidget);
	player_->RegisterEventCallback(mediaPlayerCallback, this);
	initPlayer();
}

void QTMediaPlayer::slotStop()
{
	qDebug() << "slotStop";
	player_->StopCommand();
}
void  QTMediaPlayer::slotPlayOrPause()
{
	qDebug() << "slotPlayOrPause";
	if (player_->IsStoped())		// ��ʼ����
	{
		if (!url_.empty())
		{
			player_->PlayCommand(url_.c_str());
			ui.playButton->setStyleSheet(PlayerStyleSheet::ToolBarPauseBtn);
		}
	}
	else
	{
		player_->TogglePauseCommand();
		if (player_->IsPaused())
		{
			// ��ͣ״̬����ʾ���Ƿ�����ʾ�û�����ָ�����
			ui.playButton->setStyleSheet(PlayerStyleSheet::ToolBarPlayBtn);
		}
		else
		{
			ui.playButton->setStyleSheet(PlayerStyleSheet::ToolBarPauseBtn);
		}
	}
}

void QTMediaPlayer::setProgressValue(int pos)
{
	// ���ͱ���  pos / progress_max_value_
}

void QTMediaPlayer::slotChangePlayButtonIcon(int value)
{
	if (1 == value)
	{
		ui.playButton->setStyleSheet(PlayerStyleSheet::ToolBarPlayBtn);
	}
	else if(2 == value)
	{
		ui.playButton->setStyleSheet(PlayerStyleSheet::ToolBarPlayBtn);
		ui.progressSlider->setValue(progress_max_value_);
	}
	else 
	{
		ui.playButton->setStyleSheet(PlayerStyleSheet::ToolBarPauseBtn);
	}
}

void QTMediaPlayer::slotChangeTime(QString time)
{
	ui.timeLabel->setText(time);
}

void QTMediaPlayer::slotChangeProgressSlider(int value)
{
	ui.progressSlider->setValue(value);
}

void QTMediaPlayer::slotSeekProgress(int pos)
{
//	player_->SeekCommand(1.0 * pos /progress_max_value_);
}

void QTMediaPlayer::slotSeekProgress()
{
	int pos = ui.progressSlider->value();
	player_->SeekCommand(1.0 * pos / progress_max_value_);
	is_progress_press_ = false;
}

void QTMediaPlayer::slotProgressPress()
{
	is_progress_press_ = true;
}

void QTMediaPlayer::slotOpenFile()
{
	// ����ֻ�Ǽ򵥵Ĵ�һ���ļ�
	QString file_url_ = QFileDialog::getOpenFileName(
		this, u8"ѡ��Ҫ���ŵ��ļ�",
		"/",//��ʼĿ¼
		u8"��Ƶ�ļ� (*.flv *.rmvb *.avi *.MP4);; �����ļ� (*.*);; ");
	if (!file_url_.isEmpty())
	{
		file_url_.replace("/", "\\");
		url_ = file_url_.toStdString();
	}
}

void QTMediaPlayer::slotSlow()
{
	player_->DecelerateCommand();
}

void QTMediaPlayer::slotFast()
{
	player_->AccelerateCommand();
}

void QTMediaPlayer::slotVolume(int pos)
{
	float percent = 1.0*pos / volume_max_value_;
	player_->AdjustVolumeCommand(percent);
}

void QTMediaPlayer::HandlePlayerEvent(PlayerEvent e, void * param)
{
	switch (e)
	{
	case PLAYER_EVENT_PLAYBACK_START:
		// ֪ͨ�޸Ĳ�����ͣ��ť����ͣ
		handlePlay();
		break;
	case PLAYER_EVENT_PLAYBACK_STOPED:
		handleStop();
		break;
	case PLAYER_EVENT_UPDATE_DURATION:	// ���µ�ǰ�ļ�����ʱ��
		handleDuration(param);
		break;
	case PLAYER_EVENT_UPDATE_PROGRESS:	// ���µ�ǰ�Ĳ��Ž���
		// (1)���½�����
		// (2)����ʱ�����ʾ
		handleProgress(param);
		break;
	case PLAYER_EVENT_PLAYBACK_FINISHED:
		handlePlayFinish();
		break;
	default:
		break;
	}
}

void QTMediaPlayer::closeEvent(QCloseEvent * event)
{
	QMessageBox::StandardButton button;       
	button = QMessageBox::question(this, tr(u8"�˳�����"),
		QString(tr(u8"ȷ���˳�����")), QMessageBox::Yes | QMessageBox::No);
	if (button == QMessageBox::No) 
	{
		event->ignore(); // �����˳��źţ������������       
	}       
	else if(button==QMessageBox::Yes)       
	{           
		event->accept(); // �����˳��źţ������˳� 
		if (!player_->IsStoped())
		{
			player_->StopCommand();
			while (true)
			{
				if (player_->IsStoped())
					break;
				Sleep(30);
			}
			delete player_;
			player_ = nullptr;
		}
	}  
}

void QTMediaPlayer::initUI()
{
	ui.playButton->setStyleSheet(PlayerStyleSheet::ToolBarPlayBtn);
	ui.previousButton->setStyleSheet(PlayerStyleSheet::ToolBarPreviousButton);
	ui.stopButton->setStyleSheet(PlayerStyleSheet::ToolBarStopBtn);
	ui.nextButton->setStyleSheet(PlayerStyleSheet::ToolBarNextButton);

	// ȫ����ť
	ui.fullScreenButton->setStyleSheet(PlayerStyleSheet::ToolBarFullScreenButton);
	// ��չ����
	ui.settingButton->setStyleSheet(PlayerStyleSheet::ToolBarSettingButton);

	// �򿪲����б�
	ui.listButton->setStyleSheet(PlayerStyleSheet::ToolBarListButton);
	// ����ѭ��ģʽ
	ui.modeButton->setStyleSheet(PlayerStyleSheet::ToolBarModeButtonRepeatOff);
	//
	ui.randomButton->setStyleSheet(PlayerStyleSheet::ToolBarRandomButton);

	//���ò����ٶ�
	ui.showListButton->setStyleSheet(PlayerStyleSheet::ToolBarSlowerButton);
	ui.fasterButton->setStyleSheet(PlayerStyleSheet::ToolBarFasterButton);

	// ����������
	ui.volumeButton->setStyleSheet(PlayerStyleSheet::ToolBarVolumeButton);

	// ���ý�������ֵ
	ui.progressSlider->setMaximum(progress_max_value_);
	ui.progressSlider->setSingleStep(1);
	// ��ʼ��ʱ��
	total_time_.sprintf("%02d:%02d:%02d", 0, 0, 0);
	cur_time_.sprintf("%02d:%02d:%02d", 0, 0, 0);
	// �󶨰�ť��ʾ�Ĳ�ͬ
	connect(this, SIGNAL(signalChangePlayButtonIcon(int)), this, SLOT(slotChangePlayButtonIcon(int)));
	// ������ʱ��
	connect(this, SIGNAL(signalChangeTotalDuration(QString)), this, SLOT(slotChangeTime(QString)));
	// ���½�����
	connect(this, SIGNAL(signalChangeProgressSlider(int)), this, SLOT(slotChangeProgressSlider(int)));

	// ���ļ�
	connect(ui.actionOpenFIle, SIGNAL(triggered()), this, SLOT(slotOpenFile()));

	// ��������
	ui.volumeSlider->setMaximum(volume_max_value_);
}

void QTMediaPlayer::initPlayer()
{
	ui.volumeSlider->setValue(volume_max_value_ * volume_default_ratio_);
	player_->AdjustVolumeCommand(volume_default_ratio_);
}

void QTMediaPlayer::handlePlay()
{
	emit signalChangePlayButtonIcon(0);	// 0 ������, 1��ͣ
}

void QTMediaPlayer::handleStop()
{
	emit signalChangePlayButtonIcon(1);	// 0 ������, 1��ͣ
}

void QTMediaPlayer::handleDuration(void * param)
{
	PlayerDuration *duration = (PlayerDuration *)param;
	QString time;
	total_time_.sprintf("%02d:%02d:%02d", duration->hour, duration->minute, duration->second);
	time = cur_time_ + "\/" + total_time_; //.sprintf("%s//%s", cur_time_.constData(), total_time_.constData());
	emit signalChangeTotalDuration(time);
}

void QTMediaPlayer::handleProgress(void * param)
{
	PlayerProgress *progress = (PlayerProgress *)param;
	QString time;
	cur_time_.sprintf("%02d:%02d:%02d", progress->hour, progress->minute, progress->second);
	time = cur_time_ + "\/" + total_time_;
	emit signalChangeTotalDuration(time);
	int pos = progress->percentage * progress_max_value_;
	if(!is_progress_press_)		// ����������ʱ���ܸ��½�����
	emit signalChangeProgressSlider(pos);
}

void QTMediaPlayer::handlePlayFinish()
{
	emit signalChangePlayButtonIcon(2);	// 0 ������, 1��ͣ , 2�������
}
