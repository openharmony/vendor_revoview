/******************************************************************************
 *
 *  Copyright (C) 2017 Spreadtrum Corporation
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at:
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 ******************************************************************************/

/******************************************************************************
 *
 *  Filename:      bt_vendor_sprd_ssp.h
 *
 *
 ******************************************************************************/

#ifndef BT_VENDOR_SPRD_SSP_H
#define BT_VENDOR_SPRD_SSP_H

#include "bt_vendor_sprd.h"

#define HCI_SET_SUPER_SSP_ENABLE 0xFCB0
#define HCI_SET_OWN_PP192_KEY 0xFCB1
#define HCI_SET_OWN_PP256_KEY 0xFCB2

#define HCI_SET_P192_DHKEY 0xFCB3
#define HCI_SET_P256_DHKEY 0xFCB4

#define HCI_PEER_P192_PUBLIC_NOTIFICATION_EVT 0xB0
#define HCI_PEER_P256_PUBLIC_NOTIFICATION_EVT 0xB1
#define HCI_SUPER_SSP_REFRESH_REQUEST_EVT 0xB2

void sprd_vnd_ssp_init(void);
void sprd_vse_cback(uint8_t len, uint8_t *p);
void sprd_vnd_ssp_deinit(void);
void sprd_vnd_hci_ssp_cback(void *pmem);
#endif /*BT_VENDOR_SPRD_SSP_H*/
