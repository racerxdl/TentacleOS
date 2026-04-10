// Copyright (c) 2025 HIGH CODE LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "nfc_apdu.h"

#include <string.h>

#include "esp_log.h"

#include "nfc_tcl.h"

static const char *TAG = "NFC_APDU";

#define APDU_MAX_BUF 300U

const char *nfc_apdu_sw_str(uint16_t sw) {
  switch (sw) {
    case APDU_SW_OK:
      return "OK";
    case APDU_SW_WRONG_LENGTH:
      return "Wrong length";
    case APDU_SW_SECURITY:
      return "Security status not satisfied";
    case APDU_SW_AUTH_FAILED:
      return "Authentication failed";
    case APDU_SW_FILE_NOT_FOUND:
      return "File not found";
    case APDU_SW_INS_NOT_SUP:
      return "INS not supported";
    case APDU_SW_CLA_NOT_SUP:
      return "CLA not supported";
    case APDU_SW_UNKNOWN:
      return "Unknown error";
    default:
      return "SW error";
  }
}

hb_nfc_err_t nfc_apdu_sw_to_err(uint16_t sw) {
  if (sw == APDU_SW_OK)
    return HB_NFC_OK;
  if (sw == APDU_SW_SECURITY || sw == APDU_SW_AUTH_FAILED)
    return HB_NFC_ERR_AUTH;
  return HB_NFC_ERR_PROTOCOL;
}

static int apdu_transceive_raw(const nfc_apdu_channel_t *ch,
                               const uint8_t *apdu,
                               size_t apdu_len,
                               uint8_t *rx,
                               size_t rx_max) {
  return nfc_tcl_transceive(ch->proto, ch->ctx, apdu, apdu_len, rx, rx_max, ch->timeout_ms);
}

static int apdu_get_response(const nfc_apdu_channel_t *ch, uint8_t le, uint8_t *rx, size_t rx_max) {
  uint8_t apdu[APDU_HEADER_SIZE] = {APDU_CLA_ISO7816, APDU_INS_GET_RESPONSE, 0x00, 0x00, le};
  return apdu_transceive_raw(ch, apdu, sizeof(apdu), rx, rx_max);
}

hb_nfc_err_t nfc_apdu_transceive(const nfc_apdu_channel_t *ch,
                                 const uint8_t *apdu,
                                 size_t apdu_len,
                                 nfc_apdu_resp_t *resp) {
  if (!ch || !ch->ctx || !apdu || apdu_len == 0 || !resp || !resp->buf || resp->max < 2U)
    return HB_NFC_ERR_PARAM;

  uint8_t cmd[APDU_MAX_BUF];
  if (apdu_len > sizeof(cmd))
    return HB_NFC_ERR_PARAM;
  memcpy(cmd, apdu, apdu_len);

  bool sw_in_buf = true;

  int len = apdu_transceive_raw(ch, cmd, apdu_len, resp->buf, resp->max);
  if (len < 2)
    return HB_NFC_ERR_PROTOCOL;

  uint16_t status = (uint16_t)((resp->buf[len - 2] << 8) | resp->buf[len - 1]);

  if (((status & APDU_SW_CLASS_MASK) == APDU_SW_WRONG_LE) && apdu_len >= APDU_HEADER_SIZE) {
    cmd[apdu_len - 1] = (uint8_t)(status & APDU_SW_DATA_MASK);
    len = apdu_transceive_raw(ch, cmd, apdu_len, resp->buf, resp->max);
    if (len < 2)
      return HB_NFC_ERR_PROTOCOL;
    status = (uint16_t)((resp->buf[len - 2] << 8) | resp->buf[len - 1]);
  }

  if ((status & APDU_SW_CLASS_MASK) == APDU_SW_MORE_DATA) {
    size_t out_pos = (size_t)(len - 2);
    uint8_t le = (uint8_t)(status & APDU_SW_DATA_MASK);
    if (le == 0)
      le = 0x00;

    for (int i = 0; i < APDU_MAX_GET_RESP; i++) {
      uint8_t tmp[APDU_MAX_BUF];
      int rlen = apdu_get_response(ch, le, tmp, sizeof(tmp));
      if (rlen < 2)
        break;

      uint16_t swr = (uint16_t)((tmp[rlen - 2] << 8) | tmp[rlen - 1]);
      int data_len = rlen - 2;
      if (data_len > 0) {
        size_t copy = (out_pos + (size_t)data_len <= resp->max)
                          ? (size_t)data_len
                          : (resp->max > out_pos ? (resp->max - out_pos) : 0);
        if (copy > 0) {
          memcpy(&resp->buf[out_pos], tmp, copy);
          out_pos += copy;
        }
      }

      status = swr;
      if ((status & APDU_SW_CLASS_MASK) != APDU_SW_MORE_DATA)
        break;
      le = (uint8_t)(status & APDU_SW_DATA_MASK);
      if (le == 0)
        le = 0x00;
    }

    if (out_pos + 2 <= resp->max) {
      resp->buf[out_pos++] = (uint8_t)(status >> 8);
      resp->buf[out_pos++] = (uint8_t)(status & APDU_SW_DATA_MASK);
      sw_in_buf = true;
    } else {
      sw_in_buf = false;
    }
    len = (int)out_pos;
  }

  resp->sw = status;
  resp->len = sw_in_buf ? (size_t)(len - 2) : (size_t)len;
  return nfc_apdu_sw_to_err(status);
}

hb_nfc_err_t nfc_apdu_send(const nfc_apdu_channel_t *ch,
                           const uint8_t *apdu,
                           size_t apdu_len,
                           nfc_apdu_resp_t *resp) {
  return nfc_apdu_transceive(ch, apdu, apdu_len, resp);
}

hb_nfc_err_t nfc_apdu_recv(const nfc_apdu_channel_t *ch,
                           const uint8_t *apdu,
                           size_t apdu_len,
                           nfc_apdu_resp_t *resp) {
  return nfc_apdu_transceive(ch, apdu, apdu_len, resp);
}

size_t
nfc_apdu_build_select_aid(nfc_apdu_buf_t dst, nfc_apdu_data_t aid, bool le_present, uint8_t le) {
  if (!dst.buf || !aid.ptr || aid.len == 0)
    return 0;
  if (dst.max < APDU_HEADER_SIZE + aid.len + (le_present ? 1U : 0U))
    return 0;
  size_t pos = 0;
  dst.buf[pos++] = APDU_CLA_ISO7816;
  dst.buf[pos++] = APDU_INS_SELECT;
  dst.buf[pos++] = 0x04;
  dst.buf[pos++] = 0x00;
  dst.buf[pos++] = (uint8_t)aid.len;
  memcpy(&dst.buf[pos], aid.ptr, aid.len);
  pos += aid.len;
  if (le_present)
    dst.buf[pos++] = le;
  return pos;
}

size_t nfc_apdu_build_select_file(nfc_apdu_buf_t dst, uint16_t fid, bool le_present, uint8_t le) {
  if (!dst.buf || dst.max < (7U + (le_present ? 1U : 0U)))
    return 0;
  size_t pos = 0;
  dst.buf[pos++] = APDU_CLA_ISO7816;
  dst.buf[pos++] = APDU_INS_SELECT;
  dst.buf[pos++] = 0x00;
  dst.buf[pos++] = 0x0C;
  dst.buf[pos++] = 0x02;
  dst.buf[pos++] = (uint8_t)((fid >> 8) & 0xFFU);
  dst.buf[pos++] = (uint8_t)(fid & 0xFFU);
  if (le_present)
    dst.buf[pos++] = le;
  return pos;
}

size_t nfc_apdu_build_read_binary(nfc_apdu_buf_t dst, uint16_t offset, uint8_t le) {
  if (!dst.buf || dst.max < APDU_HEADER_SIZE)
    return 0;
  dst.buf[0] = APDU_CLA_ISO7816;
  dst.buf[1] = APDU_INS_READ_BINARY;
  dst.buf[2] = (uint8_t)((offset >> 8) & 0xFFU);
  dst.buf[3] = (uint8_t)(offset & 0xFFU);
  dst.buf[4] = le;
  return APDU_HEADER_SIZE;
}

size_t nfc_apdu_build_update_binary(nfc_apdu_buf_t dst, uint16_t offset, nfc_apdu_data_t data) {
  if (!dst.buf || !data.ptr || data.len == 0 || data.len > APDU_LE_MAX)
    return 0;
  if (dst.max < APDU_HEADER_SIZE + data.len)
    return 0;
  size_t pos = 0;
  dst.buf[pos++] = APDU_CLA_ISO7816;
  dst.buf[pos++] = APDU_INS_UPDATE_BINARY;
  dst.buf[pos++] = (uint8_t)((offset >> 8) & 0xFFU);
  dst.buf[pos++] = (uint8_t)(offset & 0xFFU);
  dst.buf[pos++] = (uint8_t)data.len;
  memcpy(&dst.buf[pos], data.ptr, data.len);
  pos += data.len;
  return pos;
}

size_t nfc_apdu_build_verify(nfc_apdu_buf_t dst, uint8_t p1, uint8_t p2, nfc_apdu_data_t data) {
  if (!dst.buf || !data.ptr || data.len == 0 || data.len > APDU_LE_MAX)
    return 0;
  if (dst.max < APDU_HEADER_SIZE + data.len)
    return 0;
  size_t pos = 0;
  dst.buf[pos++] = APDU_CLA_ISO7816;
  dst.buf[pos++] = APDU_INS_VERIFY;
  dst.buf[pos++] = p1;
  dst.buf[pos++] = p2;
  dst.buf[pos++] = (uint8_t)data.len;
  memcpy(&dst.buf[pos], data.ptr, data.len);
  pos += data.len;
  return pos;
}

size_t nfc_apdu_build_get_data(nfc_apdu_buf_t dst, uint8_t p1, uint8_t p2, uint8_t le) {
  if (!dst.buf || dst.max < APDU_HEADER_SIZE)
    return 0;
  dst.buf[0] = APDU_CLA_ISO7816;
  dst.buf[1] = APDU_INS_GET_DATA;
  dst.buf[2] = p1;
  dst.buf[3] = p2;
  dst.buf[4] = le;
  return APDU_HEADER_SIZE;
}

bool nfc_apdu_parse_fci(const uint8_t *fci, size_t fci_len, nfc_fci_info_t *out) {
  if (!fci || fci_len < 2U || !out)
    return false;
  memset(out, 0, sizeof(*out));

  size_t pos = 0;
  while (pos + 2U <= fci_len) {
    uint8_t tag = fci[pos++];
    uint8_t len = fci[pos++];
    if (pos + len > fci_len)
      break;

    if (tag == APDU_FCI_TAG_DF_NAME) {
      size_t copy = (len > sizeof(out->df_name)) ? sizeof(out->df_name) : len;
      memcpy(out->df_name, &fci[pos], copy);
      out->df_name_len = (uint8_t)copy;
      return true;
    }

    pos += len;
  }
  return false;
}
