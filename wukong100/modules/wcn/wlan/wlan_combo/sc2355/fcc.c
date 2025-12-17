/*
* SPDX-FileCopyrightText: 2020-2023 Unisoc (Shanghai) Technologies Co. Ltd
* SPDX-License-Identifier: GPL-2.0-only
*/

#include "common/common.h"
#include "common/npi.h"
#include "cmdevt.h"

static struct sprd_fcc_priv fcc_info;

static struct fcc_power_bo g_fcc_power_table[MAX_FCC_COUNTRY_NUM] = {
	{
		.country = "UY",
		.num = 4,
		.power_backoff = {
			/* subtype, channel, bw, {mode(2.4g : b,g,n,ac; 5g : a,n,ac), value} */
			{0, 1, 0, { {0, 1}, {1, 2}, {2, 3}, {3, 4}, {4, 5}, {5, 6}, {6, 7} } },
			{1, 2, 0, { {0, 1}, {1, 2}, {2, 3}, {3, 4}, {4, 5}, {5, 6}, {6, 7} } },
			{0, 1, 0, { {0, 1}, {1, 2}, {2, 3}, {3, 4}, {4, 5}, {5, 6}, {6, 7} } },
			{1, 4, 0, { {0, 1}, {1, 2}, {2, 3}, {3, 4}, {4, 5}, {5, 6}, {6, 7} } },
		},
	},
	{
		.country = "MX",
		.num = 4,
		.power_backoff = {
			/* subtype, channel, bw, {mode(2.4g : b,g,n,ac; 5g : a,n,ac), value} */
			{0, 5, 0, { {0, 1}, {1, 2}, {2, 3}, {3, 4}, {4, 5}, {5, 6}, {6, 7} } },
			{1, 6, 0, { {0, 1}, {1, 2}, {2, 3}, {3, 4}, {4, 5}, {5, 6}, {6, 7} } },
			{0, 7, 0, { {0, 1}, {1, 2}, {2, 3}, {3, 4}, {4, 5}, {5, 6}, {6, 7} } },
			{1, 8, 0, { {0, 1}, {1, 2}, {2, 3}, {3, 4}, {4, 5}, {5, 6}, {6, 7} } },

		},
	},
	{
		.country = "CN",
		.num = 8,
		.power_backoff = {
			/* subtype, channel, bw, {mode(2.4g : b,g,n,ac; 5g : a,n,ac), value} */
			{1,   1, 0, { {0, 127}, {1,  13}, {2,  12}, {3, 127}, {4, 127}, {5, 127}, {6, 127} } },
			{1,   2, 0, { {0, 127}, {1,  13}, {2,  12}, {3, 127}, {4, 127}, {5, 127}, {6, 127} } },
			{1,  10, 0, { {0, 127}, {1,  13}, {2,  12}, {3, 127}, {4, 127}, {5, 127}, {6, 127} } },
			{1,  11, 0, { {0, 127}, {1,  13}, {2,  12}, {3, 127}, {4, 127}, {5, 127}, {6, 127} } },
			{1,  36, 0, { {0, 127}, {1, 127}, {2, 127}, {3, 127}, {4, 127}, {5,  11}, {6,  10} } },
			{1,  64, 0, { {0, 127}, {1, 127}, {2, 127}, {3, 127}, {4, 127}, {5,  11}, {6,  10} } },
			{1, 100, 0, { {0, 127}, {1, 127}, {2, 127}, {3, 127}, {4, 127}, {5,  11}, {6,  10} } },
			{1, 140, 0, { {0, 127}, {1, 127}, {2, 127}, {3, 127}, {4, 127}, {5,  11}, {6,  10} } },
		},
	},
};

static int sc2355_fcc_fresh_bo(struct sprd_priv *priv, struct sprd_vif *vif, u8 channel, u8 bw, bool flag)
{
	struct sprd_power_backoff *p_backoff;
	struct fcc_power_bo *current_power_bo;
	int index;

	mutex_lock(&fcc_info.lock);
	fcc_info.flag = flag;
	if (priv->hif.hw_type == SPRD_HW_SC2355_SIPC) {
		if (flag && vif && vif->ctx_id < 2) {
			fcc_info.channel[vif->ctx_id] = channel;
			fcc_info.bw[vif->ctx_id] = bw;
		}
	} else {
		if (flag) {
			fcc_info.channel[0] = channel;
			fcc_info.bw[0]  = bw;
		}
        }

	current_power_bo = fcc_info.cur_power_bo;
	mutex_unlock(&fcc_info.lock);

	if (!current_power_bo) {
		wl_debug("current_power_bo is NULL, reset default!\n");
		p_backoff = NULL;
	} else {
		for (index = 0; index < current_power_bo->num; index++) {
			p_backoff = &current_power_bo->power_backoff[index];
			if (channel == p_backoff->channel &&
			    bw == p_backoff->bw) {
				wl_info("match channel : %hhu bw : %hhu\n",
					channel, bw);
				break;
			}
		}

		if (index == current_power_bo->num) {
			wl_info("do not match channel %hhu bw %hhu, reset default\n",
				channel, bw);
			p_backoff = NULL;
		}
	}

	atomic_set(&priv->power_back_off, 1);
	sc2355_set_power_backoff(priv, vif, p_backoff);
	atomic_set(&priv->power_back_off, 0);
	return 0;
}

void sc2355_fcc_fresh_bo_work(struct sprd_vif *vif, void *data, u16 len)
{
	struct fresh_bo_info *info = (struct fresh_bo_info *)data;
	sc2355_fcc_fresh_bo(vif->priv, vif, info->pw_channel, info->pw_bw, true);
}

void sc2355_fcc_match_country(struct sprd_priv *priv, const char *alpha2)
{
	bool found_country = false;
	bool need_refresh = false;
	struct fcc_power_bo *last_power_bo;
	int i;
	u8 channel[2] = {0}, bw[2] = {0};

	mutex_lock(&fcc_info.lock);
	for (i = 0; i < MAX_FCC_COUNTRY_NUM; i++) {
		if (g_fcc_power_table[i].country[0] == alpha2[0] &&
			g_fcc_power_table[i].country[1] == alpha2[1]) {
			wl_debug("matched fcc country %s!\n", alpha2);
			found_country = true;
			last_power_bo = fcc_info.cur_power_bo;
			fcc_info.cur_power_bo = &g_fcc_power_table[i];
			/* handle alpha2 change after connected */
			if (last_power_bo && last_power_bo != fcc_info.cur_power_bo)
				fcc_info.flag = true;
			/* handle set regdom just after connected */
			if (fcc_info.flag) {
				need_refresh = true;
				channel[0] = fcc_info.channel[0];
				channel[1] = fcc_info.channel[1];
				bw[0] = fcc_info.bw[0];
				bw[1] = fcc_info.bw[1];
				wl_debug("evt_fresh_backoff had came, now fresh it!\n");
			}
			break;
		}
	}

	if (!found_country) {
		wl_debug("not fcc country, need reset fcc power\n");
		fcc_info.cur_power_bo = NULL;
	}
	mutex_unlock(&fcc_info.lock);

	if (!found_country || need_refresh) {
		  if (priv->hif.hw_type == SPRD_HW_SC2355_SIPC)
			sc2355_fcc_fresh_all_mode_bo(priv, channel, bw);
		  else
			sc2355_fcc_fresh_bo(priv,NULL, channel[0], bw[0], false);
	}
}

void sc2355_fcc_fresh_scan_bo(struct sprd_priv *priv)
{
        struct sprd_power_backoff *p_backoff, *backoff_data, *tmp;
        struct fcc_power_bo *current_power_bo;
        int index, num = 0;

        mutex_lock(&fcc_info.lock);
        current_power_bo = fcc_info.cur_power_bo;
        mutex_unlock(&fcc_info.lock);

	backoff_data = (struct sprd_power_backoff *)kzalloc(sizeof(struct sprd_power_backoff)*MAX_POWER_BACKOFF_RULE, GFP_KERNEL);

	if (!backoff_data) {
		wl_err("%s alloc data fail\n", __func__);
		return;
	}

	if (!current_power_bo) {
                wl_debug("current_power_bo is NULL, reset default!\n");
                p_backoff = NULL;
        } else {
		tmp = backoff_data;
                for (index = 0; index < current_power_bo->num; index++) {
                        p_backoff = &current_power_bo->power_backoff[index];
                        if (0 != p_backoff->subtype &&
                            0 == p_backoff->bw) {
                               memcpy(tmp, p_backoff, sizeof(struct sprd_power_backoff));
			       num++;
			       tmp++;
                        }
                }

        }

        atomic_set(&priv->power_back_off, 1);
        sc2355_set_scan_power_backoff(priv, NULL, backoff_data, num);
        atomic_set(&priv->power_back_off, 0);
	kfree(backoff_data);
	backoff_data = NULL;
        return;
}

void sc2355_fcc_fresh_all_mode_bo(struct sprd_priv *priv, u8 *channel, u8 *bw)
{
        struct sprd_vif *vif;

	if (channel[0]) {
		vif =  sc2355_ctxid_to_vif(priv, 0);
		if (vif && vif->mode == SPRD_MODE_STATION)
			sc2355_fcc_fresh_bo(priv, vif, channel[0], bw[0], false);
		if (vif)
			sprd_put_vif(vif);
	}

	if (channel[1]) {
		vif = sc2355_ctxid_to_vif(priv, 1);
		if (vif && (vif->mode == SPRD_MODE_AP ||
			vif->mode == SPRD_MODE_P2P_CLIENT ||
			vif->mode == SPRD_MODE_P2P_GO))
			sc2355_fcc_fresh_bo(priv, vif, channel[1], bw[1], false);
		if (vif)
			sprd_put_vif(vif);
	}

        return;
}

void sc2355_fcc_reset_bo(struct sprd_priv *priv, struct sprd_vif *vif)
{
	mutex_lock(&fcc_info.lock);
	fcc_info.flag = false;

	if (priv->hif.hw_type == SPRD_HW_SC2355_SIPC) {
		if (vif && vif->ctx_id < 2) {
			fcc_info.channel[vif->ctx_id] = 0;
			fcc_info.bw[vif->ctx_id] = 0;
		}
	} else {
		fcc_info.channel[0] = 0;
		fcc_info.bw[0] = 0;
	}
	mutex_unlock(&fcc_info.lock);
}

void sc2355_fcc_init(void)
{
	fcc_info.flag = false;
	fcc_info.channel[0] = 0;
	fcc_info.channel[1] = 0;
	fcc_info.bw[0] = 0;
	fcc_info.bw[1] = 0;
	mutex_init(&fcc_info.lock);
}

u8 sc2355_pw_backoff_band2value(u8 channel)
{
	u8 value = 0;

	if (!channel)
		return value;
	mutex_lock(&g_set_5g_sar_info.lock);
	g_set_5g_sar_info.channel = channel;
	switch (channel) {
	case 30 ... 50:
		value = g_set_5g_sar_info.value[0];
		break;
	case 51 ... 70:
		value = g_set_5g_sar_info.value[1];
		break;
	case 71 ... 145:
		value = g_set_5g_sar_info.value[2];
		break;
	case 146 ... 170:
		value = g_set_5g_sar_info.value[3];
		break;
	default:
		value = g_set_5g_sar_info.value[4];
		break;
	}
	mutex_unlock(&g_set_5g_sar_info.lock);
	return value;

}

void sc2355_init_5g_sar_info(void)
{
	mutex_init(&g_set_5g_sar_info.lock);
	g_set_5g_sar_info.channel = 0;
	g_set_5g_sar_info.band_sar_supported = false;
	memset(g_set_5g_sar_info.value, 0x00, 5);
}

void sc2355_reset_5g_sar_info(void)
{
	mutex_lock(&g_set_5g_sar_info.lock);
	g_set_5g_sar_info.channel = 0;
	mutex_unlock(&g_set_5g_sar_info.lock);
}
void sc2355_set_5g_sar_info(struct sprd_priv *priv, struct sprd_vif * vif, u8 *data, u16 len)
{

	sc2355_set_band_sar_value(priv, vif, data, len, SPRD_SET_SAR_RELATIVE);
	mutex_lock(&g_set_5g_sar_info.lock);
	if (data == NULL)
		memset(g_set_5g_sar_info.value, 0x00, 5);
	else
		memcpy(g_set_5g_sar_info.value, data, 5);
	wl_info("%s band sar: %d, %d, %d, %d, %d\n", __func__,
			g_set_5g_sar_info.value[0],
			g_set_5g_sar_info.value[1],
			g_set_5g_sar_info.value[2],
			g_set_5g_sar_info.value[3],
			g_set_5g_sar_info.value[4]);

	mutex_unlock(&g_set_5g_sar_info.lock);
}


int sc2355_set_reduce_power(struct sprd_vif *vif, unsigned char *data, unsigned short len)
{
	struct reduce_power *para = NULL;
	int ret = 0;

	if (len > sizeof(struct reduce_power)) {
		wl_err("%s Invalid s_buf len:%d\n", __func__, len);
		return -EINVAL;
	}

	para = (struct reduce_power *)data;
	if (para->type < SET_SAR_POWER || para->type >= REDUCE_POWER_SUBTYPE_MAX) {
		wl_err("data type error :%d\n", para->type);
		return -1;
	}

	switch (para->type) {
	case SET_APC_STATUS:
	case SET_APC_MODE:
	case SET_CLIENT_POWER:
		wl_info("%s set sar cmd!\n", __func__);
		ret = sc2355_send_reduce_power_cmd(vif->priv, vif, para, NULL, NULL);
		break;
	default:
		break;
	}

	return ret;
}

int sc2355_get_reduce_power(struct sprd_vif *vif, unsigned char *data, unsigned short len,
			    unsigned char *rbuf, unsigned short *r_len)
{
	struct reduce_power *para = NULL;
	int ret = 0;

	if (len > sizeof(struct reduce_power)) {
		wl_err("%s Invalid s_buf len:%d\n", __func__, len);
		return -EINVAL;
	}

	para = (struct reduce_power *)data;
	if (para->type < SET_SAR_POWER || para->type >= REDUCE_POWER_SUBTYPE_MAX) {
		wl_err("data type error :%d\n", para->type);
		return -1;
	}

	switch (para->type) {
	case GET_CLIENT_POWER:
	case GET_APC_MODE:
		wl_info("%s get sar cmd!\n", __func__);
		ret = sc2355_send_reduce_power_cmd(vif->priv, vif, para, rbuf, r_len);
	default:
		break;
	}

	return ret;
}
