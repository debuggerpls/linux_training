// SPDX-License-Identifier: GPL-2.0
#include <asm/io.h>
#include <linux/debugfs.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/miscdevice.h>
#include <linux/mod_devicetable.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/pm_runtime.h>
#include <linux/serial_reg.h>
#include <linux/spinlock.h>

#define SERIAL_BUFSIZE 16
#define SERIAL_RESET_COUNTER 0
#define SERIAL_GET_COUNTER 1

#ifdef CONFIG_OF
static const struct of_device_id serial_of_match[] = {
	{ .compatible = "bootlin,serial" },
	{}
};
MODULE_DEVICE_TABLE(of, serial_of_match);
#endif

struct serial_dev {
	struct miscdevice miscdev;
	void __iomem *regs;
	struct dentry* debug_entry;
	wait_queue_head_t wait_queue;
	spinlock_t lock;
	char serial_buf[SERIAL_BUFSIZE];
	int serial_buf_rd;
	int serial_buf_wr;
	int irq;
	unsigned int tx_bytes;
};

static unsigned int reg_read(struct serial_dev *dev, int off)
{
	return readl(dev->regs + off * 4);
}

static void reg_write(struct serial_dev *dev, int val, int off)
{
	writel(val, dev->regs + off * 4);
}

static void uart_write(struct serial_dev *dev, char c)
{
	unsigned long flags;

	spin_lock_irqsave(&dev->lock, flags);

	while (!(reg_read(dev, UART_LSR) & UART_LSR_THRE))
		cpu_relax();

	reg_write(dev, c, UART_TX);
	++dev->tx_bytes;
	spin_unlock_irqrestore(&dev->lock, flags);
}

static ssize_t serial_read(struct file *f, char __user *buf,
			   size_t sz, loff_t *off)
{
	int sig, ret = 0;
	unsigned long flags;
	struct serial_dev *dev = container_of(f->private_data,
					      struct serial_dev, miscdev);

	sig = wait_event_interruptible(dev->wait_queue, dev->serial_buf_wr != dev->serial_buf_rd);
	if (!sig) {
		spin_lock_irqsave(&dev->lock, flags);

		if (put_user(dev->serial_buf[dev->serial_buf_rd], buf)) {
			spin_unlock_irqrestore(&dev->lock, flags);
			return -EFAULT;
		}
		dev->serial_buf_rd = (dev->serial_buf_rd + 1) % SERIAL_BUFSIZE;
		++ret;
		spin_unlock_irqrestore(&dev->lock, flags);
	}

	*off += ret;
	return ret;
}

static ssize_t serial_write(struct file *f, const char __user *buf,
			   size_t sz, loff_t *off)
{
	char c;
	int i;
	struct serial_dev *dev = container_of(f->private_data,
					      struct serial_dev, miscdev);

	for (i = 0; i < sz; ++i) {
		if (get_user(c, buf + i))
			return -EFAULT;

		dev_dbg(dev->miscdev.parent, "Writing %c\n", c);
		uart_write(dev, c);
		if (c == '\n')
			uart_write(dev, '\r');
	}

	*off += sz;
	return sz;
}

static long serial_ioctl(struct file *f, unsigned int cmd, unsigned long arg)
{
	struct serial_dev *dev = container_of(f->private_data,
					      struct serial_dev, miscdev);
	unsigned int __user *ptr = (void __user *)arg;
	unsigned long flags;

	switch (cmd) {
	case SERIAL_RESET_COUNTER:
		spin_lock_irqsave(&dev->lock, flags);
		dev->tx_bytes = 0;
		spin_unlock_irqrestore(&dev->lock, flags);
		break;
	case SERIAL_GET_COUNTER:
		spin_lock_irqsave(&dev->lock, flags);
		if (put_user(dev->tx_bytes, ptr)) {
			spin_unlock_irqrestore(&dev->lock, flags);
			return -EFAULT;
		}
		spin_unlock_irqrestore(&dev->lock, flags);
		break;
	default:
		return -ENOTTY;
	}

	return 0;
}

static irqreturn_t serial_interrupt(int irq, void *dev_id)
{
	unsigned long flags;
	struct serial_dev *dev = dev_id;

	spin_lock_irqsave(&dev->lock, flags);
	dev->serial_buf[dev->serial_buf_wr] = reg_read(dev, UART_RX) & 0xff;
	dev_dbg(dev->miscdev.parent, "Received %c\n", dev->serial_buf[dev->serial_buf_wr]);
	dev->serial_buf_wr = (dev->serial_buf_wr + 1) % SERIAL_BUFSIZE;
	spin_unlock_irqrestore(&dev->lock, flags);

	wake_up(&dev->wait_queue);
	//pr_info("%s with irq=%d, received=%c", __func__, irq, reg_read(dev, UART_RX));

	return IRQ_HANDLED;
}

static const struct file_operations serial_ops = {
	.owner = THIS_MODULE,
	.read = serial_read,
	.write = serial_write,
	.unlocked_ioctl = serial_ioctl
};

static int serial_probe(struct platform_device *pdev)
{
	struct resource *res;
	struct serial_dev *dev;
	unsigned int uartclk, baud_divisor;
	int err;

	pm_runtime_enable(&pdev->dev);
	pm_runtime_get_sync(&pdev->dev);

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res)
		return -EINVAL;

	dev = devm_kzalloc(&pdev->dev, sizeof(struct serial_dev), GFP_KERNEL);
	if (!dev)
		return -ENOMEM;

	dev->regs = devm_ioremap_resource(&pdev->dev, res);
	if (!dev->regs) {
		dev_err(&pdev->dev, "Cannot remap registers\n");
		return -ENOMEM;
	}

	// Configure baud rate to 115200
	of_property_read_u32(pdev->dev.of_node, "clock-frequency", &uartclk);
	baud_divisor = uartclk / 16 / 115200;
	reg_write(dev, 0x07, UART_OMAP_MDR1);
	reg_write(dev, 0x00, UART_LCR);
	reg_write(dev, UART_LCR_DLAB, UART_LCR);
	reg_write(dev, baud_divisor & 0xff, UART_DLL);
	reg_write(dev, (baud_divisor >> 8) & 0xff, UART_DLM);
	reg_write(dev, UART_LCR_WLEN8, UART_LCR);
	// Enable interrupts
	reg_write(dev, UART_IER_RDI, UART_IER);

	// Soft reset
	reg_write(dev, UART_FCR_CLEAR_RCVR | UART_FCR_CLEAR_XMIT, UART_FCR);
	reg_write(dev, 0x00, UART_OMAP_MDR1);

	dev->irq = platform_get_irq(pdev, 0);
	err = devm_request_irq(&pdev->dev, dev->irq, serial_interrupt, IRQF_ONESHOT, pdev->name, dev);
	if (err) {
		dev_err(&pdev->dev, "Failed to request IRQ\n");
		return err;
	}

	init_waitqueue_head(&dev->wait_queue);

	spin_lock_init(&dev->lock);

	dev->miscdev.minor = MISC_DYNAMIC_MINOR;
	dev->miscdev.name = devm_kasprintf(&pdev->dev, GFP_KERNEL, "serial-%x", res->start);
	dev->miscdev.fops = &serial_ops;
	dev->miscdev.parent = &pdev->dev;
	platform_set_drvdata(pdev, dev);

	err = misc_register(&dev->miscdev);
	if (err) {
		dev_err(&pdev->dev, "Failed to register misc device\n");
		return err;
	}

#ifdef CONFIG_DEBUG_FS
	dev->debug_entry = debugfs_create_dir(dev->miscdev.name, NULL);
	debugfs_create_u32("tx_counter", 0444, dev->debug_entry, &dev->tx_bytes);
#endif

	return 0;
}

static int serial_remove(struct platform_device *pdev)
{
	struct serial_dev *dev = platform_get_drvdata(pdev);
#ifdef CONFIG_DEBUG_FS
	debugfs_remove(dev->debug_entry);
#endif
	misc_deregister(&dev->miscdev);
	pm_runtime_disable(&pdev->dev);

	return 0;
}

static struct platform_driver serial_driver = {
	.driver = { .name = "serial",
		    .owner = THIS_MODULE,
		    .of_match_table = serial_of_match },
	.probe = serial_probe,
	.remove = serial_remove,
};

module_platform_driver(serial_driver);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Bootlin lab UART driver");
MODULE_AUTHOR("Deividas Puplauskas");
