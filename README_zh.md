# vendor_revoview

## 简介

本仓用于存放WUKONG100的hdf配置信息、产品定义信息，以及对应开发板（产品形态）的相关demo案例。

## 目录
```
/vendor/revoview
.
├── bootanimation           #开机动画相关json配置
├── bundle.json             #编译组件配置
├── config.json             #产品功能配置
├── default_app_config      #默认app配置
├── etc                     #hardware, product参数配置
├── hals
│   ├── audio               #audio alsa方案参数配置
├── hdf_config              #audio, camera, device_info, sensor等HDF驱动hcs文件配置        
├── image_conf              #ramdisk, system, updater分区大小参数配置
├── modules                 #内核modules编译脚本
├── ohos.build              #编译组件控制脚本
├── preinstall-config       #预装app json配置文件
├── product.gni             #product配置gni
├── resourceschedule        #cgroup相关json配置文件
├── security_config         #安全相关jso配置文件
├── updater_config          #updater配置文件
└── window_config           #window 窗口配置xml文件
```


## 使用方法

[WUKONG100开发板使用说明](https://gitcode.com/openharmony/device_board_revoview/blob/master/wukong100/README_zh.md)


## 许可说明

参见本目录下LICENSE文件


## 相关仓

[device_board_revoview](https://gitcode.com/openharmony/device_board_revoview)

[device_soc_unisoc](https://gitcode.com/openharmony/device_soc_unisoc)

