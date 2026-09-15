/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
 *
 * sys/net/if_bridge.c: bridge_input()'s recovery arm restores sc and
 * ifp for a netmap-injected frame and leaves bif NULL behind.
 *
 *   cbmc -DOLD tools/verify/probes/bridge_input_bif.c
 *       -> 1 of 1 failed, FAILED
 *   cbmc       tools/verify/probes/bridge_input_bif.c
 *       -> 0 of 1, SUCCESSFUL
 *
 * The shape:
 *
 *	bif = ifp->if_bridge;
 *	if (bif)
 *		sc = bif->bif_sc;
 *	if (sc == NULL) {
 *		sc = if_getsoftc(ifp);
 *		ifp = bridge_rtlookup(sc, eh->ether_shost, vlan);
 *		if (ifp == NULL) { ...; return (NULL); }
 *		m->m_pkthdr.rcvif = ifp;
 *	}
 *	...
 *	if (!bridge_vfilter_in(bif, m)) {
 *
 * The arm is entered EXACTLY when bif is NULL or its softc is -- ifp
 * was the bridge ifnet, which carries no if_bridge -- so on the only
 * path that reaches it, bif is the one thing not put back.
 * bridge_vfilter_in() then reads `sbif->bif_sc->sc_flags' as its first
 * act, and bif_stp, bif_flags, bif_addrmax and bridge_forward(sc, bif,
 * m) follow it: eight dereferences of a null pointer on the bridge
 * receive path.
 *
 * ifp now names the member the source address was learned on, and
 * bridge_lookup_member_if(sc, ifp) is `return (ifp->if_bridge)', so
 * the member's own bif is what the frame arrived through.
 *
 * The assertion is that whatever reaches bridge_vfilter_in() is a
 * pointer it may dereference.
 */

int nondet_int(void);

struct bridge_softc { int sc_flags; };
struct bridge_iflist { struct bridge_softc *bif_sc; int bif_pvid; };
struct ifnet { struct bridge_iflist *if_bridge; struct bridge_softc *sc; };

static struct bridge_softc a_softc = { 0 };
static struct bridge_iflist a_member = { &a_softc, 0 };

/* The bridge ifnet: a softc, and no if_bridge. */
static struct ifnet bridge_ifp = { 0, &a_softc };
/* A member ifnet: an if_bridge that points at its bridge_iflist. */
static struct ifnet member_ifp = { &a_member, &a_softc };

static struct bridge_softc *
if_getsoftc(struct ifnet *ifp)
{
	return (ifp->sc);
}

static struct bridge_iflist *
bridge_lookup_member_if(struct bridge_softc *sc, struct ifnet *ifp)
{
	(void)sc;
	return (ifp->if_bridge);
}

/* Returns the member the source address was learned on, or NULL. */
static struct ifnet *
bridge_rtlookup(struct bridge_softc *sc)
{
	(void)sc;
	return (nondet_int() ? &member_ifp : 0);
}

static int
bridge_vfilter_in(const struct bridge_iflist *sbif)
{
	__CPROVER_assert(sbif != 0,
	    "bridge_vfilter_in is handed a bif it may dereference");
	return (sbif->bif_sc->sc_flags == 0);
}

static int
bridge_input(struct ifnet *ifp)
{
	struct bridge_softc *sc = 0;
	struct bridge_iflist *bif;

	bif = ifp->if_bridge;
	if (bif)
		sc = bif->bif_sc;

	if (sc == 0) {
		sc = if_getsoftc(ifp);
		ifp = bridge_rtlookup(sc);
		if (ifp == 0)
			return (0);
#ifndef OLD
		bif = bridge_lookup_member_if(sc, ifp);
		if (bif == 0)
			return (0);
#endif
	}
	return (bridge_vfilter_in(bif));
}

int
main(void)
{
	/* A frame arrives either on a member or on the bridge itself. */
	bridge_input(nondet_int() ? &member_ifp : &bridge_ifp);
	return (0);
}
