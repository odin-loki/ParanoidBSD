# liblattzfs

* License:	2-Clause BSD License
* Author:	Shawn Webb
  [shawn.webb@hardenedbsd.org](mailto:shawn.webb@hardenedbsd.org)

This is a tiny little wrapper around libzfs so that applications in
ports don't have to depend on all the weird, funky CFLAGS.

This code is developed out-of-tree, but will eventually be imported as
vendor code into HardenedBSD.

The reason behind this project is to make ZFS integration in hbsdmon
sane.
