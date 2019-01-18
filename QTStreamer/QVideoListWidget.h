#pragma once
#include <QtWidgets/QWidget>

#include "ui_VideoListWidget.h"
// ��Ƶ�б��п�ȳ�ʼ��ϵ�� 200+120+120+140+120=700
const int	k_column_num = 5;
// ��Ƶ��
const float  k_column_video_name_ratio = (1.0 * 200 / 700);
// ʱ��
const float  k_column_video_duration_ratio = (1.0 * 120 / 700);
// ��С
const float  k_column_video_size_ratio = (1.0 * 120 / 700);
// ����
const float  k_column_video_date_ratio = (1.0 * 140 / 700);
// ����
const float  k_column_video_more_ratio = (1.0 * 120 / 700);
class QVideoListWidget : public QWidget
{
	Q_OBJECT

public:
	QVideoListWidget(QWidget *parent = Q_NULLPTR);

private:
	Ui::Widget ui;

	void initTableWidget();
};
