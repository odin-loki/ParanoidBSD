int hci_recv_diag(struct hci_dev *hdev, struct sk_buff *skb)
{
	/* Mark as diagnostic packet */
	hci_skb_pkt_type(skb) = HCI_DIAG_PKT;

	/* Time stamp */
	__net_timestamp(skb);

	skb_queue_tail(&hdev->rx_q, skb);
	queue_work(hdev->workqueue, &hdev->rx_work);

	return 0;
}
