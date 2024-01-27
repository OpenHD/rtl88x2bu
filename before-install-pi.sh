#!/bin/bash
if [ -e "/lib/modules/6.1.21-v7+/kernel/drivers/net/wireless/88X2bu.ko" ]; then
    sudo rm -Rf "/lib/modules/6.1.21-v7+/kernel/drivers/net/wireless/88X2bu.ko"
fi
if [ -e "/lib/modules/6.1.21-v7l+/kernel/drivers/net/wireless/88X2bu.ko" ]; then
    sudo rm -Rf "/lib/modules/6.1.21-v7+/kernel/drivers/net/wireless/88X2bu.ko"
fi
if [ -e "/lib/modules/6.1.21-v7l+/kernel/drivers/net/wireless/realtek/" ]; then
    sudo rm -Rf "/lib/modules/6.1.21-v7+/kernel/drivers/net/wireless/realtek/*"
fi