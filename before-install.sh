#!/bin/bash

if [ -e "/lib/modules/6.3.13-060313-generic/kernel/drivers/net/wireless/88X2bu.ko" ]; then
    sudo rm -Rf /lib/modules/6.3.13-060313-generic/kernel/drivers/net/wireless/88X2bu.ko
fi
if [ -e "/lib/modules/6.3.13-060313-generic/kernel/drivers/net/wireless/88X2bu_ohd.ko" ]; then
    sudo rm -Rf /lib/modules/6.3.13-060313-generic/kernel/drivers/net/wireless/88X2bu_ohd.ko
fi