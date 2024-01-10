#!/bin/bash

if [ -e "/lib/modules/6.3.13-060313-generic/kernel/drivers/net/wireless/rtl8852bu.ko" ]; then
    sudo rm -Rf /lib/modules/6.3.13-060313-generic/kernel/drivers/net/wireless/rtl8852bu.ko
fi
if [ -e "/lib/modules/6.3.13-060313-generic/kernel/drivers/net/wireless/rtl8852bu_ohd.ko" ]; then
    sudo rm -Rf /lib/modules/6.3.13-060313-generic/kernel/drivers/net/wireless/rtl8852bu_ohd.ko
fi