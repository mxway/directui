# directui

## 前言

本项目将开源项目[duilib]([GitHub - duilib/duilib](https://github.com/duilib/duilib))以及[gtkduilib]([GitHub - progmboy/gtkduilib](https://github.com/progmboy/gtkduilib))两个项目的源码进行整合。提供统一接口，以便duilib能够以统一的接口运行在不同的操作系统平台以及国产化的操作系统。项目中采用了c++11标准编写代码，在涉及的常量字符串中使用u8(这是c++11的字符串常量前缀)作为前缀，表示字符串以utf8的编码方式进行内存存储。这样同一份代码无论是在windows操作系统还是linux操作系统，使用gcc还是vs或是其它编译器进行编译，字符串都是以utf8编码的格式编译到目标文件中。由于windows操作系统平台使用A和W两种版本的API函数，Linux操作系统的API默认使用utf8编码，对于操作系统API的调用在windows操作系统平台只需要utf8编码的字符串转换为UCS2，然后调用windows操作系统的W版本API如CreateWindowW、GetTextMetricsW、CreateFontIndirectW等。

## 简介

项目目标是将duilib及gtkduilib两个项目进行整合，提供统一的接口。实现一个适合windows、linux以及国产化操作系统的duilib项目，通过简单的xml就能够实现界面效果。为了实现源码能够在不同操作系统，不同的编译器进行编译的目标，所有源码文件的编码格式默认都是UTF8+BOM，这样能够保证无论使用gcc还是vs、在windows还是linux操作系统或是国产化操作系统上进行编译时都不会有编译错误。若使用其它的文件编码格式，源码中存在汉字时可能会造成不同编译器在编译过程中出现错误。

## 源码编译

源码使用cmake进行项目的代码管理。在CMakeLists.txt中会自动根据操作系统将对应的源码文件加入到源码工程中，对于cmake只要cmake版本大小3.11即可，用cmake可以生成Makefile以及vs的源码工程文件。同时要求编译器需要支持c++11特性，在编译时需要判断自己的编译器是否支持c++11特性，对于gcc工具链高于4.8.5的版本即可。目前项目所使用的编译器在windows以及linux操作系统平台下都是使用gcc编译工具链，后续支持windows下vs编译器。

### Windows编译

#### gcc编译

windows下使用mingw对项目进行编译。编译过程如下

切换到项目source目录下。

* 系统环境检查
  
  确定系统中是否支持cmake以及g++编译器。
  
  ```bat
  cmake --version
  ```
  
  输出以下信息
  
  ```bat
  cmake version 3.16.6
  
  CMake suite maintained and supported by Kitware (kitware.com/cmake).
  ```
  
  确定系统中的mingw中g++版本
  
  ```c++
  g++ --version
  ```
  
  输出如下信息，确保g++的目录被添加到系统的PATH环境变量中。
  
  ```bat
  g++ (MinGW-W64 i686-posix-dwarf, built by Brecht Sanders) 7.5.0
  Copyright (C) 2017 Free Software Foundation, Inc.
  This is free software; see the source for copying conditions.  There is NO
  warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  ```

* 编译源码
  
  执行如下命令
  
  ```bat
  mkdir build
  cd build
  cmake ../ -DCMAKE_BUILD_TYPE=Release -G "MinGW Makefiles"
  mingw32-make -j4
  ```

​    以上命令执行完成后在build目录下生成可执行文件。 双击相应的可执行文件查看demo效果。

#### msvc编译

在windows操作系统下可以使用cmake生成visual studio的sln解决方案工程。具体执行过程如下，命令行切换到source目录。执行命令

```bat
mkdir build
cd build
cmake ../ -DCMAKE_BUILD_TYPE=Release
```

以上命令执行完成后会在build目录下生成.sln文件。使用visual studio打开该解决方案进行编译即可生成可执行程序

### Linux编译

在Linux操作系统下依赖于gtk库。所以在编译前需要确定系统中是否已安装了gtk库。以ubuntu为例，安装gtk命令如下

```shell
sudo apt install libgtk-3-dev
```

同时在Linux系统下也需要检查是否安装了cmake以及gcc编译工具链。

```shell
cmake --version
g++ --version
```

分别用于查看系统中是否安装了cmake以及g++。

**代码编译**

切换到项目source目录下执行以下命令

```shell
mkdir build && cd build
cmake ../ -DCMAKE_BUILD_TYPE=Release
make -j4
```

执行以上命令后在build目录生成可执行文件。双击或是在命令行中输入可执行文件名运行程序查看软件运行效果。

## 效果示例

目前已将[duilib]([GitHub - duilib/duilib](https://github.com/duilib/duilib))原版本中部分示例完成了移植，在不同操作系统平台展示出一致的效果。

* windows 操作系统

![360safe_win](https://github.com/mxway/directui/blob/main/images/win_360safe.png)
![list_win](https://github.com/mxway/directui/blob/main/images/win_list.png)
![richlist_win)(https://github.com/mxway/directui/blob/main/images/win_richlist.png)
![gamedemo_win)(https://github.com/mxway/directui/blob/main/images/win_gamedemo.png)

* Ubuntu 20

![360safe_ubuntu](https://github.com/mxway/directui/blob/main/images/ubuntu_360safe.png)
![list_ubuntu](https://github.com/mxway/directui/blob/main/images/ubuntu_list.png)
![richlist_ubuntu](https://github.com/mxway/directui/blob/main/images/ubuntu_richlist.png)
![gamedemo_ubuntu)(https://github.com/mxway/directui/blob/main/images/ubuntu_gamedemo.png)

* deepin

![360safe_deepin](https://github.com/mxway/directui/blob/main/images/deepin_360safe.png)
![list_deepin](https://github.com/mxway/directui/blob/main/images/deepin_list.png)

* 银河麒麟

​    ![360safe_kylin](https://github.com/mxway/directui/blob/main/images/kylin_360safe.png)

​    ![list_kylin](https://github.com/mxway/directui/blob/main/images/kylin_list.png)

* UOS

## 技术交流

欢迎添加微信进行技术交流
![微信联系方式](https://github.com/mxway/directui/blob/main/images/%E5%BE%AE%E4%BF%A1%E8%81%94%E7%B3%BB%E6%96%B9%E5%BC%8F.jpg)

## TODO

- [ ] DrawHtml功能目前已完成window操作系统的W版本修改工作。Linux已完成DrawHtml大部分功能移植，但是对于选中文本背景色功能还未实现

- [ ] Edit控件不使用原版中的代码，使用自绘控件功能，目前windows及linux已完成Edit部分基础功能；可支持输入法输入文本，backspace删除文字、delete删除文字、home光标定位、end光标定位功能，但不支持全选以及鼠标选择范围，删除选中文字功能。

- [ ] Duilib原版本中的其它示例的移植

- [ ] Tree控件
