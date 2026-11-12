#include "prelude.h"
#undef N_DEVS
#define N_DEVS ((1U<<MINORBITS)/AOE_PARTITIONS)
enum flush_parms { 	NOT_EXITING = 0, 	EXITING = 1, };
/*@ requires true; ensures \result == 0; */
int __init
aoedev_init(void)
{
	return 0;
}
