# directui

## 前言

本项目将开源项目[duilib]([GitHub - duilib/duilib](https://github.com/duilib/duilib))以及[gtkduilib]([GitHub - progmboy/gtkduilib](https://github.com/progmboy/gtkduilib))两个项目的源码进行整合。提供统一接口，以便duilib能够以统一的接口运行在不同的操作系统平台以及国产化的操作系统。项目中采用了c++11标准编写代码，在涉及的常量字符串中使用u8(这是c++11的字符串常量前缀)作为前缀，表示字符串以utf8的编码方式进行内存存储。这样同一份代码无论是在windows操作系统还是linux操作系统，使用gcc还是vs或是其它编译器进行编译，字符串都是以utf8编码的格式编译到目标文件中。由于windows操作系统平台使用A和W两种版本的API函数，Linux操作系统的API默认使用utf8编码，对于操作系统API的调用在windows操作系统平台只需要utf8编码的字符串转换为UCS2，然后调用windows操作系统的W版本API如CreateWindowW、GetTextMetricsW、CreateFontIndirectW等。

## 效果示例

目前已将[duilib]([GitHub - duilib/duilib](https://github.com/duilib/duilib))原版本中部分示例完成了移植，在不同操作系统平台展示出一致的效果。

* windows 操作系统
  
![360safe_win](https://github.com/mxway/directui/blob/main/images/win_360safe.png)
![list_win](https://github.com/mxway/directui/blob/main/images/win_list.png)

* Ubuntu 20

![360safe_ubuntu](https://github.com/mxway/directui/blob/main/images/ubuntu_360safe.png)
![list_ubuntu](https://github.com/mxway/directui/blob/main/images/ubuntu_list.png)

* deepin

![360safe_deepin](https://github.com/mxway/directui/blob/main/images/deepin_360safe.png)
![list_deepin](https://github.com/mxway/directui/blob/main/images/deepin_list.png)

* kylin

* UOS

## 技术交流
欢迎添加微信进行技术交流
![微信联系方式](https://github.com/mxway/directui/blob/main/images/%E5%BE%AE%E4%BF%A1%E8%81%94%E7%B3%BB%E6%96%B9%E5%BC%8F.jpg)

## TODO

- [ ] DrawHtml功能目前已完成window操作系统的W版本修改工作。Linux已完成DrawHtml大部分功能移植，但是对于选中文本背景色功能还未实现

- [ ] Edit控件不使用原版中的代码，使用自绘控件功能，目前windows已完成Edit部分功能开发；可支持输入法输入文本，backspace删除文字、delete删除文字、home光标定位、end光标定位功能，但不支持全选。Linux只能显示文字，还不能输入文本。

- [ ] Duilib原版本中的其它示例的移植

- [ ] Tree控件
