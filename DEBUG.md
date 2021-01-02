#### Dynamic debugging
---

* `printk()` are no longer recommended for new debugging messages.

* Recommended `pr_*()` family. Defined in `include/linux/printk.h`. Take classic format string.

* Recommended `dev_*()` family of functions. Defined in `include/linux/device.h`. Takes pointer to `struct device` as first arg, then formated string.

* `pr_debug()` and `dev_dbg()` are special. When driver is compiled with `DEBUG` defined, these messages are compiled and printed at the debug level. 

* Compile kernel with `CONFIG_DYNAMIC_DEBUG` to enable dynamic debugging. Then these messages can dynamically be enabled on a per-file, per-module or per-message basis. This way you can get only the messages you want.

* Details in `admin-guide/dynamic-debug-howto`.

* Messages are associated to a priority, from 0 = emergency to 7 = debug. All these messages are stored using `dmesg` command.

* Some messages may appear on console, depending on their priority and config of `loglevel` kernel parameter or `/proc/sys/kernel/printk` which allows to change at runtime which messages are displayed on console. `loglevel=0` - no messages, `loglevel=8` - all messages.

* Debugfs needs to be mounted: `sudo mount -t debugfs none /sys/kernel/debug`

* Available debug points can be found in `less /sys/kernel/debug/dynamic_debug/control`. 

* To enable for example module debug messages: `echo "module serial +p" > /sys/kernel/debug/dynamic_debug/control`.

#### Debugfs
---

* Kernel needs to be compiled with `CONFIG_DEBUG_FS`. Then mounted as described above.

* `debugfs_*()` function family can be used to create and remove entries for the driver. Declared in `include/linux/debugfs.h`. 

* WARNING: Each file/dir needs to be removed manually. This is not done when module is removed. `debugfs_remove()` removes everything recursively, therefore save the root entry in private data struct and remove everything in `remove()`.

 
