# vendor_revoview

## Introduction

This repository is used to store HDF configuration information, product definition information, and related demo cases for the corresponding development board (product form) of WUKONG100.

## Directory
```
/vendor/revoview
.
├── bootanimation           # Boot animation related JSON configuration
├── bundle.json             # Build component configuration
├── config.json             # Product feature configuration
├── default_app_config      # Default app configuration
├── etc                     # Hardware, product parameter configuration
├── hals
│   ├── audio               # Audio ALSA solution parameter configuration
├── hdf_config              # HDF driver hcs file configuration for audio, camera, device_info, sensor, etc.        
├── image_conf              # Ramdisk, system, updater partition size parameter configuration
├── modules                 # Kernel modules build script
├── ohos.build              # Build component control script
├── preinstall-config       # Pre-installed app JSON configuration file
├── product.gni             # Product configuration gni
├── resourceschedule        # Cgroup related JSON configuration files
├── security_config         # Security related JSON configuration files
├── updater_config          # Updater configuration file
└── window_config           # Window configuration XML file
```


## Usage

[WUKONG100 Development Board User Guide](https://gitcode.com/openharmony/device_board_revoview/blob/master/wukong100/README_EN.md)


## License

See the LICENSE file in this directory.


## Related Repositories

[device_board_revoview](https://gitcode.com/openharmony/device_board_revoview)

[device_soc_unisoc](https://gitcode.com/openharmony/device_soc_unisoc)
