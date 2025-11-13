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
