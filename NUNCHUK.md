### Nunchuk device driver
---

* Create `am335x-customboneblack.dts` file in arch/arm/boot/dts/

* Add the .dtb file to Makefile bellow `boneblack.dtb`

* `i2c1` is defined in `am33xx-l4.dtsi`

* Change i2c1 `status="okay"` and set `clock-frequency=<100000>` (100kHz)

* Add nunchuck as i2c1 bus device with `reg=52` and `compatible= "nintendo,nunchuk"`

* Change uboot bootcmd to:
```
bootcmd=tftp 0x81000000 zImage; tftp 0x82000000 am335x-customboneblack.dtb; bootz 0x81000000 - 0x82000000
```

* Check presence of nunchuk node
```
# find /sys/firmware/devicetree -name "*nunchuk*"

/sys/firmware/devicetree/base/ocp/interconnect@48000000/segment@0/target-mo
dule@2a000/i2c@0/nunchuk@52/
```

* Base address of I2C1 controller registers can be calculated from above line
```
0x48000000 + 0x2a000 = 0x4802a000
```

* Check whole structure of loaded Device Tree
```
# dtc -I fs /sys/firmware/devicetree/base/ > /tmp/dts
```

* Kermit:
```
kermit
set line /dev/ttyUSB0
set speed 115200
connect
```

* Nunchuk driver can be found under
```
/sys/module/nunchuk/drivers/i2c\:nunchuk_i2c/
```

* Nunchuk device can be found under
```
/sys/bus/i2c/devices/i2c-1/1-0052/
``` 

* Both driver and device contains symlinks to each other

* Communicating with I2C device can be done using: raw API, message transfer and SMBus calls. SMBus is a subset of I2C protocol and `should` be used instead of raw API (provided that I2C device supports it). Thus, the driver can then be used on both SMBus and I2C adapters. 

* Not all I2C controllers support all functionalities. Controller drivers tell I2C core which functionalities are supported. Device drivers must check if needed functionalities are provided using `i2c_check_functionality()` function. Full list can be found `include/uapi/linux/i2c.h`.

* Since Linux 3.2, `pinctrl` subsystem has been added for pin muxing (`drivers/pinctrl/`). It offers pin muxing consumer interface for device drivers (and drivers interface that configures the muxing). Most pinctrl drivers provide a Device Tree binding and pin muxing MUST be described in the DT. `Documentation/devicetree/bindings/pinctrl` documents each binding.

* Add as patch to kernel
```
git add <files>
git commit -as

git format-patch 5.9.y

# to change last commit message
git commit --amend
git format-patch 5.9.y
```

* BeagleBone Black System Reference Manual Page 86 has P9 Pinout.
```
PIN	PROC 	NAME 		MODE
17	A16	I2C1_SCL	MODE2
18	B16	I2C1_SDA	MODE2
1,2		GND
3,4		DC_3.3V
```

* ZCZ Package for Beaglebone Black. A16 & B16. A16 has pin name SPI0_CS0. B16 has pin name SPI0_D1.

* L4_WKUP Peripheral Memory Map. You will find a table containing a Control Module Registers entry with its address:0x44E1_0000. Register offsets that control muxing: conf_spi0_cs0 - 0x95C ; conf_spi0_d1 - 0x958 . 

* Disable dmesg messages to console
```
dmesg -n 1 

# better to do this
dmesg -D # console off
dmesg -E # console on

# check current log level
cat /proc/sys/kernel/printk
```

* I2C bus tests
```
# check adapters
i2cdetect -l

# check funtionalities of i2c1 (bus 1)
i2cdetect -F 1

# if no SMBus Quick Commands are available, run which uses usual set of i2c cmds
i2cdetect -r 1 
```

* `udelay()` is active wait, `usleep_range()` lets cpu do other things while waiting.

* General rule: whenever the symbol you’re looking for is defined in `arch/<arch>/include/asm/<file>.h`, you can include `linux/<file>.h` in your kernel code.

* Nunchuk updates state of its internal registers only when they have been read.
