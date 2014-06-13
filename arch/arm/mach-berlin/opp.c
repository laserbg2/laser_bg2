/*
 *  MARVELL BERLIN OPP related functions
 *
 *  Author:	Jisheng Zhang <jszhang@marvell.com>
 *  Copyright (c) 2014 Marvell Technology Group Ltd.
 *		http://www.marvell.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/init.h>
#include <linux/cpu.h>
#include <linux/opp.h>
#include <linux/clk.h>
#include <linux/of_platform.h>

static unsigned long freq[] = {600000000, 800000000, 1000000000};

static int __init berlin_init_opp_table(void)
{
	int i, ret = 0;
	int found = 0;
	unsigned long cpu_freq;
	struct device_node *np;
	struct device *cpu_dev;
	struct clk *cpu_clk;

	np = of_find_node_by_path("/cpus/cpu@0");
	if (!np) {
		pr_err("failed to find cpu0 node\n");
		return -ENODEV;
	}

	cpu_dev = get_cpu_device(0);
	if (!cpu_dev) {
		pr_err("failed to get cpu0 device\n");
		ret = -ENODEV;
		goto out_put_node;
	}
	cpu_dev->of_node = np;

	cpu_clk = devm_clk_get(cpu_dev, NULL);
	if (IS_ERR(cpu_clk)) {
		ret = PTR_ERR(cpu_clk);
		goto out_put_node;
	}

	cpu_freq = clk_get_rate(cpu_clk);
	for (i = 0; i < ARRAY_SIZE(freq); i++) {
		opp_add(cpu_dev, freq[i], 1200000);
		if (cpu_freq == freq[i])
			found = 1;
	}
	if (!found)
		opp_add(cpu_dev, cpu_freq, 1200000);

	platform_device_register_simple("cpufreq-cpu0", -1, NULL, 0);

	devm_clk_put(cpu_dev, cpu_clk);
out_put_node:
	of_node_put(np);
	return ret;
}
late_initcall(berlin_init_opp_table);
