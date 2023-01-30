#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QFileDialog> //打开文件对话框
#include <QDebug> //qinfo调试
#include <QDir> //文件目录操作


#include <QMediaPlayer> //播放
#include <QAudioOutput> //输出
#include <QVideoWidget>

//QStringList MediaList; //全局变量播放列表

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowTitle("Dotplayer 0.1");//窗口名称

    mediaplayer = new QMediaPlayer(this);
    audiooutput = new QAudioOutput(this);
    videowidget = new QVideoWidget(ui->LB_playwidget);//这里把父对象指定给LB窗口

    mediaplayer->setAudioOutput(audiooutput); //把视频播放链接到音频输出
    mediaplayer->setVideoOutput(videowidget); //链接视频输出
    videowidget->resize(ui->LB_playwidget->size());//设置窗口大小


    //获取当前媒体长度，自定义信号
    connect(mediaplayer,&QMediaPlayer::durationChanged,this,[=](qint64 duration){ //返回值为毫秒
        ui->LB_total->setText(QString("%1:%2").arg(duration/60000,2,10,QChar('0')).arg(duration/1000%60,2,10,QChar('0')));//格式化字符串
        ui->SL_progress->setRange(0,duration);//设置进度条范围
    });

    //获取媒体播放进度
    connect(mediaplayer,&QMediaPlayer::positionChanged,this,[=](qint64 position){ //2为位数，10为进制
        ui->LB_current->setText(QString("%1:%2").arg(position/60000,2,10,QChar('0')).arg(position/1000%60,2,10,QChar('0')));
        ui->SL_progress->setValue(position);//设置进度条位置
    });

    //拖动进度条，改变播放进度
    connect(ui->SL_progress,&QSlider::sliderMoved,mediaplayer,&QMediaPlayer::setPosition);

    //音量调节
    ui->SL_sound->setRange(0,100);
    ui->SL_sound->setValue(100); //设置默认是满音量吧，感觉这个调整音量没什么用
    connect(ui->SL_sound,&QSlider::sliderMoved,this,[=](float soundDB){
        audiooutput->setVolume(soundDB / 100);
        qInfo()<<soundDB / 100;
    });

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_B_AddFile_clicked() //添加播放列表按键
{
    auto path =  QFileDialog::getExistingDirectory(this,"选择媒体文件目录","D:\\CloudMusic");//文件对话框选择媒体
    //qInfo() << path; //应用程序输出内容
    // 获取path路径中的mp3、wav、flac文件
    QDir dir(path);
    MediaList = dir.entryList(QStringList()<<"*.mp3"<<"*.wav"<<"*.flac"<<"*.mkv"<<"*.mp4"); //设置过滤选项
    ui->L_MediaList->addItems(MediaList); //添加到播放列表list中展示

    for (auto file: MediaList){
        playlist.append(QUrl::fromLocalFile(path+"/"+file)); //获取完整路径保存在playlist中
        //qInfo()<<playlist;
    }

    ui->L_MediaList->setCurrentRow(0);//默认选中第一行，下标从0开始


}


void MainWindow::on_B_play_clicked()
{
    if(playlist.empty()){
        return; //没有添加播放内容的时候是放不了的
    }
    switch (mediaplayer->playbackState()) { //获取播放状态
    case QMediaPlayer::PlaybackState::StoppedState:
    { //没有播放，播放当前选中媒体
        int row = ui->L_MediaList->currentRow();
        mediaplayer->setSource(playlist[row]);
        mediaplayer->play();

        //ui->B_play->text(); 这里加一个切换
        break;
    }
    case QMediaPlayer::PlaybackState::PausedState:
        //暂停播放，按键继续播放
        mediaplayer->play();
        break;
    case QMediaPlayer::PlaybackState::PlayingState:
        //正在播放，暂停播放
        mediaplayer->pause();
        break;
    }
}


void MainWindow::on_B_prev_clicked() //播放上一个文件
{
    int row = ui->L_MediaList->currentRow();
    if(row>0){
        row--;
    }
    else {
        row = playlist.size()-1; //反向约瑟夫环怎么不行了 妈的
    }
    ui->L_MediaList->setCurrentRow(row);
    mediaplayer->setSource(playlist[row]);
}


void MainWindow::on_B_next_clicked()  //播放下一个文件
{
    int row = ui->L_MediaList->currentRow();
    row = (row+1) % playlist.size(); //约瑟夫环实现循环选择
    ui->L_MediaList->setCurrentRow(row);
    mediaplayer->setSource(playlist[row]);
    //mediaplayer->play();
}


void MainWindow::on_B_stop_clicked()
{
    mediaplayer->stop();
}




void MainWindow::on_pushButton_5_clicked()//添加播放列表
{
    MediaList.append(QFileDialog::getOpenFileNames(this,"选择要添加到播放列表的文件","D:\\CloudMusic","allfiles(*.*)")); //直接获取全部路径了
    //playlist.append(QUrl::fromLocalFile(QFileDialog::getOpenFileNames(this,"选择要添加到播放列表的文件","D:\\CloudMusic","allfiles(*.*)")));
    ui->L_MediaList->addItems(MediaList);
}

