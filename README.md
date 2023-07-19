# Smart_Rubbish_Car
“全国大学生嵌入式芯片与系统设计竞赛”参赛作品项目————智能垃圾车。

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
将`Smart_Car_ch32v`用 **MounRiver Studio** 打开，编译烧录至 **赤菟开发板**；将`Smart_Car_fpga`用 **Pango Design Suite** 打开，编译烧录至 **PGL22G**。

## 文件说明
### Smart_Car_ch32v  
> Smart_Car_ch32v  
>> User  
>>> car.c  小车行为代码  
>>> lcd.c  lcd显示屏代码  
>>> arm.c  机械臂代码  
>>> main.c  主代码  
  
### Smart_Car_fpga  
> Smart_Car_fpga  
>> prj  
>>> sourse  
>>>> smart_car.fdc  引脚分配文件  
>> rtl  
>>> smart_car.v  verilog文件  

## 代码原理简述

## 其他
### 作者
* [AwwwCat](https://github.com/AwwwCat)    主要进行ch32v30x和OpenMV的编写
* [jixieer](https://github.com/jixieer)    主要进行FPGA的编写和电路设计

### 许可
允许他人使用、修改、复制和分发你的代码，甚至可以用于商业用途。代码可以被私有化和闭源，只要包含版权声明和许可证条件即可。
