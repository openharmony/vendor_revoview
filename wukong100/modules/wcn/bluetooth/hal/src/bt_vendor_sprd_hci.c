/******************************************************************************
 *
 *  Copyright (C) 2016 Spreadtrum Corporation
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
 *  This is the stream state machine for the BRCM offloaded advanced audio.
 *
 ******************************************************************************/

#define LOG_TAG "bt_vnd_hci"

#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <utils/Log.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <ctype.h>
#include <stdlib.h>

#include "bt_hci_bdroid.h"
#include "bt_vendor_sprd.h"
#include "bt_vendor_sprd_hci.h"

#include <pthread.h>

#include <netinet/in.h>
#include <semaphore.h>

#define BTD(param, ...) {HILOGD(param, ## __VA_ARGS__);}
#define BTE(param, ...) {HILOGE(param, ## __VA_ARGS__);}

#define HCI_TIMEOUT_VALUE 6
#define HCI_TIMEOUT_STRING "ACK: TIMEOUT"
#define READ_BUFFER_MAX 1024

#define VENDOR_HCI_SOCKET "bt_vendor_sprd_hci"
#define VENDOR_N79_FLAG_NODE "/sys/class/misc/wcn/devices/n79_mode"

/*****************************************************************************
** Constants and types
*****************************************************************************/
static int s_listen = -1;
static int s_socket = -1;
static sem_t thread_lock;
static pthread_t thread_id;
static bool thread_isrunning = FALSE;

/*******************************************************************************
** Local Utility Functions
*******************************************************************************/

static void log_bin_to_hexstr(uint8_t *bin, uint8_t binsz, const char *log_tag)
{
  char     *str, hex_str[]= "0123456789abcdef";
  uint8_t  i;

  str = (char *)malloc(binsz * 3);
  if (!str) {
    HILOGE("%s alloc failed", __FUNCTION__);
    return;
  }

  for (i = 0; i < binsz; i++) {
      str[(i * 3) + 0] = hex_str[(bin[i] >> 4) & 0x0F];
      str[(i * 3) + 1] = hex_str[(bin[i]     ) & 0x0F];
      str[(i * 3) + 2] = ' ';
  }
  str[(binsz * 3) - 1] = 0x00;
  HILOGD("%s %s", log_tag, str);
  free(str);
}


uint8_t sprd_vnd_send_hci_vsc(uint16_t cmd, uint8_t *payload, uint8_t len, hci_cback cback)
{
    HC_BT_HDR   *p_buf;
    uint8_t     *p, status;
    uint16_t    opcode;

    // Perform Opening configure cmds. //
    if (bt_vendor_cbacks) {
        p_buf = (HC_BT_HDR *)bt_vendor_cbacks->alloc(
            BT_HC_HDR_SIZE + HCI_CMD_PREAMBLE_SIZE + len);
        if (p_buf)
        {
            p_buf->event = MSG_STACK_TO_HC_HCI_CMD;
            p_buf->offset = 0;
            p_buf->layer_specific = 0;
            p_buf->len = HCI_CMD_PREAMBLE_SIZE + len;
            p = (uint8_t *)(p_buf + 1);

            UINT16_TO_STREAM(p, cmd);
            *p++ = len;
            memcpy(p, payload, len);
            log_bin_to_hexstr((uint8_t *)(p_buf + 1), HCI_CMD_PREAMBLE_SIZE + len, __FUNCTION__);
            if (bt_vendor_cbacks->xmit_cb(cmd, p_buf))
            {
                return BTC_OP_RESULT_SUCCESS;
            }
            bt_vendor_cbacks->dealloc(p_buf);
        }
    }
    return BTC_OP_RESULT_FAIL;
}

void sprd_vnd_socket_turn_back(uint8_t *data, size_t length)
{
    if (s_socket > 0) {
        if (write(s_socket, data, length) == -1 && errno == ECONNRESET) {
            BTD("write: %d error: %s (%d)", s_socket, strerror(errno),errno);
        }
    }
}

void sprd_vnd_socket_hci_uipc_cback(void *pmem)
{
    HC_BT_HDR    *p_evt_buf = (HC_BT_HDR *)pmem;
    uint8_t     *p, len, vsc_result, uipc_opcode;
    uint16_t    vsc_opcode, uipc_event;
    HC_BT_HDR    *p_buf = NULL;

    p = (uint8_t *)(p_evt_buf + 1) + HCI_EVT_CMD_CMPL_LEN;
    len = *p;
    p = (uint8_t *)(p_evt_buf + 1) + HCI_EVT_CMD_CMPL_VSC;
    STREAM_TO_UINT16(vsc_opcode,p);
    //vsc_result = *p++;

    log_bin_to_hexstr(((uint8_t *)(p_evt_buf + 1) + HCI_EVT_CMD_CMPL_VSC), len-1, __FUNCTION__);

    if (vsc_opcode == HCI_WRITE_LOCAL_HW_REGISTER) {
        sprd_vnd_socket_turn_back(p, len -1);
    } else if (vsc_opcode == HCI_READ_LOCAL_HW_REGISTER) {
        sprd_vnd_socket_turn_back(p, len -1);
   } else if (vsc_opcode == HCI_WRITE_LOCAL_RF_REGISTER) {
        sprd_vnd_socket_turn_back(p, len -1);
   } else if (vsc_opcode == HCI_READ_LOCAL_RF_REGISTER) {
        sprd_vnd_socket_turn_back(p, len -1);
    }

    sem_post(&thread_lock);
    /* Free the RX event buffer */
    bt_vendor_cbacks->dealloc(p_evt_buf);
}

static void client_connection_handler(int fd){
    int ret = 0, i;
    uint16_t opcode = 0;
    uint32_t value;
    uint8_t buffer[READ_BUFFER_MAX] = {0}, *p, *q, msg_req[HCI_CMD_MAX_LEN];
    uint8_t length;

    ret = read(fd, buffer, READ_BUFFER_MAX);
    if (ret <= 0) {
        BTD("%s read: %d error", __func__, fd);
        return;
    }

    log_bin_to_hexstr(buffer, ret, __FUNCTION__);

    p = buffer;
    STREAM_TO_UINT32(value, p);
    STREAM_TO_UINT16(opcode, p);
    STREAM_TO_UINT8(length, p);

    BTD("encrpty_code: 0x%08x, opcode: 0x%04x, length: 0x%02x", value, opcode, length);

    STREAM_TO_UINT32(value, p);

    q = msg_req;
    UINT32_TO_STREAM(q, value);
    UINT8_TO_STREAM(q, length);

    if (opcode == HCI_WRITE_LOCAL_HW_REGISTER
        || opcode == HCI_WRITE_LOCAL_RF_REGISTER) {
        for (i = 0; i < length; i++) {
            STREAM_TO_UINT32(value, p);
            UINT32_TO_STREAM(q, value);
        }
    }

    sprd_vnd_send_hci_vsc(opcode, msg_req, q - msg_req, sprd_vnd_socket_hci_uipc_cback);
}

static inline int accept_server_socket(int sfd) {
    struct sockaddr_un remote;
    int fd;
    socklen_t len = sizeof(struct sockaddr_un);

    if ((fd = accept(sfd, (struct sockaddr *)&remote, &len)) == -1) {
         BTE("sock accept failed (%s)", strerror(errno));
         return -1;
    }
    return fd;
}
