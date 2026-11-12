static int init_chip(struct hfi1_devdata *dd)
{
	int i;
	int ret = 0;

	/*
	 * Put the HFI CSRs in a known state.
	 * Combine this with a DC reset.
	 *
	 * Stop the device from doing anything while we do a
	 * reset.  We know there are no other active users of
	 * the device since we are now in charge.  Turn off
	 * off all outbound and inbound traffic and make sure
	 * the device does not generate any interrupts.
	 */

	/* disable send contexts and SDMA engines */
	write_csr(dd, SEND_CTRL, 0);
	for (i = 0; i < chip_send_contexts(dd); i++)
		write_kctxt_csr(dd, i, SEND_CTXT_CTRL, 0);
	for (i = 0; i < chip_sdma_engines(dd); i++)
		write_kctxt_csr(dd, i, SEND_DMA_CTRL, 0);
	/* disable port (turn off RXE inbound traffic) and contexts */
	write_csr(dd, RCV_CTRL, 0);
	for (i = 0; i < chip_rcv_contexts(dd); i++)
		write_csr(dd, RCV_CTXT_CTRL, 0);
	/* mask all interrupt sources */
	for (i = 0; i < CCE_NUM_INT_CSRS; i++)
		write_csr(dd, CCE_INT_MASK + (8 * i), 0ull);

	/*
	 * DC Reset: do a full DC reset before the register clear.
	 * A recommended length of time to hold is one CSR read,
	 * so reread the CceDcCtrl.  Then, hold the DC in reset
	 * across the clear.
	 */
	write_csr(dd, CCE_DC_CTRL, CCE_DC_CTRL_DC_RESET_SMASK);
	(void)read_csr(dd, CCE_DC_CTRL);

	if (use_flr) {
		/*
		 * A FLR will reset the SPC core and part of the PCIe.
		 * The parts that need to be restored have already been
		 * saved.
		 */
		dd_dev_info(dd, "Resetting CSRs with FLR\n");

		/* do the FLR, the DC reset will remain */
		pcie_flr(dd->pcidev);

		/* restore command and BARs */
		ret = restore_pci_variables(dd);
		if (ret) {
			dd_dev_err(dd, "%s: Could not restore PCI variables\n",
				   __func__);
			return ret;
		}

		if (is_ax(dd)) {
			dd_dev_info(dd, "Resetting CSRs with FLR\n");
			pcie_flr(dd->pcidev);
			ret = restore_pci_variables(dd);
			if (ret) {
				dd_dev_err(dd, "%s: Could not restore PCI variables\n",
					   __func__);
				return ret;
			}
		}
	} else {
		dd_dev_info(dd, "Resetting CSRs with writes\n");
		reset_cce_csrs(dd);
		reset_txe_csrs(dd);
		reset_rxe_csrs(dd);
		reset_misc_csrs(dd);
	}
	/* clear the DC reset */
	write_csr(dd, CCE_DC_CTRL, 0);

	/* Set the LED off */
	setextled(dd, 0);

	/*
	 * Clear the QSFP reset.
	 * An FLR enforces a 0 on all out pins. The driver does not touch
	 * ASIC_QSFPn_OUT otherwise.  This leaves RESET_N low and
	 * anything plugged constantly in reset, if it pays attention
	 * to RESET_N.
	 * Prime examples of this are optical cables. Set all pins high.
	 * I2CCLK and I2CDAT will change per direction, and INT_N and
	 * MODPRS_N are input only and their value is ignored.
	 */
	write_csr(dd, ASIC_QSFP1_OUT, 0x1f);
	write_csr(dd, ASIC_QSFP2_OUT, 0x1f);
	init_chip_resources(dd);
	return ret;
}
