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


sudo apt update 
sudo apt install -y build-essential flex bc bison dkms
make KSRC=/usr/src/linux-headers-6.3.13-060313-generic O="" modules
mkdir -p package/lib/modules/6.3.13-060313-generic/kernel/drivers/net/wireless/
cp *.ko package/lib/modules/6.3.13-060313-generic/kernel/drivers/net/wireless/
ls -a
fpm -a amd64 -s dir -t deb -n rtl8852bu-x86 -v 2.5-evo-$(date '+%m%d%H%M') -C package -p rtl8852bu-x86.deb --before-install before-install.sh

echo "copied deb file"
echo "push to cloudsmith"
git describe --exact-match HEAD >/dev/null 2>&1
echo "Pushing the package to OpenHD 2.5 repository"
ls -a
cloudsmith push deb --api-key "$API_KEY" openhd/release/ubuntu/lunar rtl8852bu-x86.deb || exit 1
