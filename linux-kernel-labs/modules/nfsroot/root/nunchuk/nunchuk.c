// SPDX-License-Identifier: GPL-2.0
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/input.h>

#define READ_REGISTER_COUNT 6
#define READ_REGISTERS_CMD 0
#define MIN_SLEEP_RANGE 10000
#define MAX_SLEEP_RANGE 20000

struct nunchuk_dev {
	struct i2c_client *i2c_client;
};

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

static void nunchuk_poll(struct input_dev *input)
{
	int ret, z_pressed, c_pressed, x, y;
	unsigned char buf[6];
	struct nunchuk_dev *nunchuk = input_get_drvdata(input);

	ret = nunchuk_read_registers(nunchuk->i2c_client, buf);
	if (ret != READ_REGISTER_COUNT) {
		pr_info("%s: read fail, ret=%d\n", __func__, ret);
		return;
	}

	z_pressed = !(buf[5] & 1);
	c_pressed = !(buf[5] & (1 << 1));
	x = buf[0];
	y = buf[1];

	input_event(input, EV_KEY, BTN_Z, z_pressed);
	input_event(input, EV_KEY, BTN_C, c_pressed);
	input_event(input, EV_ABS, ABS_X, x);
	input_event(input, EV_ABS, ABS_Y, y);
	input_sync(input);
}

static int nunchuk_probe(struct i2c_client *client)
{
	int ret;
	unsigned char buf[2];
	struct input_dev *input;
	struct nunchuk_dev *nunchuk;

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

	input = devm_input_allocate_device(&client->dev);
	if (!input)
		return -ENOMEM;

	input->name = "Wii Nunchuk";
	input->id.bustype = BUS_I2C;

	set_bit(EV_KEY, input->evbit);
	set_bit(BTN_C, input->keybit);
	set_bit(BTN_Z, input->keybit);
	set_bit(EV_ABS, input->evbit);
	set_bit(ABS_X, input->absbit);
	set_bit(ABS_Y, input->absbit);
	input_set_abs_params(input, ABS_X, 30, 220, 4, 8);
	input_set_abs_params(input, ABS_Y, 40, 200, 4, 8);

	/* Classic buttons */
	set_bit(BTN_TL, input->keybit);
	set_bit(BTN_SELECT, input->keybit);
	set_bit(BTN_MODE, input->keybit);
	set_bit(BTN_START, input->keybit);
	set_bit(BTN_TR, input->keybit);
	set_bit(BTN_TL2, input->keybit);
	set_bit(BTN_B, input->keybit);
	set_bit(BTN_Y, input->keybit);
	set_bit(BTN_A, input->keybit);
	set_bit(BTN_X, input->keybit);
	set_bit(BTN_TR2, input->keybit);

	nunchuk = devm_kzalloc(&client->dev, sizeof(struct nunchuk_dev),
			       GFP_KERNEL);
	if (!nunchuk)
		return -ENOMEM;
	nunchuk->i2c_client = client;
	input_set_drvdata(input, nunchuk);

	ret = input_setup_polling(input, nunchuk_poll);
	if (ret)
		return ret;
	input_set_poll_interval(input, 50);

	ret = input_register_device(input);
	if (ret)
		return ret;

	return 0;
}

static int nunchuck_remove(struct i2c_client *client)
{
	return 0;
}

static struct i2c_driver nunchuk_driver = {
	.probe_new = nunchuk_probe,
	.remove = nunchuck_remove,
	.driver = { .name = "nunchuk_i2c", .of_match_table = nunchuck_of_match }
};
module_i2c_driver(nunchuk_driver);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Nintendo Nunchuk controller driver");
MODULE_AUTHOR("Deividas Puplauskas");
