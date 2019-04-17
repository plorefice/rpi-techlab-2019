/*
 * LPS25H I2C pressure driver using IIO framework
 *
 * Copyright 2019 Pietro Lorefice <pietro@develer.com>
 *
 * Licensed under the GPL-2.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/iio/iio.h>

#define LPS25H_WHO_AM_I_REG	0x0F
#define LPS25H_CTRL_REG1	0x20
#define LPS25H_TEMP_OUT_L_REG	0x2B
#define LPS25H_TEMP_OUT_H_REG	0x2C

struct lps25h {
	struct i2c_client	*client;
};

static ssize_t lps25h_read_raw(struct iio_dev *indio_dev,
			       struct iio_chan_spec const *chan,
			       int *val, int *val2, long mask)
{
	struct lps25h *lps = iio_priv(indio_dev);
	struct i2c_client *client = lps->client;
	s32 temp, ret;
	s16 raw;

	switch (mask) {
	case IIO_CHAN_INFO_SCALE:
		switch (chan->type) {
		case IIO_TEMP:
			ret = i2c_smbus_read_byte_data(client, LPS25H_TEMP_OUT_L_REG);
			if (ret < 0) {
				return ret;
			}
			raw = (s16)(ret & 0xff);

			ret = i2c_smbus_read_byte_data(client, LPS25H_TEMP_OUT_H_REG);
			if (ret < 0) {
				return ret;
			}
			raw |= (s16)((ret & 0xff) << 8);

			temp = 42500 + (((s32)raw * 1000) / 480);

			*val = temp / 1000;
			*val2 = (temp % 1000) * 1000;
			return IIO_VAL_INT_PLUS_MICRO;
		default:
			return -EINVAL;
		}
		break;
	}

	return -EINVAL;
}

static const struct iio_chan_spec lps25h_channels[] = {
	{
		.type = IIO_TEMP,
		.indexed = 1,
		.channel = 0,
		.info_mask_separate = BIT(IIO_CHAN_INFO_SCALE),
		.info_mask_shared_by_all = 0,
		.address = LPS25H_TEMP_OUT_L_REG,
		.scan_index = 0,
		.scan_type = {
			.sign = 's',
			.realbits = 16,
			.storagebits = 16,
			.endianness = IIO_LE,
		},
	},
};

static const struct iio_info lps25h_info = {
	.read_raw = lps25h_read_raw,
};

static int lps25h_enable(struct lps25h *lps)
{
	/* Disable power-down and configure ODR bits @ 1Hz */
	return i2c_smbus_write_byte_data(lps->client, LPS25H_CTRL_REG1, 0x90);
}

static int lps25h_disable(struct lps25h *lps)
{
	/* Enable power-down and configure ODR for one-shot conversion */
	return i2c_smbus_write_byte_data(lps->client, LPS25H_CTRL_REG1, 0x00);
}

static int lps25h_i2c_probe(struct i2c_client *client,
			    const struct i2c_device_id *id)
{
	struct iio_dev *indio_dev;
	struct lps25h *lps;
	int err;

	indio_dev = devm_iio_device_alloc(&client->dev, sizeof(*lps));
	if (!indio_dev) {
		dev_err(&client->dev, "failed to allocate memory\n");
		return -ENOMEM;
	}

	lps = iio_priv(indio_dev);

	i2c_set_clientdata(client, indio_dev);

	/* Fill in IIO data structure */
	indio_dev->name = client->dev.driver->name;
	indio_dev->dev.parent = &client->dev;
	indio_dev->info = &lps25h_info;

	indio_dev->channels = lps25h_channels;
	indio_dev->num_channels = ARRAY_SIZE(lps25h_channels);
	indio_dev->modes = INDIO_DIRECT_MODE;

	/* Fill in LPS25H data structure */
	lps->client = client;

	/* Startup sensor */
	err = lps25h_enable(lps);
	if (err) {
		dev_err(&client->dev, "failed to issue enable\n");
		return err;
	}

	err = iio_device_register(indio_dev);
	if (err) {
		return err;
	}

	dev_info(&indio_dev->dev, "registered sensor %s\n",
			indio_dev->name);

	return 0;
}

static int lps25h_i2c_remove(struct i2c_client *client)
{
	struct iio_dev *indio_dev = i2c_get_clientdata(client);
	struct lps25h *lps = iio_priv(indio_dev);

	iio_device_unregister(indio_dev);

	lps25h_disable(lps);

	return 0;
}

#ifdef CONFIG_OF
static const struct of_device_id lps25h_of_match[] = {
	{ .compatible = "st,lps25h-press" },
	{},
};
MODULE_DEVICE_TABLE(of, lps25h_of_match);
#else
#define lps25h_of_match NULL
#endif

static struct i2c_driver lps25h_driver = {
	.driver = {
		.name = "lps25h-i2c",
		.of_match_table = of_match_ptr(lps25h_of_match),
	},
	.probe = lps25h_i2c_probe,
	.remove = lps25h_i2c_remove,
};
module_i2c_driver(lps25h_driver);

MODULE_AUTHOR("Pietro Lorefice <pietro@develer.com>");
MODULE_DESCRIPTION("IIO driver for LPS25H MEMS pressure sensor");
MODULE_LICENSE("GPL v2");
