#!/bin/bash

# Copyright (C) 2023 HiHope Open Source Organization .
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
set -e

pushd ${1}

ROOT_BUILD_DIR=${2}
ROOT_DIR=${2}/../..

MODULES_SOURCE=${3}
COMMON_SOURCE=${4}
GPU_MODULES_SOURCE=${5}
export PATH=${ROOT_DIR}/prebuilts/clang/ohos/linux-x86_64/llvm/bin/:$PATH
MAKE="make LLVM=1 LLVM_IAS=1 CROSS_COMPILE=aarch64-linux-gnu-"

export BSP_MODULES_OUT=${ROOT_BUILD_DIR}/modules

#wcn bt driver config
export BSP_BOARD_UNISOC_WCN_SOCKET="sdio"

#wcn module version config
export BSP_BOARD_WLAN_DEVICE="sc2355"

export BSP_BOARD_PRODUCT_USING_VDSP="qogirn6pro"

BSP_KERNEL_OUT=${ROOT_BUILD_DIR}/kernel/OBJ/linux-5.15
BSP_KERNEL_PATH=${ROOT_BUILD_DIR}/kernel/src_tmp/linux-5.15

#BSP_KERNEL_CROSS_COMPILE=${ROOT_DIR}/prebuilts/gcc/linux-x86/aarch64/gcc-linaro-7.5.0-2019.12-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-


rm -rf ${BSP_MODULES_OUT} 

mkdir -p ${BSP_MODULES_OUT}

export BSP_MODULE_DISP_VERSION="ums9620"
${MAKE} -C ${GPU_MODULES_SOURCE}/unisoc_platform/sprdwcn -f Makefile O=${BSP_KERNEL_OUT} \
    ARCH=arm64  \
    BSP_KERNEL_OUT=${BSP_KERNEL_OUT} \
    BSP_KERNEL_PATH=${BSP_KERNEL_PATH} \
    BSP_MODULES_OUT=${BSP_MODULES_OUT}  modules -j64

${MAKE} -C ${GPU_MODULES_SOURCE}/wcn/wlan/wlan_combo -f Makefile O=${BSP_KERNEL_OUT} \
    ARCH=arm64  \
    CROSS_COMPILE=${BSP_KERNEL_CROSS_COMPILE} \
    BSP_KERNEL_OUT=${BSP_KERNEL_OUT} \
    BSP_KERNEL_PATH=${BSP_KERNEL_PATH} \
    BSP_MODULES_OUT=${BSP_MODULES_OUT}  modules -j64

${MAKE} -C ${GPU_MODULES_SOURCE}/wcn/bluetooth/driver -f Makefile O=${BSP_KERNEL_OUT} \
    ARCH=arm64  \
    CROSS_COMPILE=${BSP_KERNEL_CROSS_COMPILE} \
    BSP_KERNEL_OUT=${BSP_KERNEL_OUT} \
    BSP_KERNEL_PATH=${BSP_KERNEL_PATH} \
    BSP_MODULES_OUT=${BSP_MODULES_OUT}  modules -j64

${MAKE} -C ${ROOT_DIR}/unisoc_p7885_property/modules/wcn/gnss/unisoc_gnss/gnss_common_ctl  -f Makefile O=${BSP_KERNEL_OUT} \
    ARCH=arm64  \
    BSP_KERNEL_OUT=${BSP_KERNEL_OUT} \
    BSP_KERNEL_PATH=${BSP_KERNEL_PATH} \
    BSP_MODULES_OUT=${BSP_MODULES_OUT}  modules -j64

${MAKE} -C ${ROOT_DIR}/unisoc_p7885_property/modules/wcn/gnss/unisoc_gnss/gnss_dbg  -f Makefile O=${BSP_KERNEL_OUT} \
    ARCH=arm64  \
    BSP_KERNEL_OUT=${BSP_KERNEL_OUT} \
    BSP_KERNEL_PATH=${BSP_KERNEL_PATH} \
    BSP_MODULES_OUT=${BSP_MODULES_OUT}  modules -j64

${MAKE} -C ${ROOT_DIR}/unisoc_p7885_property/modules/wcn/gnss/unisoc_gnss/gnss_pmnotify_ctl  -f Makefile O=${BSP_KERNEL_OUT} \
    ARCH=arm64  \
    BSP_KERNEL_OUT=${BSP_KERNEL_OUT} \
    BSP_KERNEL_PATH=${BSP_KERNEL_PATH} \
    BSP_MODULES_OUT=${BSP_MODULES_OUT}  modules -j64

export MALI_PLATFORM_NAME=qogirn6pro
${MAKE} -C ${GPU_MODULES_SOURCE}/gpu/natt -f Makefile O=${BSP_KERNEL_OUT} \
    ARCH=arm64  \
    BSP_KERNEL_OUT=${BSP_KERNEL_OUT} \
    BSP_KERNEL_PATH=${BSP_KERNEL_PATH} \
    BSP_MODULES_OUT=${BSP_MODULES_OUT}  modules -j64

${MAKE} -C ${ROOT_DIR}/kernel_unisoc_p7885/drivers/input/touchscreen/gslx680 -f Makefile O=${BSP_KERNEL_OUT} \
    ARCH=arm64  \
    BSP_KERNEL_OUT=${BSP_KERNEL_OUT} \
    BSP_KERNEL_PATH=${BSP_KERNEL_PATH} \
    BSP_MODULES_OUT=${BSP_MODULES_OUT}  modules -j64

${MAKE} -C ${GPU_MODULES_SOURCE}/video/sprd-vpu -f Makefile O=${BSP_KERNEL_OUT} \
    ARCH=arm64  \
    BSP_KERNEL_OUT=${BSP_KERNEL_OUT} \
    BSP_KERNEL_PATH=${BSP_KERNEL_PATH} \
    BSP_MODULES_OUT=${BSP_MODULES_OUT}  modules -j64

NPU_MODULES_SOURCE=${ROOT_DIR}/unisoc_p7885_property/modules/npu

${MAKE} -C ${NPU_MODULES_SOURCE}/vdsp -f Makefile O=${BSP_KERNEL_OUT} \
    ARCH=arm64  \
    BSP_KERNEL_OUT=${BSP_KERNEL_OUT} \
    BSP_KERNEL_PATH=${BSP_KERNEL_PATH} \
    BSP_MODULES_OUT=${BSP_MODULES_OUT}  modules -j64

${MAKE} -C ${NPU_MODULES_SOURCE}/imgtec -f Makefile O=${BSP_KERNEL_OUT} \
    ARCH=arm64  \
    BSP_KERNEL_OUT=${BSP_KERNEL_OUT} \
    BSP_KERNEL_PATH=${BSP_KERNEL_PATH} \
    BSP_MODULES_OUT=${BSP_MODULES_OUT}  modules -j64

#################### ws73 build start ######################################
mkdir -p ${BSP_MODULES_OUT}/ws73
cp -rf ${GPU_MODULES_SOURCE}/ws73/sle ${BSP_MODULES_OUT}/ws73/sle

export KERNELPATH=${BSP_KERNEL_OUT}
echo "KERNELPATH = [$KERNELPATH]"
export KERNELARCH=arm64
export TOOLPREFIX_PATH=${ROOT_DIR}/prebuilts/gcc/linux-x86/aarch64/gcc-linaro-7.5.0-2019.12-x86_64_aarch64-linux-gnu/bin/
export TOOLPREFIX=${TOOLPREFIX_PATH}aarch64-linux-gnu-
export WSCFG_KERNEL_DIR=${KERNELPATH}
export WSCFG_ARCH_NAME=${KERNELARCH}
export KSRC=${KERNELPATH}
export WSCFG_CROSS_COMPILE=${TOOLPREFIX}

${MAKE} -C ${BSP_MODULES_OUT}/ws73/sle -f Makefile  O=${BSP_KERNEL_OUT} \
	BSP_MODULES_OUT=${BSP_MODULES_OUT} platform -j64

${MAKE} -C ${BSP_MODULES_OUT}/ws73/sle -f Makefile  O=${BSP_KERNEL_OUT} \
	BSP_MODULES_OUT=${BSP_MODULES_OUT} sle -j64

#################### ws73 build end ######################################

${MAKE} -C ${GPU_MODULES_SOURCE}/audio -f Makefile O=${BSP_KERNEL_OUT} \
    ARCH=arm64  \
    BSP_KERNEL_OUT=${BSP_KERNEL_OUT} \
    BSP_KERNEL_PATH=${BSP_KERNEL_PATH} \
    BSP_MODULES_OUT=${BSP_MODULES_OUT}  modules -j64
