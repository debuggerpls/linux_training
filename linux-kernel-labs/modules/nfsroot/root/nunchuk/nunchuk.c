// SPDX-License-Identifier: GPL-2.0
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/module.h>
#include <linux/i2c.h>

#define READ_REGISTER_COUNT 6
#define READ_REGISTERS_CMD 0
#define MIN_SLEEP_RANGE 10000
#define MAX_SLEEP_RANGE 20000

static const struct of_device_id nunchuck_of_match[] = {
	{ .compatible = "nintendo,nunchuk" },
	{}
};
MODULE_DEVICE_TABLE(of, nunchuck_of_match);

static int nunchuk_read_registers(struct i2c_client *client,
				  unsigned char buf[READ_REGISTER_COUNT])
{
	int ret;
	unsigned char cmd;

	usleep_range(MIN_SLEEP_RANGE, MAX_SLEEP_RANGE);
	cmd = READ_REGISTERS_CMD;
	ret = i2c_master_send(client, &cmd, 1);
	if (ret != 1)
		return ret;

	usleep_range(MIN_SLEEP_RANGE, MAX_SLEEP_RANGE);
	return i2c_master_recv(client, buf, READ_REGISTER_COUNT);
}

static int nunchuk_probe(struct i2c_client *client)
{
	int z_pressed, c_pressed;
	int ret;
	unsigned char buf[6];

	buf[0] = 0xf0;
	buf[1] = 0x55;
	ret = i2c_master_send(client, buf, 2);
	if (ret != 2) {
		pr_err("Init failed, ret=%d\n", ret);
		return ret;
	}
	udelay(1000);

	buf[0] = 0xfb;
	buf[1] = 0x00;
	ret = i2c_master_send(client, buf, 2);
	if (ret != 2) {
		pr_err("Init failed, ret=%d\n", ret);
		return ret;
	}

	ret = nunchuk_read_registers(client, buf);
	if (ret != READ_REGISTER_COUNT) {
		pr_err("Read failed, ret=%d\n", ret);
		return ret;
	}

	ret = nunchuk_read_registers(client, buf);
	if (ret != READ_REGISTER_COUNT) {
		pr_err("Read failed, ret=%d\n", ret);
		return ret;
	}

	z_pressed = !(buf[5] & 1);
	c_pressed = !(buf[5] & (1 << 1));
	pr_info("Z pressed: %d\n", z_pressed);
	pr_info("C pressed: %d\n", c_pressed);

	return 0;
}

static int nunchuck_remove(struct i2c_client *client)
{
	return 0;
}

static struct i2c_driver nunchuk_driver = {
	.probe_new = nunchuk_probe,
	.remove = nunchuck_remove,
	.driver = {
		.name = "nunchuk_i2c",
		.of_match_table = nunchuck_of_match
	}
};
module_i2c_driver(nunchuk_driver);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Nintendo Nunchuk controller driver");
MODULE_AUTHOR("Deividas Puplauskas");
