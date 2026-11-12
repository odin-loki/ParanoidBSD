int snd_seq_delete_port(struct snd_seq_client *client, int port)
{
	struct snd_seq_client_port *found = NULL, *p;

	scoped_guard(mutex, &client->ports_mutex) {
		guard(write_lock_irq)(&client->ports_lock);
		list_for_each_entry(p, &client->ports_list_head, list) {
			if (p->addr.port == port) {
				/* ok found.  delete from the list at first */
				list_del(&p->list);
				client->num_ports--;
				found = p;
				break;
			}
		}
	}
	if (found)
		return port_delete(client, found);
	else
		return -ENOENT;
}
