static ssize_t proc_set_thermal_state(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct registry_priv *pregpriv = &padapter->registrypriv;
	HAL_DATA_TYPE *pHalData = GET_HAL_DATA(padapter);
	char tmp[32];
	u32 offset_temp;

	if (!padapter)
		return -EFAULT;

	if (count < 1) {
		RTW_INFO("Set thermal_state Argument error. \n");
		return -EFAULT;
	}

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {
		int num = sscanf(tmp, "%u", &offset_temp);
		if (num < 1)
			return count;
	}

	if (offset_temp > 70) {
		RTW_INFO("Set thermal_state Argument range error. \n");
		return -EFAULT;
	}

	RTW_INFO("Write to thermal_state offset tempC : %d\n", offset_temp);
	pHalData->eeprom_thermal_offset_temperature = (u8)offset_temp;

	return count;
}


static int proc_get_thermal_state(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	HAL_DATA_TYPE *pHalData = GET_HAL_DATA(padapter);

	u8 rx_cnt = 0; /* rf_type_to_rf_rx_cnt(pHalData->rf_type); */
	int thermal_value = 0;
	int thermal_offset = 0;
	int temperature_offset = 32; /* measured value. see comment in commit/5b7a66d for details */
	int temperature = 0;
	u32 thermal_reg_mask = 0;
	int rf_path = 0;

	if (IS_8822C_SERIES(GET_HAL_DATA(padapter)->version_id)
		|| IS_8723F_SERIES(GET_HAL_DATA(padapter)->version_id)
		/*|| IS_8822E_SERIES(GET_HAL_DATA(padapter)->version_id)*/)
		thermal_reg_mask = 0x007e;	/*0x42: RF Reg[6:1], 35332(themal K  & bias k & power trim) & 35325(tssi )*/
	else
		thermal_reg_mask = 0xfc00;	/*0x42: RF Reg[15:10]*/

	temperature_offset = (pHalData->eeprom_thermal_offset_temperature == 0) ?
			     temperature_offset : pHalData->eeprom_thermal_offset_temperature;

	if (pHalData->rf_type == RF_1T1R)
		rx_cnt = 1;
	else if (pHalData->rf_type == RF_1T2R || pHalData->rf_type == RF_2T2R)
		rx_cnt = 2;
	else if (pHalData->rf_type == RF_2T3R || pHalData->rf_type == RF_3T3R)
		rx_cnt = 3;
	else if (pHalData->rf_type == RF_2T4R || pHalData->rf_type == RF_3T4R || pHalData->rf_type == RF_4T4R)
		rx_cnt = 4;

	for (rf_path = 0; rf_path < rx_cnt; rf_path++) {
		/* need to manually trigger the ADC conversion for latest data */
		phy_set_rf_reg(padapter, rf_path, 0x42, BIT19, 0x1);
		phy_set_rf_reg(padapter, rf_path, 0x42, BIT19, 0x0);
		phy_set_rf_reg(padapter, rf_path, 0x42, BIT19, 0x1);

		rtw_usleep_os(15); /* 15us in halrf_get_thermal_8822e() */

		thermal_value = phy_query_rf_reg(padapter, rf_path, 0x42, thermal_reg_mask);
		/* thermal_offset = pHalData->eeprom_thermal_meter_multi[rf_path]; */
		/* 88x2bu does not seem to have eeprom_thermal_meter_multi */
		thermal_offset = pHalData->eeprom_thermal_meter;
		temperature = (((thermal_value - thermal_offset) * 5) / 2) + temperature_offset;
		RTW_PRINT_SEL(m, "rf_path: %d, thermal_value: %d, offset: %d, temperature: %d\n", rf_path, thermal_value, thermal_offset, temperature);
	}

	return 0;
}

static int proc_get_edcca_threshold_jaguar3_override(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct registry_priv	*pregpriv = &padapter->registrypriv;

	RTW_PRINT_SEL(m, "Set EDCCA Threshold Override For Realtek Jaguar3 Series (8723F/8735B/8730A/8822C/8812F/8197G/8822E/8198F/8814B/8814C)\n");
	RTW_PRINT_SEL(m, "Github: libc0607/rtl88x2eu-20230815\n");
	RTW_PRINT_SEL(m, "\n");
	RTW_PRINT_SEL(m, "Usage: echo \"<en> <dBm_l2h>\" > edcca_threshold_jaguar3_override\n");
	RTW_PRINT_SEL(m, "\ten: 0-disable, 1-enable\n");
	RTW_PRINT_SEL(m, "\tdBm_l2h: Energy CCA level in dBm, range: [-100, 7]; it's -72dBm (0xB0-248) by default from my adaptor's register.\n");
	RTW_PRINT_SEL(m, "I don't know the actual range since I don't have the detailed datasheet with register definitions, but in theory it can be tuned in +7dBm ~ -240dBm. A threshold below -100dBm seems make no sense so I manually disabled them.\n");
	RTW_PRINT_SEL(m, "Note: the H2L value is automatically set with 8dBm (default) lower as hysteresis.\n");
	RTW_PRINT_SEL(m, "Note 2: It's a bit similar to the 'thresh62' patch in ath9k.\n");
	RTW_PRINT_SEL(m, "\n");
	RTW_PRINT_SEL(m, "e.g.  To set the threshold to l2h=-40dBm (then h2l=-48dBm), use \n");
	RTW_PRINT_SEL(m, "\techo \"1 -40\" > edcca_threshold_jaguar3_override\n");
	RTW_PRINT_SEL(m, "e.g.  To disable, use \n");
	RTW_PRINT_SEL(m, "\techo \"0 <any_number>\" > edcca_threshold_jaguar3_override\n");
	RTW_PRINT_SEL(m, "\n");
	RTW_PRINT_SEL(m, "Disclaimer: There's no guarantee on performance. \n");
	RTW_PRINT_SEL(m, "This operation may damage your hardware.\n");
	RTW_PRINT_SEL(m, "You should obey the law, and use it at your own risk.\n");
	RTW_PRINT_SEL(m, "\n");
	RTW_PRINT_SEL(m, "Current value: %u %d\n", pregpriv->edcca_thresh_override_en, pregpriv->edcca_thresh_l2h_override);

	return 0;
}

static ssize_t proc_set_edcca_threshold_jaguar3_override(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct registry_priv *pregpriv = &padapter->registrypriv;
	char tmp[32];
	s32 edcca_thresh = -72;
	u32 edcca_thresh_en = 0;

	if (!padapter)
		return -EFAULT;

	if (count < 2) {
		RTW_INFO("edcca_threshold_jaguar3_override Argument error. \n");
		return -EFAULT;
	}

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {
		int num = sscanf(tmp, "%u %d", &edcca_thresh_en, &edcca_thresh);
		if (num < 1)
			return count;
	}

	if (edcca_thresh > 7 || edcca_thresh < -100) {
		RTW_INFO("edcca_threshold_jaguar3_override dBm_l2h out of range: %d\n", edcca_thresh);
		return count;
	}

	if (edcca_thresh_en < 0 || edcca_thresh_en > 1) {
		RTW_INFO("edcca_threshold_jaguar3_override en out of range: %d\n", edcca_thresh_en);
		return count;
	}

	if (edcca_thresh_en == 0) {
		edcca_thresh = -72;	// set a default value here
	}

	RTW_INFO("Write to edcca_threshold_jaguar3_override: EDCCA override %s, L2H threshold = %ddBm\n", (edcca_thresh_en==1)? "enabled": "disabled", edcca_thresh);

	pregpriv->edcca_thresh_override_en = (u8)edcca_thresh_en;
	pregpriv->edcca_thresh_l2h_override = (s8)edcca_thresh;

	return count;
}

static int proc_get_slottime_override(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct registry_priv	*pregpriv = &padapter->registrypriv;
	struct mlme_ext_priv *pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info *pmlmeinfo = &(pmlmeext->mlmext_info);

	RTW_PRINT_SEL(m, "Slot Time Override\n");
	RTW_PRINT_SEL(m, "Github: libc0607/rtl88x2eu-20230815\n");
	RTW_PRINT_SEL(m, "\n");
	RTW_PRINT_SEL(m, "See DOI: 10.1109/TMC.2010.27 for why we need tuning this.\n");
	RTW_PRINT_SEL(m, "\n");
	RTW_PRINT_SEL(m, "Usage: echo \"<en> <slottime>\" > slottime_override\n");
	RTW_PRINT_SEL(m, "en:			0-disable, 1-enable\n");
	RTW_PRINT_SEL(m, "slottime_override:	us\n");
	RTW_PRINT_SEL(m, "\n");
	RTW_PRINT_SEL(m, "e.g.  \n");
	RTW_PRINT_SEL(m, "\techo \"1 5\" > slottime_override\n");
	RTW_PRINT_SEL(m, "\techo \"0 <any_number>\" > slottime_override\n");
	RTW_PRINT_SEL(m, "\n");
	RTW_PRINT_SEL(m, "Disclaimer: There's no guarantee on performance. \n");
	RTW_PRINT_SEL(m, "This operation may damage your hardware.\n");
	RTW_PRINT_SEL(m, "You should obey the law, and use it at your own risk.\n");
	RTW_PRINT_SEL(m, "\n");
	RTW_PRINT_SEL(m, "Current value: %u %u\n", pmlmeinfo->slottime_override_en, pmlmeinfo->slottime_override);

	return 0;
}

static ssize_t proc_set_slottime_override(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct mlme_ext_priv *pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info *pmlmeinfo = &(pmlmeext->mlmext_info);
	char tmp[32];
	u32 en, slottime;

	if (!padapter)
		return -EFAULT;

	if (count < 2) {
		RTW_INFO("slottime_override Argument error. \n");
		return -EFAULT;
	}

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {
		int num = sscanf(tmp, "%u %u", &en, &slottime);
		if (num < 1)
			return count;
	}

	if (slottime < 0 || en < 0 || en > 1) {
		RTW_INFO("out of range: %d %d\n", en, slottime);
		return count;
	}

	if (en == 0) {
		slottime = 9; // should be the default value
	}

	pmlmeinfo->slottime_override = slottime;
	pmlmeinfo->slottime_override_en = en;
	//rtw_hal_set_hwreg(padapter, HW_VAR_SLOT_TIME, (u8 *)(&slottime));

	RTW_INFO("Write to slottime_override: %u, %u\n", en, slottime);

	return count;
}

static int proc_get_sifs_override(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct registry_priv	*pregpriv = &padapter->registrypriv;
	struct mlme_ext_priv *pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info *pmlmeinfo = &(pmlmeext->mlmext_info);

	RTW_PRINT_SEL(m, "SIFS Override\n");
	RTW_PRINT_SEL(m, "Github: libc0607/rtl88x2eu-20230815\n");
	RTW_PRINT_SEL(m, "\n");
	RTW_PRINT_SEL(m, "Usage: echo \"<en> <sifs>\" > sifs_override\n");
	RTW_PRINT_SEL(m, "en: 0-disable, 1-enable\n");
	RTW_PRINT_SEL(m, "sifs: SIFS time, in us\n");
	RTW_PRINT_SEL(m, "\n");
	RTW_PRINT_SEL(m, "e.g.  \n");
	RTW_PRINT_SEL(m, "\techo \"1 16\" > sifs_override\n");
	RTW_PRINT_SEL(m, "\techo \"0 <any_number>\" > sifs_override\n");
	RTW_PRINT_SEL(m, "\n");
	RTW_PRINT_SEL(m, "Disclaimer: There's no guarantee on performance. \n");
	RTW_PRINT_SEL(m, "This operation may damage your hardware.\n");
	RTW_PRINT_SEL(m, "You should obey the law, and use it at your own risk.\n");
	RTW_PRINT_SEL(m, "\n");
	RTW_PRINT_SEL(m, "Current value: %u %u\n", pmlmeinfo->sifs_override_en, pmlmeinfo->sifs_override);

	return 0;
}

static ssize_t proc_set_sifs_override(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct mlme_ext_priv *pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info *pmlmeinfo = &(pmlmeext->mlmext_info);
	char tmp[32];
	u32 sifs_override, sifs_override_en;

	if (!padapter)
		return -EFAULT;

	if (count < 2) {
		RTW_INFO("sifs_override Argument error. \n");
		return -EFAULT;
	}

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {
		int num = sscanf(tmp, "%u %u", &sifs_override_en, &sifs_override);
		if (num < 1)
			return count;
	}

	if (sifs_override < 0 || sifs_override_en < 0 || sifs_override_en > 1) {
		RTW_INFO("out of range: %u %u\n", sifs_override_en, sifs_override);
		return count;
	}

	if (sifs_override_en == 0) {
		sifs_override = 16;
	}

	pmlmeinfo->sifs_override = sifs_override;
	pmlmeinfo->sifs_override_en = sifs_override_en;

	RTW_INFO("Write to sifs_override: %u, %u\n", sifs_override_en, sifs_override);

	return count;
}

static int proc_get_dis_cca(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct registry_priv	*pregpriv = &padapter->registrypriv;
	struct dm_struct *dm;
	u32 bit_dis_cca;

	if (!padapter)
		return -EFAULT;

	dm = adapter_to_phydm(padapter);

	bit_dis_cca = odm_get_mac_reg(dm, R_0x520, BIT(15));

	RTW_PRINT_SEL(m, "BIT_DIS_EDCCA = %d, CCA %s\n", bit_dis_cca, bit_dis_cca? "disabled": "enabled");

	return 0;
}

static ssize_t proc_set_dis_cca(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct registry_priv	*pregpriv = &padapter->registrypriv;
	struct dm_struct *dm;
	dm = adapter_to_phydm(padapter);

	char tmp[32];
	u32 en;

	if (!padapter)
		return -EFAULT;

	if (count < 1) {
		RTW_INFO("Set dis_cca Argument error.\n");
		return -EFAULT;
	}

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {
		int num = sscanf(tmp, "%u", &en);
		if (num < 1)
			return count;
	}

	if (en != 0 && en != 1) {
		RTW_INFO("Set dis_cca Argument range error.\n");
		return -EFAULT;
	}

	if (en == 1) {
		// mac bit_dis_edcca
		odm_set_mac_reg(dm, R_0x520, BIT(15), 1);
		// mac bit_edcca_msk_countdown
		odm_set_mac_reg(dm, R_0x524, BIT(11), 0);
		// bb cck cca
		odm_set_bb_reg(dm, R_0x1a9c, BIT(20), 0x0);
		odm_set_bb_reg(dm, R_0x1a14, 0x300, 0x3);
		// bb ofdm cca
		odm_set_bb_reg(dm, R_0x1d58, 0xff8, 0x1ff);
	} else {
		// mac bit_dis_edcca
		odm_set_mac_reg(dm, R_0x520, BIT(15), 0);
		// mac bit_edcca_msk_countdown
		odm_set_mac_reg(dm, R_0x524, BIT(11), 1);
		// bb cck cca
		odm_set_bb_reg(dm, R_0x1a9c, BIT(20), 0x1);
		odm_set_bb_reg(dm, R_0x1a14, 0x300, 0x0);
		// bb ofdm cca
		odm_set_bb_reg(dm, R_0x1d58, 0xff8, 0x0);
	}


	RTW_INFO("Write to dis_cca: %d, %s cca\n", en, (en==1)? "disabled": "enabled");

	return count;
}

static int proc_get_single_tone(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct registry_priv	*pregpriv = &padapter->registrypriv;
	struct dm_struct *dm;
	u32 bit_dis_cca;

	if (!padapter)
		return -EFAULT;

	dm = adapter_to_phydm(padapter);


	RTW_PRINT_SEL(m, "single_tone: <en:0(dis)/1(en)> <rf_path:0(A)/1(B)/4(AB)>\n");

	return 0;
}

static ssize_t proc_set_single_tone(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	struct registry_priv	*pregpriv = &padapter->registrypriv;
	struct dm_struct *dm;
	dm = adapter_to_phydm(padapter);

	char tmp[32];
	u32 en, rf_path;
	int num;

	if (!padapter)
		return -EFAULT;

	if (count < 2) {
		RTW_INFO("Set single_tone Argument error.\n");
		return -EFAULT;
	}

	if (count > sizeof(tmp)) {
		rtw_warn_on(1);
		return -EFAULT;
	}

	if (buffer && !copy_from_user(tmp, buffer, count)) {
		num = sscanf(tmp, "%u %u", &en, &rf_path);
		if (num < 1)
			return count;
	}

	if (rf_path != RF_PATH_A && rf_path != RF_PATH_B && rf_path != RF_PATH_AB) {
		RTW_INFO("Set single_tone rf_path Argument range error.\n");
		return -EFAULT;
	}

	if (en != 0 && en != 1) {
		RTW_INFO("Set single_tone en Argument range error.\n");
		return -EFAULT;
	}

	if (en == 1) {
		phydm_mp_set_single_tone(dm, true, rf_path);
	} else {
		phydm_mp_set_single_tone(dm, false, rf_path);
