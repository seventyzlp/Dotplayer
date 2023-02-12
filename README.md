# Dotplayer1.0
基于QT实现的媒体播放器，以QMediaPlayer为播放器内核。实现视频的播放、暂停、变速，声音音量调节等基本操作，以及使用UI界面对播放列表的调整、排序，设置界面主题。

在UI界面的设计上主要参考了PotPlayer，所以笔者把这个项目称为Dotplayer。但QT中内置的MediaPlayer性能完全比不上PotPlayer，甚至有些mkv文件是打不开的(笑)。

## 一、设计思路
音频播放器->视频播放器->有很多功能的多媒体播放器
由于QTcreator是基于控件事件的控制方式，模块化设计方便了程序设计，只要想好这个控件对应的是什么功能就行，和MFC、VB的思路很像，比imgui没有图形化的操控界面要方便些。

在设计相关模块的时候其实并没有直接使用很多算法的思维，唯一的双重循环是对歌单的冒泡排序，更多的其实是使用现成的库函数。
所以在后文的模块分析中将主要讲述实现功能的思路，而不是具体代码的实现。

## 二、模块分析

### 1、添加·读取多媒体文件内容

``` cpp
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
```
在不想思考变量类型的时候，直接用auto是个很偷懒的办法，但是会比较损失内存空间。这里调用了QFileDialog，会弹出文件选择对话框，getExistingDirectory是获取路径内的所有文件路径。

QDir相当于是一个漏斗，设置entryList的筛选项为文件后缀名之后，就会对path中的路径进行筛选，剩余能够被播放的多媒体文件会被保存到MediaList中，但这只是文件名，并不能直接播放。

想要被QMediaPlayer播放，需要完整路径，所以把path和Medialist中的文件名组合，形成完整的路径存储于playlist中，用于播放。

这样操作起到的是批量添加文件夹中的可播放媒体的功能，比一个一个手选添加方便很多。

``` cpp
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
```
上文代码块中，是添加多媒体文件到播放列表的第二个方式，即为打开文件选择对话框后手动选择添加。笔者给这个功能设计的用途为”添加播放列表“，即为在批量添加文件夹内的文件后，可以更自定义的选择想要补充添加的少量文件，当然，文件支持多选添加。
```cpp
void MainWindow::on_tabWidget_2_tabBarClicked(int index) //选中这个框框之后读取电脑音乐文件夹中的东西
{
    if(index == 1){ //选中了此电脑界面
        QString user_path = "D:\\CloudMusic";
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
```
“此电脑”选项卡在选中时，会自动读取指定目录下文件的内容，并且添加到User_MediaList中，这里由于playlist会有冲突，所以使用起来其实不是很方便，在之后的版本中会对这个进行修改。

``` cpp
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
```
这里是提供另外一种选择播放媒体的方式，即为添加服务器的URL。为了把服务器文件的播放列表和本地播放列表区分开，就新建了playlist_cloud来存储播放路径。

由于服务器的带宽有限，所以需要等待服务器加载一段时间才能完成文件的加载，所以需要等待一段时间才能播放。也正是因此，没有在服务器放太多的资源。

## 2、多媒体文件播放控制

``` cpp
void MainWindow::on_B_play_clicked()
{
    if(playlist.empty()){
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
```
这是播放按钮的点击响应事件，关键就在于如何让一个按钮的点击事件实现播放、暂停、续播三个功能。实现的方法在于获取mediaplayer的播放状态(playbackstate)，用switch分支根据不同的播放情况，实现播控的调整。可以说这个按钮是所有代码的核心也不为过。

``` cpp
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
```
这是播放上一个、下一个、和停止播放的按钮事件，基本就是修改选中的列表行就行，本来想是直接选择下一个之后播放的，但是这样就削弱了播放按钮的重要地位，所以达咩。

停止按钮是stop()函数，注意与暂停函数pause()的区分。

想要实现上下循环选择的难点就在于约瑟夫环的使用，可以看到播放下一个文件是用了约瑟夫环的方式，如果不使用约瑟夫环的向上选择可以参考播放向上播放，实现起来比较繁琐。

```cpp
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
```
倍速播放控制，本质上就是改改参数，没什么技术含量。

```cpp
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
```
使用进度条控制媒体播放位置，以及显示剩余时间、已经播放时间。
这里关键就在于自定义事件的使用，使用MediaPlayer的durationchange事件联动进度条的变化，以及格式化文本字符串的使用。

```cpp
 //音量调节
    ui->SL_sound->setRange(0,100);
    ui->SL_sound->setValue(100); //设置默认是满音量吧，感觉这个调整音量没什么用
    connect(ui->SL_sound,&QSlider::sliderMoved,this,[=](float soundDB){
        audiooutput->setVolume(soundDB / 100);
        qInfo()<<soundDB / 100;
    });
```
拖动滑块改变音量，同样也是自定义事件的使用。这里的关键就在于mediaplayer的音量大小区间为[0,1]，所以需要把进度条的范围转化成小数在设置为音量。因为进度条的范围一定是整数的区间，所以设置的区间大一点如[0,100]在拖动起来就会流畅很多。


## 3、播放列表控制

``` cpp
void MainWindow::on_pushButton_11_clicked() // 删除播放列表中的选项
{
    int row = ui->L_MediaList->currentRow(); //获取选中的行
    MediaList.removeAt(row);
    playlist.removeAt(row);
    ui->L_MediaList->clear();
    ui->L_MediaList->addItems(MediaList);

}
```
删除播放列表中的内容，就直接清除数组中的对象就行。然后再刷新下L_Medialist。

``` cpp
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
```
这里是把播放列表的选中对象置顶或者置底，思路是半个冒泡排序，需要注意的是开始交换位置的起点。如果循环变量设置错误的话，会导致全部都是那个被转移的东西，或者直接下标越界。

在交换的时候注意要有中间变量，以及使用函数来优化多段重复代码。

``` cpp
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
```
向下和向上移位置，需要考虑的就是防止下标越界，在这里设置循环移动很奇怪，所以就设计成移动到顶端之后按钮就不再响应了。本质上就是两个数组对象之间的位置交换，注意留出中间变量。
``` cpp
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
```
这个就纯粹是因为PotPlayer有排序功能，所以就自己用冒泡排序复刻了一下效果。难点在于，MediaList和Playlist是联动的关系，导致使用数组自带的排序函数会打破这个关联的形式，解决方案就是自己手动排序了，
这样对于排序的过程更可控一点。

## 4、界面美化设置

```cpp
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
```
这里修改主题的方式是修改主窗口的样式表，图片样子可以自定义，就随便找了一张，再次点击可以切换回到原来的样子。


