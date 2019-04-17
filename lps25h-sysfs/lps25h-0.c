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

static int lps25h_i2c_probe(struct i2c_client *client,
			    const struct i2c_device_id *id)
{
	dev_info(&client->dev, "module loaded\n");
	return 0;
}

static int lps25h_i2c_remove(struct i2c_client *client)
{
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
