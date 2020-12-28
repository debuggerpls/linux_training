### Configuration for Beaglebone Black
---

Original information from [bootlin linux-kernel-labs][1]. Quick summary to work with Ethernet port and without Ethernet over USB.

Flashing U-Boot is required. Steps in `UBOOT.md`.

#### Setting up networking
---

Set board and PC host IPs:
```
# U-Boot

setenv ipaddr 192.168.0.100
setenv serverip 192.168.0.1
saveenv
```

TFTP configuration on PC host:
```
# PC host

sudo apt install tftpd-hpa 
# put something in /var/lib/tftpboot
```

Test TFTP:
```
# U-Boot

tftp 0x81000000 test.txt
md 0x81000000
```

#### Cross-compiling kernel
---

Install packages:
```
sudo apt install libssl-dev bison flex
sudo apt install gcc-arm-linux-gnueabi
```

Configure kernel:
```
make ARCH=arm omap2plus_defconfig

# add CONFIG_ROOT_NFS=y option

make -j8
```

Copy kernel image and device tree blob:
```
cp arch/arm/boot/zImage arch/arm/boot/zImage arch/arm/boot/dts/am335x-boneblack.dtb /var/lib/tftpboot
```

Install NFS server:
```
sudo apt install nfs-kernel-server
```

Edit /etc/exports to include:
```
# I think you can use buildroot instead of 192.168.0.100
/home/<user>/linux-kernel-labs/modules/nfsroot 192.168.0.100(rw,no_root_squash,no_subtree_check)
```

Restart NFS server:
```
sudo /etc/init.d/nfs-kernel-server restart
```

Set kernel bootargs and U-Boot bootcmd:
```
setenv bootargs root=/dev/nfs rw ip=192.168.0.100:${serverip}:192.168.0.1:255.255.255.0:buildroot:eth0:off console=ttyS0,115200n8 nfsroot=${serverip}:/home/<user>/linux-kernel-labs/modules/nfsroot,nfsvers=3,tcp

setenv bootcmd 'tftp 0x81000000 zImage; tftp 0x82000000 am335x-boneblack.dtb; bootz 0x81000000 - 0x82000000'

saveenv         
```

Additional infos about configs: [link][2] [link][3]


[1]: https://bootlin.com/doc/training/linux-kernel/linux-kernel-labs.pdf
[2]: http://wiki.dreamrunner.org/public_html/Embedded-System/U-BootOverview.html
[3]: https://support.criticallink.com/redmine/projects/arm9-platforms/wiki/Setting_a_Static_IP_Address
