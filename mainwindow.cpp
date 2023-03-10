#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QFileDialog> //打开文件对话框
#include <QDebug> //qinfo调试
#include <QDir> //文件目录操作
#include <qfileinfo.h>


#include <QMediaPlayer> //播放
#include <QAudioOutput> //输出
#include <QVideoWidget>

//QStringList MediaList; //全局变量播放列表

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowTitle("Dotplayer 1.0");//窗口名称

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
    }

    ui->L_MediaList->setCurrentRow(0);//默认选中第一行，下标从0开始


}

void MainWindow::on_B_play_clicked()
{

    if(playlist.empty() && !Cloud){
        return; //没有添加播放内容的时候是放不了的
    }
    switch (mediaplayer->playbackState()) { //获取播放状态
    case QMediaPlayer::PlaybackState::StoppedState:
    { //没有播放，播放当前选中媒体
        int row;
        if(!Cloud){

            row = ui->L_MediaList->currentRow();
            mediaplayer->setSource(playlist[row]);
        }
        else{
            //row = ui->L_CloudList->currentRow();
            qInfo()<<"正在缓冲播放媒体";
            mediaplayer->setSource(playlist_cloud[0]);
        }

        mediaplayer->play();

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

void MainWindow::on_pushButton_5_clicked()//添加文件到播放列表，现在支持多文件选择添加了
{
    auto add_FilePath = QFileDialog::getOpenFileNames(this,"选择要添加到播放列表的文件","D:\\CloudMusic","allfiles(*.*)"); //直接获取全部路径了

    for (int i = 0;i < add_FilePath.length();i++){
    auto fi = QFileInfo(add_FilePath[i]);
    QString add_FileName = fi.fileName();

    playlist.append(QUrl::fromLocalFile(add_FilePath[i]));
    MediaList.append(add_FileName); //MediaList 是放文件名的 playlist 是放文件路径的
    }
    ui->L_MediaList->addItems(MediaList);
}

void MainWindow::on_pushButton_11_clicked() // 删除播放列表中的选项
{
    int row = ui->L_MediaList->currentRow(); //获取选中的行
    MediaList.removeAt(row);
    playlist.removeAt(row);
    ui->L_MediaList->clear();
    ui->L_MediaList->addItems(MediaList);

}

void MainWindow::on_tabWidget_2_tabBarClicked(int index) //选中这个框框之后读取电脑音乐文件夹中的东西
{
    if(index == 1){ //选中了此电脑界面
        QString user_path = "C:/Users";
        QDir dir(user_path);//这边的播放还要修改前面的东西，因为
        QStringList User_Medialist = dir.entryList(QStringList()<<"*.mp3"<<"*.wav"<<"*.flac"<<"*.mkv"<<"*.mp4"); //设置过滤选项

        playlist.clear();

        for (auto file:User_Medialist){
            playlist.append(QUrl::fromLocalFile(user_path+"/"+file)); //获取完整路径保存在playlist中
        }
        qInfo()<<playlist;
        if(User_Medialist.length()!=0){
            ui->L_UserFile->addItems(User_Medialist);
        }
        else if(flag){
            ui->L_UserFile->addItem("在C:/Users路径下找不到可以播放的媒体");
            flag = false;
        }
    }
    else{
        ui->L_UserFile->clear();
        flag = true;
    }
}

void MainWindow::on_B_Top_clicked() //把选中播放列表的播放对象置顶
{
    int target = ui->L_MediaList->currentRow();
    QString target_item = MediaList[target];
    QUrl target_url = playlist[target];

    if(target>0){

        for(int i=target;i>0;i--){
            MediaList[i]=MediaList[i-1]; //后置
            playlist[i]=playlist[i-1];
        }

        MediaList[0] = target_item;
        playlist[0] = target_url;

        ui->L_MediaList->clear();
        ui->L_MediaList->addItems(MediaList);
        ui->L_MediaList->setCurrentRow(0);

    }
}

void MainWindow::on_B_Bottom_clicked() //把选中播放列表的播放对象置底
{
    int target = ui->L_MediaList->currentRow();
    QString target_item = MediaList[target];
    QUrl target_url = playlist[target];

    if(target<MediaList.length()-1){

        for(int i=target;i<MediaList.length()-1;i++){

            MediaList[i]=MediaList[i+1]; //前置
            playlist[i]=playlist[i+1];
        }

        MediaList[MediaList.length()-1] = target_item;
        playlist[MediaList.length()-1] = target_url;

        ui->L_MediaList->clear();
        ui->L_MediaList->addItems(MediaList); //这边之后可以用函数来优化一下
        ui->L_MediaList->setCurrentRow(MediaList.length()-1);

    }
}

void MainWindow::on_B_Up_clicked() //把选中的播放对象向上一格
{
    //这边就直接交换位置了
    int target = ui->L_MediaList->currentRow();
    QString target_item = MediaList[target];
    QUrl target_url = playlist[target];

    if(target>0){

        playlist[target]=playlist[target-1];
        playlist[target-1]=target_url;

        MediaList[target]=MediaList[target-1];
        MediaList[target-1]=target_item;

        ui->L_MediaList->clear();
        ui->L_MediaList->addItems(MediaList);
        ui->L_MediaList->setCurrentRow(target-1);

    }
}

void MainWindow::on_B_Down_clicked() //把选中的播放对象向下一格
{
    int target = ui->L_MediaList->currentRow();
    QString target_item = MediaList[target];
    QUrl target_url = playlist[target];

    if(target<MediaList.length()-1){

        playlist[target]=playlist[target+1];
        playlist[target+1]=target_url;

        MediaList[target]=MediaList[target+1];
        MediaList[target+1]=target_item;

        ui->L_MediaList->clear();
        ui->L_MediaList->addItems(MediaList);
        ui->L_MediaList->setCurrentRow(target+1);

    }

}

void MainWindow::on_pushButton_12_clicked() //对播放列表内容按照码值大小进行排序（没啥用草）
{
    QString t_name;
    QUrl t_url;

    for (int i=0;i<MediaList.length()-1;++i){ //冒泡

        for(int j=MediaList.length()-1;j>i;--j){

            if(MediaList[j]<MediaList[j-1]){

                t_name=MediaList[j];
                MediaList[j]=MediaList[j-1];
                MediaList[j-1]=t_name;

                t_url=playlist[j];
                playlist[j]=playlist[j-1];
                playlist[j-1]=t_url;
            }
        }
    }
    ui->L_MediaList->clear();
    ui->L_MediaList->addItems(MediaList);
}

void MainWindow::on_tabWidget_tabBarClicked(int index)// 点击到浏览器界面 获取云端的播放列表
{
    if(index == 0){ //判断点击浏览器
        MediaList.clear();
        ui->L_CloudList->clear();
        playlist_cloud.append(QUrl("http://thecircus.top/wp-content/uploads/2022/11/JMJM.mp3"));
        MediaList.append("JMJM.mp3");

        ui->L_CloudList->addItems(MediaList);
        MediaList.clear();
        Cloud = true;
        qInfo()<<"进入互联网模式";
        qInfo()<<playlist_cloud;
    }
    else{
        Cloud = false;
        playlist_cloud.clear();
    }
}

void MainWindow::on_B_HighSpeed_clicked() //设置二倍速播放
{
    mediaplayer->setPlaybackRate(2);
}

void MainWindow::on_B_LowSpedd_clicked()  //设置半速播放
{
    mediaplayer->setPlaybackRate(0.5);
}

void MainWindow::on_B_NSpeed_clicked()  //设置常速播放
{
    mediaplayer->setPlaybackRate(1);
}

void MainWindow::on_B_Color_clicked() //修改背景图片
{
    if(!style_set){

        this->setStyleSheet("background-image: url(:/BG.png);");
        style_set = !style_set;

    }
    else{

        this->setStyleSheet("");
        style_set = !style_set;

    }
}

