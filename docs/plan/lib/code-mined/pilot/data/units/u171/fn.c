static inline int ipack_get_carrier(struct ipack_device *dev)
{
	return try_module_get(dev->bus->owner);
}
