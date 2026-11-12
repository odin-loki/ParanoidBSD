static ssize_t cache_read(struct file *filp, char __user *buf, size_t count,
			  loff_t *ppos, struct cache_detail *cd)
{
	struct cache_reader *rp = filp->private_data;
	struct cache_request *rq;
	struct inode *inode = file_inode(filp);
	int err;

	if (count == 0)
		return 0;

	inode_lock(inode); /* protect against multiple concurrent
			      * readers on this file */
 again:
	spin_lock(&queue_lock);
	/* need to find next request */
	while (rp->q.list.next != &cd->queue &&
	       list_entry(rp->q.list.next, struct cache_queue, list)
	       ->reader) {
		struct list_head *next = rp->q.list.next;
		list_move(&rp->q.list, next);
	}
	if (rp->q.list.next == &cd->queue) {
		spin_unlock(&queue_lock);
		inode_unlock(inode);
		WARN_ON_ONCE(rp->offset);
		return 0;
	}
	rq = container_of(rp->q.list.next, struct cache_request, q.list);
	WARN_ON_ONCE(rq->q.reader);
	if (rp->offset == 0)
		rq->readers++;
	spin_unlock(&queue_lock);

	if (rq->len == 0) {
		err = cache_request(cd, rq);
		if (err < 0)
			goto out;
		rq->len = err;
	}

	if (rp->offset == 0 && !test_bit(CACHE_PENDING, &rq->item->flags)) {
		err = -EAGAIN;
		spin_lock(&queue_lock);
		list_move(&rp->q.list, &rq->q.list);
		spin_unlock(&queue_lock);
	} else {
		if (rp->offset + count > rq->len)
			count = rq->len - rp->offset;
		err = -EFAULT;
		if (copy_to_user(buf, rq->buf + rp->offset, count))
			goto out;
		rp->offset += count;
		if (rp->offset >= rq->len) {
			rp->offset = 0;
			spin_lock(&queue_lock);
			list_move(&rp->q.list, &rq->q.list);
			spin_unlock(&queue_lock);
		}
		err = 0;
	}
 out:
	if (rp->offset == 0) {
		/* need to release rq */
		spin_lock(&queue_lock);
		rq->readers--;
		if (rq->readers == 0 &&
		    !test_bit(CACHE_PENDING, &rq->item->flags)) {
			list_del(&rq->q.list);
			spin_unlock(&queue_lock);
			cache_put(rq->item, cd);
			kfree(rq->buf);
			kfree(rq);
		} else
			spin_unlock(&queue_lock);
	}
	if (err == -EAGAIN)
		goto again;
	inode_unlock(inode);
	return err ? err :  count;
}
