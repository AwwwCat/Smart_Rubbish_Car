# Smart_Rubbish_Car
“全国大学生嵌入式芯片与系统设计竞赛”参赛作品项目————智能垃圾车。
## 目录
1. [项目介绍](https://github.com/AwwwCat/Smart_Rubbish_Car/blob/master/README.md#%E9%A1%B9%E7%9B%AE%E4%BB%8B%E7%BB%8D)
2. [所用配件](https://github.com/AwwwCat/Smart_Rubbish_Car/blob/master/README.md#%E6%89%80%E7%94%A8%E9%85%8D%E4%BB%B6)
3. [项目说明](https://github.com/AwwwCat/Smart_Rubbish_Car/blob/master/README.md#%E9%A1%B9%E7%9B%AE%E8%AF%B4%E6%98%8E)
4. [主要文件说明](https://github.com/AwwwCat/Smart_Rubbish_Car/blob/master/README.md#%E4%B8%BB%E8%A6%81%E6%96%87%E4%BB%B6%E8%AF%B4%E6%98%8E)
5. [部分代码原理简述](https://github.com/AwwwCat/Smart_Rubbish_Car/blob/master/README.md#%E9%83%A8%E5%88%86%E4%BB%A3%E7%A0%81%E5%8E%9F%E7%90%86%E7%AE%80%E8%BF%B0)
6. [其他](https://github.com/AwwwCat/Smart_Rubbish_Car/blob/master/README.md#%E5%85%B6%E4%BB%96)

## 项目介绍
据相关研究以及调查表明，全球每年烟头垃圾有28亿升，全球每年塑料瓶消费量达5000亿个……可见，街头垃圾量不容小觑，为此，本队参赛设计为一台智能垃圾收集车。其能够在公园、学校、商场等场所进行自动垃圾收集活动。其搭载的摄像头能够识别一定类型的垃圾，可以进行分类并收集。小车能够直行或拐弯。搭载了超声波测距模块，且拥有基本的寻路算法和系统，能够在正常运行中将沿路边行走并能及时避障，若在一定范围内识别到垃圾，将前往收集，收集后返回路边并继续前行。车顶装载有车灯，当正常运行时车灯将正常显示，若发现有垃圾需要收集，将进行相应的颜色变化，提醒路人小心不要碰撞。

## 所用配件
* 主控板————[赤菟开发板](https://www.wch.cn/products/CH32V307.html "赤菟开发板官方介绍")  
![](https://mmbiz.qpic.cn/mmbiz_jpg/t4HGUiau80iagyzwKKXkiaylc8v6hT6rSf2l2RxBoqZvzib2YrcVaKNVjLHEW8njXTozM2UhTI33yoKmjr8BTiaPTEA/640?wx_fmt=jpeg&tp=webp&wxfrom=5&wx_lazy=1&wx_co=1 "赤菟开发板") 
* FPGA————Logos系列 PGL22G  
![](http://t15.baidu.com/it/u=1667932565,3317867372&fm=224&app=112&f=JPEG?w=500&h=500 "紫光同创 Logos系列 PGL22G")  
* [OpenMV Cam H7 Plus摄像头](https://singtown.com/product/50812/openmv4-h7-plus/ "OpenMV Cam H7 Plus官方介绍")  
![](https://book.openmv.cc/assets/002.jpg "OpenMV Cam H7 Plus摄像头")  
* L298N电机驱动板模块TB6612FNG  
* HC-SR04 US-100 US-015超声波模块  
* DC-DC转换板  
* 舵机，马达，轮子  

## 项目说明
### 软件环境
* MounRiver Studio
* Pango Design Suite
* OpenMV IDE
### 使用方法
将`Smart_Car_ch32v`用 **MounRiver Studio** 打开，编译烧录至 **赤菟开发板**；将`Smart_Car_fpga`用 **Pango Design Suite** 打开，编译烧录至 **PGL22G**；将`ei_image_classification.py`用 **OpenMV IDE** 打开，烧录，`labels.txt`和`trained.tflite`复制进 **OpenMV Cam H7 Plus** 里。

## 主要文件说明
### Smart_Car_ch32v  
#### Smart_Car_ch32v/User  
* car.c  小车行为代码  
* lcd.c  lcd显示屏代码  
* arm.c  机械臂代码  
* main.c  主代码  
  
### Smart_Car_fpga  
#### Smart_Car_fpga/prj/sourse   
* smart_car.fdc  引脚分配文件

#### Smart_Car_fpga/rtl  
* smart_car.v  verilog文件 

### OpenMV
* trained.tflite 垃圾图像训练模型

## 部分代码原理简述
```
u8 pick = High_Level_Measure();                 // 获取高电平，若有垃圾则则接收到高电平，反之为0
if (pick != 0)                                  // 若不为0，则有垃圾
{
    Echo_Switch(Close);
    u16 angle = 0;
    while (!angle)                              // 当角度为零，即未接收到角度信号时，再次接收
        angle = Angle_Measure();
    Turn_Angle(angle);
    u16 distance = 0;
    while (!distance)                           // 当距离为零，即未接收到距离信号时，再次接收
        distance = Distance_Measure();
    u8 refuse = Refuse_Classification();        // 获取分类信息
...
```
摄像头检测到阈值高于0.85的物体时，发出一段高电平，单片机识别到高电平进入拾取模式，关闭超声波模块并等待角度信号。摄像头会发送一段高电平，高电平时长代表小车需要旋转的角度，10us代表一度。接收到角度信号后小车旋转并等待距离信号，摄像头会在一段时间后发送距离信号，高电平时长代表小车需要行走的距离，10us代表1cm。同时摄像头将发送垃圾信息并用单片机接收。
> 垃圾信息可以同样通过高电平时长分类，但由于演示为两类垃圾，所以仅使用高低电平进行区分。

## 其他
### 作者
* [AwwwCat](https://github.com/AwwwCat)    主要进行ch32v30x和OpenMV的编写
* [jixieer](https://github.com/jixieer)    主要进行FPGA的编写和电路设计

### 许可
允许他人使用、修改、复制和分发你的代码，甚至可以用于商业用途。代码可以被私有化和闭源，只要包含版权声明和许可证条件即可。
