#### Accessing IO memory and ports lab
---

* Create `am335x_customboneblack.dts` file with uart2 and uart4 pinmuxing, as well as enable the uart2 and uart4 devices.

* Add array of `struct of_device_id` with entry of `{ .compatible = "bootlin,serial" }`. Then add it to `MODULE_DEVICE_TABLE()` and in `struct platform_driver` in driver member as `.of_match_table`.

* Extract the base physical address using `platform_get_resource(pdev, IORESOURCE_MEM, 0)`. Returned `struct resource` holds base address at `start` member. 

* Then create a struct that will hold the virtual address. 

* Get the virtual address by remapping the resource using `devm_ioremap_resource(&pdev->dev, res)`. 

* Create `reg_read()` and `reg_write()` functions that take the private data struct as argument and write or read to the virtual address pointer. Offset to write needs to be multiplied by 4 for OMAP SoCs.

* Add power management initialization in `probe()` - `pm_runtime_enable()` and `pm_runtime_sync()`. In `remove()` - `pm_runtime_disable()`.

* Configure the line and baudrate. Read device tree properties using `of_property_read_*` functions. Then soft reset the device.

* Check section 16 of AM335x TRM for details about configuration.

* Write function to write single character to the device. It takes private data struct and char as argument. Wait until `UART_LSR_THRE` gets set in `UART_LSR` register. Use `cpu_relax()` in busy wait, so it doesnt get optimized out. Then write the char to `UART_TX` register.

#### Output-only misc driver
---

* Now the driver will be exposed to misc framework. To do this, `miscdevice` structure needs to be registered using `misc_register()` in `probe()` and `misc_deregister()` in `remove()`.

* Register and deregister functions require `struct miscdevice`, which will be added to the private data struct. To be able to remove the device from misc subsystem, the private data struct will be saved in platform_device using `platform_set_drvdata()`.

* Create write and read stub functions. Then initialize `serial_ops` struct that inherits from `struct file_operations` and add read and write stub functions to it.

* At the end of `probe()`, when device is fuly ready to work, initialize the miscdevice structure. Assign automatical minor number using `MISC_DYNAMIC_MINOR`. Set name using `devm_kasprintf()`. Set fops to `serial_ops`. Set parent to `&pdev->dev`. Then register the misc device.

* Each device will show up in `/dev` and `/sys/class/misc` with names provided.

#### Implementing write()
---

* To implement write, we need the private data structure. This structure can be received using the `container_of(f->private_data, struct serial_dev, miscdev)` macro. Normally `p->private_data` returns pointer to `miscdevice` structure. Because `miscdev` is contained inside the private data struct, that macro can calculate the offset. 

* Other way to get the private data would be to get the `miscdev`, then use `parent` member to then get `platform_get_drvdata()`. 

* All thats left to do is write characters from `buf` using `get_user()`, to copy the data from userspace to kernelspace and pass it to our UART write function.

* Same goes for `ioctl` function, just check the command and copy the bytes written, that could be saved in private data struct, to the userspace using `put_user()`.

#### Module reference count
---

* To enable module referencing, add `.owner = THIS_MODULE` to the `struct file_operations`. This will increase the count when using `lsmod` and disable module unloading, when someone is referencing it.

#### Sleeping and handling interrupts
---

* Linux uses virtual IRQ number that it derives from the hardware interrupt number.

* First add `int irq` member to `struct serial_dev`

* Retrieve the IRQ number using `platform_get_irq(pdev, 0)` and save it in `struct serial_dev`.

* Then, request irq by providing the interrupt handler and irq number using `devm_request_irq()`.

* Enable interrupts on the UART device by writing `UART_IER_RDI` to `UART_IER` register.

* In IRQ_handler function just return `IRQ_HANDLED` to tell the kernel that interrupt was handled.

* By sending a char over the line, `interrupt flood` should happen. We need to acknowledge the interrupt in UART device, otherwise interrupts will be sent continuously. 

* Interrupt acknogledment on UART controller are done by reading the `UART_RX` register. 

* `read()` operation of the driver will go to sleep, when no new data is available. This is done using a circular buffer which is saved in `struct serial_dev`.

* First add `wait_queue_head_t wait_queue` to the `struct serial_dev`. This needs to be initialized in `probe()` using `init_waitqueue_head(&dev->wait_queue)`. 

* Then in `read()` go to sleep until there is data in our circular buffer. This is done using `wait_event_interruptible(dev->wait_queue, dev->serial_buf_wr != dev->serial_buf_rd)`. If 0 is returned, then the condition got evaluated to true.

* Now just write the char to userspace using `put_user()` and update the `serial_buf_rd` position. Lastly return characters written.

* To wake up waiting processes, add `wake_up(&dev->wait_queue)` in the interrupt handler routine. 

#### Mutual exclusion 
---

* Shared resourses are the access to driver register itself and circular buffer. Therefore add `spinlock_t` to `struct serial_dev`.

* Init the spinlock in `probe()` using `spin_lock_init(&dev->lock)`.

* Add `spin_lock_irqsave(&dev->lock, flags)` and `spin_unlock_irqrestone(&dev->lock, flags)` to each part, when the registers or circular buffer is accessed.





