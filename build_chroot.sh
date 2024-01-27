#!/bin/bash
# This file is the install instruction for the CHROOT build
# We're using cloudsmith-cli to upload the file in CHROOT

sudo apt install -y python3-pip ruby ruby-dev rubygems build-essential
gem install fpm
sudo pip3 install --upgrade cloudsmith-cli --break-system-packages
ls -a
API_KEY=$(cat cloudsmith_api_key.txt)
DISTRO=$(cat distro.txt)
FLAVOR=$(cat flavor.txt)
REPO=$(cat repo.txt)
CUSTOM=$(cat custom.txt)
ARCH=$(cat arch.txt)

echo ${DISTRO}
echo ${FLAVOR}
echo ${CUSTOM}
echo ${ARCH}

if [[ -e /etc/os-release && $(grep -c "Raspbian" /etc/os-release) -gt 0 ]]; then
    sudo pip3 install --upgrade cloudsmith-cli
    echo "building for the raspberry pi"
    sudo apt update 
    sudo apt install -y build-essential flex bc bison dkms raspberrypi-kernel-headers
    echo "___________________BUILDING-DRIVER___________________"
    make KSRC=/usr/src/linux-headers-6.1.21-v7l+ O="" modules
    mkdir -p package/lib/modules/6.1.21-v7l+/kernel/drivers/net/wireless/
    cp *.ko package/lib/modules/6.1.21-v7l+/kernel/drivers/net/wireless/
    ls -a
    make clean
    make KSRC=/usr/src/linux-headers-6.1.21-v7+ O="" modules
    mkdir -p package/lib/modules/6.1.21-v7+/kernel/drivers/net/wireless/
    cp *.ko package/lib/modules/6.1.21-v7+/kernel/drivers/net/wireless/
    fpm -a armhf -s dir -t deb -n 88X2bu-rpi -v 2.5-evo-$(date '+%m%d%H%M') -C package -p 88X2bu-rpi.deb --before-install before-install-pi.sh --after-install after-install.sh
    echo "copied deb file"
    echo "push to cloudsmith"
    git describe --exact-match HEAD >/dev/null 2>&1
    echo "Pushing the package to OpenHD 2.5 repository"
    ls -a
    cloudsmith push deb --api-key "$API_KEY" openhd/release/raspbian/bullseye 88X2bu-rpi.deb || exit 1
else
sudo apt update 
sudo apt install -y build-essential flex bc bison dkms
make KSRC=/usr/src/linux-headers-6.3.13-060313-generic O="" modules
mkdir -p package/lib/modules/6.3.13-060313-generic/kernel/drivers/net/wireless/
cp *.ko package/lib/modules/6.3.13-060313-generic/kernel/drivers/net/wireless/
ls -a
fpm -a amd64 -s dir -t deb -n rtl88x2bu-x86 -v 2.5-evo-$(date '+%m%d%H%M') -C package -p rtl88x2bu-x86.deb --before-install before-install.sh --after-install after-install.sh

echo "copied deb file"
echo "push to cloudsmith"
git describe --exact-match HEAD >/dev/null 2>&1
echo "Pushing the package to OpenHD 2.5 repository"
ls -a
cloudsmith push deb --api-key "$API_KEY" openhd/release/ubuntu/lunar rtl88x2bu-x86.deb || exit 1
fi