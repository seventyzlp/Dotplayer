#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "qmediaplayer.h"
#include <QMainWindow>
#include <QUrl>
#include <QVideoWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:

      void on_B_AddFile_clicked();

      void on_B_play_clicked();

      void on_B_prev_clicked();

      void on_B_next_clicked();

      void on_B_stop_clicked();

      void on_pushButton_5_clicked();

      void on_pushButton_11_clicked();

      void on_tabWidget_2_tabBarClicked(int index);

      void on_B_Top_clicked();

      void on_B_Bottom_clicked();

      void on_B_Up_clicked();

      void on_B_Down_clicked();

      void on_pushButton_12_clicked();

      void on_tabWidget_tabBarClicked(int index);

      void on_B_HighSpeed_clicked();

      void on_B_LowSpedd_clicked();

      void on_B_NSpeed_clicked();

      void on_B_Color_clicked();

private:
    Ui::MainWindow *ui;
    QList<QUrl> playlist; //存储媒体路径
    QAudioOutput* audiooutput;
    QMediaPlayer* mediaplayer;
    QVideoWidget* videowidget;
    QStringList MediaList;
    bool flag = true;
    bool Cloud = false;
    bool style_set = false;
    QList<QUrl> playlist_cloud;
};
#endif // MAINWINDOW_H
