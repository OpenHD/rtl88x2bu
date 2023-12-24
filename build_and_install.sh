#bin/bash

# for whatever reason, we need to run make 2 times to get the .ko
make
make

# now remove the bad in-kernel driver (fuck you linux for this bullshit)
# if it is not loaded / already removed, this has no effect
# (and an error here can be ignored)
sudo rmmod rtw88_8822bu

# and install the good one
# NOTE Here I remove then re-install the driver, since during development, I also use this script
sudo rmmod 88x2bu_ohd.ko
sudo insmod 88x2bu_ohd.ko

echo "88x2bu_ohd driver installed"