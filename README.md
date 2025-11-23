# REALTEK RTL88x2B USB Linux Driver
**Current Driver Version**: 5.13.1
**Support Kernel**: 2.6.24 ~ 6.0 (with unofficial patches)

Linux in-tree rtw8822bu driver is working in process, check [this](https://lore.kernel.org/lkml/20220518082318.3898514-1-s.hauer@pengutronix.de/) patchset.

Official release note please check ReleaseNotes.pdf

**Note:** if you believe your device is **RTL8812BU** or **RTL8822BU** but after loaded the module no NIC shows up, the device ID maybe not in the driver whitelist. In this case please submit a new issue with `lsusb` result, and your device name, brand, website, etc.

## Linux 5.18+ and RTW88 Driver
Starting from Linux 5.18, some distributions have added experimental RTW88 USB support (include RTW88x2BU support).
It is not yet stable but if it works well on your system, then you no longer need this driver.
But if it doesn't work or is unstable, you need to manually blacklist it because it has a higher loading priority than this external drivers.

Check the currently loaded module using `lsmod`. If you see `rtw88_core`, `rtw88_usb`, or any name beginning with `rtw88_` then you are using the RTW88 driver.
If you see `88x2bu` then you are using this RTW88x2BU driver.

To blacklist RTW88 8822bu USB driver, run the following command:

```
echo "blacklist rtw88_8822bu" > /etc/modprobe.d/rtw8822bu.conf
```

And reboot your system.

## Supported Devices
<details>
  <summary>
    ASUS
  </summary>

* ASUS AC1300 USB-AC55 B1
* ASUS U2
* ASUS USB-AC53 Nano
* ASUS USB-AC58
</details>

<details>
  <summary>
    Dlink
  </summary>

* Dlink - DWA-181
* Dlink - DWA-182
* Dlink - DWA-183 D Version
* Dlink - DWA-185
* Dlink - DWA-T185
</details>

<details>
  <summary>
    Edimax
  </summary>

* Edimax EW-7822ULC
* Edimax EW-7822UTC
* Edimax EW-7822UAD
</details>

<details>
  <summary>
    NetGear
  </summary>

* NetGear A6150
</details>

<details>
  <summary>
    TP-Link
  </summary>

* TP-Link Archer T3U
* TP-Link Archer T3U Plus
* TP-Link Archer T4U V3
* TP-Link Archer T4U Plus
</details>

<details>
  <summary>
    TRENDnet
  </summary>

* TRENDnet TEW-808UBM
</details>

<details>
  <summary>
    ZYXEL
  </summary>

* ZYXEL NWD6602
</details>


And more.

# How to use this kernel module
* Ensure you have C compiler & toolchains, e.g. `build-essential` for Debian/Ubuntu, `base-devel` for Arch, etc.
* Make sure you have installed the corresponding kernel headers
* All commands need to be run in the driver directory
* You need rebuild the kernel module everytime you update/change the kernel if you are not using DKMS


## Manual installation
### Clean
* Make sure you cleaned old build files before builds new one
```
make clean
```

### Building module for current running kernel
```
make
```

### Building module for other kernels
```
make KSRC=/lib/modules/YOUR_KERNEL_VERSION/build
```

### Installing
```
sudo make install
```

### Uninstalling
```
sudo make uninstall
```

## Unlocked channel control via procfs
The RF synthesizer can cover a wider range than typical 5GHz Wi-Fi channels (roughly 5080MHz ~ 6165MHz). A procfs helper lets you retune to those custom center frequencies without changing user-space tools.

To view usage details:
```
cat /proc/net/rtl88x2bu/<wlan>/monitor_chan_override
```

Usage: `echo "<chan> <bw>" > monitor_chan_override`

* `chan`: 16~253. The center frequency is calculated as `channel*5+5000` MHz.
* `bw`: 10/20/40/80 MHz. This value should match the bandwidth you configure via `iw`.

Examples:

1. Transmit at 6005MHz with 10MHz BW:
   * set the interface BW to 10MHz with `iw`
   * configure your injection tool (e.g., `wfb-ng`) for 20MHz radiotap
   * `echo "201 10" > /proc/net/rtl88x2bu/<wlan>/monitor_chan_override`

2. Transmit at 5080MHz with 20MHz BW:
   * set the interface BW to 20MHz with `iw`
   * configure your injection tool for 20MHz radiotap
   * `echo "16 20" > /proc/net/rtl88x2bu/<wlan>/monitor_chan_override`

3. Transmit at 5255MHz with 40MHz BW:
   * set the interface BW to HT40 with `iw`
   * configure your injection tool for 40MHz radiotap
   * `echo "51 40" > /proc/net/rtl88x2bu/<wlan>/monitor_chan_override`

Disclaimer: some chips may not lock every frequency. Use at your own risk and comply with local regulations.

## Manual DKMS installation
```
git clone "https://github.com/RinCat/RTL88x2BU-Linux-Driver.git" /usr/src/rtl88x2bu-git
sed -i 's/PACKAGE_VERSION="@PKGVER@"/PACKAGE_VERSION="git"/g' /usr/src/rtl88x2bu-git/dkms.conf
dkms add -m rtl88x2bu -v git
dkms autoinstall
```

# USB 3.0 Support
You can try use `modprobe 88x2bu rtw_switch_usb_mode=1` to force the adapter run under USB 3.0. But if your adapter/port/motherboard not support it, the driver will be in restart loop. Remove the parameter and reload the driver to restore. Alternatively, `modprobe 88x2bu rtw_switch_usb_mode=2` let\'s it run as USB 2 device.

Notice: If you had already loaded the moduel, use `modprobe -r 88x2bu` to unload it first.

If you want to force a given mode permanently (even when switching the adapter across devices), create the file `/etc/modprobe.d/99-RTL88x2BU.conf` with the following content:
`options 88x2bu rtw_switch_usb_mode=1`


# Debug
Set debug log use `echo 5 > /proc/net/rtl88x2bu/log_level` or `modprobe 88x2bu rtw_drv_log_level=5`

# Distribution
* Archlinux AUR https://aur.archlinux.org/packages/rtl88x2bu-dkms-git/
