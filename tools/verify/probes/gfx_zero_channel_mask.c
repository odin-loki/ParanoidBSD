/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
 *
 * stand/common/gfx_fb.c:gfx_fb_color_map() and
 * stand/efi/libefi/efi_console.c:efi_cons_init() shift a channel mask
 * right by ffs(mask) - 1.  ffs(0) is 0, so a zero mask shifts by -1.
 *
 *   cbmc --undefined-shift-check -DOLD
 *        tools/verify/probes/gfx_zero_channel_mask.c
 *       -> shift distance is negative, FAILED
 *   cbmc --undefined-shift-check
 *        tools/verify/probes/gfx_zero_channel_mask.c
 *       -> 0 of N, SUCCESSFUL
 *
 *	roff = ffs(gfx_state.tg_fb.fb_mask_red) - 1;
 *	...
 *	rmask = gfx_state.tg_fb.fb_mask_red >> roff;
 *
 * The masks are firmware's, not the loader's.
 * stand/efi/loader/framebuffer.c:133-137 takes the PixelBitMask case
 * verbatim off the GOP:
 *
 *	case PixelBitMask:
 *		efifb->fb_mask_red = pixinfo->RedMask;
 *
 * and nothing between there and the shift rejects a zero.  UEFI 2.10
 * requires at least one bit set per channel for PixelBitMask, so a
 * zero is firmware out of spec - which is the case a boot loader has
 * to survive rather than assume away.  On x86 a negative shift count
 * is masked to five bits, so `mask >> -1' is `mask >> 31': not a
 * crash, a silently wrong palette, which is worse to diagnose.
 *
 * The assertion is that the shift distance is in range.
 */

int nondet_int(void);

static int
ffs_model(unsigned int v)
{
	int i;

	for (i = 0; i < 32; i++)
		if (v & (1u << i))
			return (i + 1);
	return (0);
}

int
main(void)
{
	unsigned int red = (unsigned int)nondet_int();
	int roff, rmask;

	/*
	 * Firmware's mask, with the one value the code does not expect
	 * left in the domain.
	 */
	__CPROVER_assume(red == 0 || red == 0x00ff0000 || red == 0xf800);

	roff = ffs_model(red) - 1;
#ifndef OLD
	if (roff < 0)
		roff = 0;
#endif
	__CPROVER_assert(roff >= 0 && roff < 32,
	    "the shift distance is in range");
	rmask = (int)(red >> roff);
	return (rmask == 0 ? 0 : 1);
}
