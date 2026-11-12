static int __init testfunc(void)
{
	int		buf[6];
	int		i, j;
	unsigned int	ret;

	printk(KERN_INFO "int fifo test start\n");

	/* put values into the fifo */
	for (i = 0; i != 10; i++)
		kfifo_put(&test, i);

	/* show the number of used elements */
	printk(KERN_INFO "fifo len: %u\n", kfifo_len(&test));

	/* get max of 2 elements from the fifo */
	ret = kfifo_out(&test, buf, 2);
	printk(KERN_INFO "ret: %d\n", ret);
	/* and put it back to the end of the fifo */
	ret = kfifo_in(&test, buf, ret);
	printk(KERN_INFO "ret: %d\n", ret);

	/* skip first element of the fifo */
	printk(KERN_INFO "skip 1st element\n");
	kfifo_skip(&test);

	/* put values into the fifo until is full */
	for (i = 20; kfifo_put(&test, i); i++)
		;

	printk(KERN_INFO "queue len: %u\n", kfifo_len(&test));

	/* show the first value without removing from the fifo */
	if (kfifo_peek(&test, &i))
		printk(KERN_INFO "%d\n", i);

	/* check the correctness of all values in the fifo */
	j = 0;
	while (kfifo_get(&test, &i)) {
		printk(KERN_INFO "item = %d\n", i);
		if (i != expected_result[j++]) {
			printk(KERN_WARNING "value mismatch: test failed\n");
			return -EIO;
		}
	}
	if (j != ARRAY_SIZE(expected_result)) {
		printk(KERN_WARNING "size mismatch: test failed\n");
		return -EIO;
	}
	printk(KERN_INFO "test passed\n");

	return 0;
}
