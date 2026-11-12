static int __init via_pmu_start(void)
{
	unsigned int __maybe_unused irq;

	if (pmu_state == uninitialized)
		return -ENODEV;

	batt_req.complete = 1;

#ifdef CONFIG_PPC_PMAC
	irq = irq_of_parse_and_map(vias, 0);
	if (!irq) {
		printk(KERN_ERR "via-pmu: can't map interrupt\n");
		return -ENODEV;
	}
	/* We set IRQF_NO_SUSPEND because we don't want the interrupt
	 * to be disabled between the 2 passes of driver suspend, we
	 * control our own disabling for that one
	 */
	if (request_irq(irq, via_pmu_interrupt, IRQF_NO_SUSPEND,
			"VIA-PMU", (void *)0)) {
		printk(KERN_ERR "via-pmu: can't request irq %d\n", irq);
		return -ENODEV;
	}

	if (pmu_kind == PMU_KEYLARGO_BASED) {
		gpio_node = of_find_node_by_name(NULL, "extint-gpio1");
		if (gpio_node == NULL)
			gpio_node = of_find_node_by_name(NULL,
							 "pmu-interrupt");
		if (gpio_node)
			gpio_irq = irq_of_parse_and_map(gpio_node, 0);

		if (gpio_irq) {
			if (request_irq(gpio_irq, gpio1_interrupt,
					IRQF_NO_SUSPEND, "GPIO1 ADB",
					(void *)0))
				printk(KERN_ERR "pmu: can't get irq %d"
				       " (GPIO1)\n", gpio_irq);
			else
				gpio_irq_enabled = 1;
		}
	}

	/* Enable interrupts */
	out_8(&via1[IER], IER_SET | SR_INT | CB1_INT);
#else
	if (request_irq(IRQ_MAC_ADB_SR, via_pmu_interrupt, IRQF_NO_SUSPEND,
			"VIA-PMU-SR", NULL)) {
		pr_err("%s: couldn't get SR irq\n", __func__);
		return -ENODEV;
	}
	if (request_irq(IRQ_MAC_ADB_CL, via_pmu_interrupt, IRQF_NO_SUSPEND,
			"VIA-PMU-CL", NULL)) {
		pr_err("%s: couldn't get CL irq\n", __func__);
		free_irq(IRQ_MAC_ADB_SR, NULL);
		return -ENODEV;
	}
#endif /* !CONFIG_PPC_PMAC */

	pmu_fully_inited = 1;

	/* Make sure PMU settle down before continuing. This is _very_ important
	 * since the IDE probe may shut interrupts down for quite a bit of time. If
	 * a PMU communication is pending while this happens, the PMU may timeout
	 * Not that on Core99 machines, the PMU keeps sending us environement
	 * messages, we should find a way to either fix IDE or make it call
	 * pmu_suspend() before masking interrupts. This can also happens while
	 * scolling with some fbdevs.
	 */
	do {
		pmu_poll();
	} while (pmu_state != idle);

	return 0;
}
