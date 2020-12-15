### Reflashing U-Boot on BeagleBone Black

These commands were taken from `bootlin` trainings. Detailed
information is available at [bootlin github][1].

Partition:
```
sudo sfdisk /dev/mmcblk0 << EOF
1,,0xE,*
EOF
```
Format:
```
sudo mkfs.vfat -F 32 /dev/mmcblk0p1 -n boot
```
Copy:
```
cp zImage dtb MLO MLO.final u-boot.img u-boot.img.final MBR /media/$USER/boot
```

Then insert SD-Card, turn on while holding BOOT button, wait 20-30 seconds 
until all LEDs are solid. 

Turn off while holding POWER button for 8s. Remove SD-Card.


[1]: https://github.com/bootlin/training-materials/tree/master/lab-data/common/bootloader/beaglebone-black