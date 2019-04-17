/*
 * LPS25H I2C pressures driver
 *
 * Copyright 2019 Pietro Lorefice <pietro@develer.com>
 *
 * Licensed under the GPL-2.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/sysfs.h>

#define LPS25H_WHO_AM_I_REG	0x0F

struct lps25h {
	struct i2c_client	*client;
};

static ssize_t lps25h_who_am_i(struct device *dev,
			       struct device_attribute *attr,
			       char *buf)
{
	struct lps25h *lps = dev_get_drvdata(dev);
	struct i2c_client *client = lps->client;
	s32 ret;

	ret = i2c_smbus_read_byte_data(client, LPS25H_WHO_AM_I_REG);
	if (ret < 0) {
		return ret;
	}

	*buf = (u8)ret;

	return 1;
}

static DEVICE_ATTR(who_am_i,	S_IRUGO, lps25h_who_am_i, NULL);

static struct attribute *lps25h_attributes[] = {
	&dev_attr_who_am_i.attr,
	NULL,
};

static const struct attribute_group lps25h_attr_group = {
	.attrs	= lps25h_attributes,
};

static int lps25h_i2c_probe(struct i2c_client *client,
			    const struct i2c_device_id *id)
{
	struct kobject *kobj = &client->dev.kobj;
	struct lps25h *lps;
	int err;

	lps = devm_kzalloc(&client->dev, sizeof(*lps), GFP_KERNEL);
	if (!lps) {
		dev_err(&client->dev, "failed to allocate memory\n");
		return -ENOMEM;
	}

	i2c_set_clientdata(client, lps);

	/* Fill in LPS25H data structure */
	lps->client = client;

	err = sysfs_create_group(kobj, &lps25h_attr_group);
	if (err) {
		dev_err(&client->dev, "failed to create sysfs group\n");
		return err;
	}

	return 0;
}

static int lps25h_i2c_remove(struct i2c_client *client)
{
	struct kobject *kobj = &client->dev.kobj;

	sysfs_remove_group(kobj, &lps25h_attr_group);

	dev_info(&client->dev, "module unloaded\n");
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

MODULE_LICENSE("GPL v2");
