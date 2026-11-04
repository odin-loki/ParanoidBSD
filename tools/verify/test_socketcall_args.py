#!/usr/bin/env python3
"""The eight ways this check was broken on purpose before it was believed.

A gate that has only ever been run on a tree that passes is a gate that
has only ever been observed printing "ok". Each case below is one edit
made to the real tree, run, and reverted, while the check was being
written - encoded here so nobody has to make those edits again.

The two halves are different in kind and the second is the point:

  - a FAULT is a tree that is wrong (a count that disagrees, a handler
    with no struct, an opcode with no table entry). It fails --gate.
  - a FAULT-TO-READ is the check itself going blind: the table renamed,
    the opcode block gone, a file not in the tree. It fails ALWAYS,
    --gate or not, because a reader that finds nothing and a tree with
    nothing wrong are otherwise the same silence, and this file exists
    because that silence once hid 72 findings' worth of premise.
"""

import io
import contextlib
import sys
import unittest
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))

import socketcall_args as sa                                # noqa: E402


PROTO = """
struct linux_socket_args {
	char domain_l_[PADL_(l_int)]; l_int domain; char domain_r_[PADR_(l_int)];
	char type_l_[PADL_(l_int)]; l_int type; char type_r_[PADR_(l_int)];
	char protocol_l_[PADL_(l_int)]; l_int protocol; char protocol_r_[PADR_(l_int)];
};
struct linux_listen_args {
	char s_l_[PADL_(l_int)]; l_int s; char s_r_[PADR_(l_int)];
	char backlog_l_[PADL_(l_int)]; l_int backlog; char backlog_r_[PADR_(l_int)];
};
"""

HANDWRITTEN = """
struct linux_accept_args {
	register_t s;
	register_t addr;
	register_t namelen;
};
"""


def data(table=None, cases=None, protos=None, ops=None, bufs=None):
    """A tree in miniature: three opcodes, three handlers, three structs."""
    return {
        "ops": ops if ops is not None else
            {"LINUX_SOCKET": 1, "LINUX_LISTEN": 2, "LINUX_ACCEPT": 3},
        "table": table if table is not None else
            [(0, "unused"), (3, "socket"), (2, "listen"), (3, "accept")],
        "cases": cases if cases is not None else {
            "LINUX_SOCKET": "linux_socket",
            "LINUX_LISTEN": "linux_listen",
            "LINUX_ACCEPT": "linux_accept",
        },
        "bufs": bufs if bufs is not None else {"a": 6, "l_args": 6},
        "protos": protos if protos is not None else {
            "sys/i386/linux/linux_proto.h": PROTO,
            "sys/amd64/linux32/linux32_proto.h": PROTO,
            "sys/amd64/linux/linux_proto.h": PROTO,
            "sys/arm64/linux/linux_proto.h": PROTO,
            sa.SOCKET_C: "",
            sa.SOCKET_H: HANDWRITTEN,
        },
    }


class FieldCounting(unittest.TestCase):
    def test_a_generated_struct_counts_arguments_not_padding(self):
        self.assertEqual(sa.struct_fields(PROTO, "linux_socket_args"),
                         ["domain", "type", "protocol"])

    def test_a_handwritten_struct_counts_declarations(self):
        self.assertEqual(sa.struct_fields(HANDWRITTEN, "linux_accept_args"),
                         ["s", "addr", "namelen"])

    def test_a_struct_that_is_not_there_is_None_not_zero(self):
        """Zero fields and no struct must not be the same answer: one
        would silently agree with a table entry of 0."""
        self.assertIsNone(sa.struct_fields(PROTO, "linux_nosuch_args"))

    def test_an_unterminated_body_is_a_fault(self):
        with self.assertRaises(sa.Fault):
            sa.struct_fields("struct linux_x_args { int a;", "linux_x_args")


class TheFixtureItself(unittest.TestCase):
    def test_the_miniature_tree_passes(self):
        """Every case below is this, minus one thing. If the baseline
        does not pass, none of them prove anything."""
        faults, lines = sa.check(data())
        self.assertEqual(faults, [])
        self.assertEqual(sum(1 for ln in lines if "copies" in ln), 3)


class Faults(unittest.TestCase):
    """A tree that is wrong. Reported, and fails --gate."""

    def only_fault(self, d):
        faults, _ = sa.check(d)
        self.assertEqual(len(faults), 1, faults)
        return faults[0]

    def test_a_count_one_too_small_names_the_kernel_stack(self):
        f = self.only_fault(data(
            table=[(0, "unused"), (2, "socket"), (2, "listen"), (3, "accept")]))
        self.assertIn("lxs_args_cnt says 2", f)
        self.assertIn("struct linux_socket_args declares 3", f)
        self.assertIn("kernel stack the copyin never wrote", f)

    def test_a_count_too_large_is_reported_but_differently(self):
        f = self.only_fault(data(
            table=[(0, "unused"), (4, "socket"), (2, "listen"), (3, "accept")]))
        self.assertIn("copies more of the user array", f)
        self.assertNotIn("kernel stack", f)

    def test_a_count_past_the_buffer_is_its_own_fault(self):
        faults, _ = sa.check(data(
            table=[(0, "unused"), (7, "socket"), (2, "listen"), (3, "accept")]))
        self.assertIn("copies 7 words into a 6-word buffer", "\n".join(faults))

    def test_the_buffer_bound_is_the_smaller_of_the_two_arrays(self):
        faults, _ = sa.check(data(
            table=[(0, "unused"), (5, "socket"), (2, "listen"), (3, "accept")],
            bufs={"a": 6, "l_args": 4}))
        self.assertIn("copies 5 words into a 4-word buffer", "\n".join(faults))

    def test_a_handler_with_no_argument_struct_anywhere(self):
        f = self.only_fault(data(cases={
            "LINUX_SOCKET": "linux_socket_nosuch",
            "LINUX_LISTEN": "linux_listen",
            "LINUX_ACCEPT": "linux_accept"}))
        self.assertIn("is not declared anywhere searched", f)

    def test_a_struct_that_disagrees_between_abis(self):
        protos = dict(data()["protos"])
        protos["sys/amd64/linux32/linux32_proto.h"] = PROTO.replace(
            "\tchar protocol_l_[PADL_(l_int)]; l_int protocol; "
            "char protocol_r_[PADR_(l_int)];\n", "")
        f = self.only_fault(data(protos=protos))
        self.assertIn("2 fields in sys/amd64/linux32/linux32_proto.h", f)
        self.assertIn("3 fields in sys/i386/linux/linux_proto.h", f)

    def test_a_struct_the_64bit_abis_have_and_socketcall_does_not(self):
        """socketcall dispatches on i386 and amd64/linux32 only. A struct
        the 64-bit headers declare and those two do not is a gap in the
        tree, not in the reader - the field count is available, and the
        ABI that needs it is the one without it."""
        protos = dict(data()["protos"])
        for rel in sa.SOCKETCALL_PROTO:
            protos[rel] = PROTO[PROTO.index("struct linux_listen_args"):]
        f = self.only_fault(data(protos=protos))
        self.assertIn("LINUX_SOCKET", f)
        self.assertIn("missing from", f)
        self.assertIn("sys/i386/linux/linux_proto.h", f)

    def test_a_struct_declared_beside_the_dispatcher_is_not_missing(self):
        """linux_send_args and linux_recv_args are declared in
        linux_socket.c itself, and linux_accept_args in linux_socket.h.
        Those are not proto-header gaps."""
        protos = dict(data()["protos"])
        protos[sa.SOCKET_C] = protos.pop("sys/i386/linux/linux_proto.h")
        protos["sys/i386/linux/linux_proto.h"] = ""
        protos["sys/amd64/linux32/linux32_proto.h"] = ""
        faults, _ = sa.check(data(protos=protos))
        self.assertEqual(faults, [])

    def test_an_opcode_the_table_covers_that_nothing_dispatches(self):
        f = self.only_fault(data(cases={
            "LINUX_LISTEN": "linux_listen", "LINUX_ACCEPT": "linux_accept"}))
        self.assertIn("LINUX_SOCKET: copied 3 words, dispatched by nothing", f)

    def test_an_opcode_renumbered_past_the_table(self):
        faults, _ = sa.check(data(
            ops={"LINUX_SOCKET": 99, "LINUX_LISTEN": 2, "LINUX_ACCEPT": 3}))
        joined = "\n".join(faults)
        self.assertIn("lxs_args_cnt[1] (socket) has no LINUX_* opcode", joined)
        self.assertIn("LINUX_SOCKET (99)", joined)

    def test_an_opcode_whose_comment_names_a_different_call(self):
        """The table's comments are how a reader checks it. If they have
        drifted from the defines, the reader is being misled even when
        every count happens to be right."""
        f = self.only_fault(data(
            table=[(0, "unused"), (3, "connect"), (2, "listen"), (3, "accept")]))
        self.assertIn("LINUX_SOCKET is 1", f)
        self.assertIn("marked connect", f)

    def test_entry_zero_must_stay_unused(self):
        f = self.only_fault(data(
            table=[(3, "unused"), (3, "socket"), (2, "listen"), (3, "accept")]))
        self.assertIn("lxs_args_cnt[0] is 3", f)

    def test_two_opcodes_sharing_a_value(self):
        faults, _ = sa.check(data(
            ops={"LINUX_SOCKET": 1, "LINUX_LISTEN": 1, "LINUX_ACCEPT": 3}))
        self.assertIn("share a value", "\n".join(faults))


class FaultsToRead(unittest.TestCase):
    """The check going blind. Fails whether or not --gate was asked for."""

    def blind(self, **files):
        real = sa._read

        def fake(rel):
            if rel in files:
                if files[rel] is None:
                    raise sa.Fault(f"{rel}: not in the tree")
                return files[rel]
            return real(rel)

        sa._read = fake
        try:
            buf = io.StringIO()
            with contextlib.redirect_stderr(buf):
                rc = sa.main([])          # NB: no --gate
            return rc, buf.getvalue()
        finally:
            sa._read = real

    def test_the_table_renamed_away(self):
        src = sa._read(sa.SOCKET_C).replace(
            "lxs_args_cnt[] =", "lxs_args_cnt_renamed[] =")
        rc, err = self.blind(**{sa.SOCKET_C: src})
        self.assertEqual(rc, 1)
        self.assertIn("lxs_args_cnt[] not found", err)

    def test_the_opcode_block_comment_gone(self):
        src = sa._read(sa.SOCKET_H).replace(sa._OP_BLOCK, "")
        rc, err = self.blind(**{sa.SOCKET_H: src})
        self.assertEqual(rc, 1)
        self.assertIn("found 0", err)

    def test_a_file_that_is_not_in_the_tree(self):
        rc, err = self.blind(**{sa.SOCKETCALL_PROTO[0]: None})
        self.assertEqual(rc, 1)
        self.assertIn("not in the tree", err)

    def test_a_dispatch_switch_with_no_cases(self):
        """If the switch is ever restructured - a table, a macro - the
        case regex finds nothing, and finding nothing is not a pass."""
        src = sa._read(sa.SOCKET_C)
        head, sep, tail = src.partition("linux_socketcall(")
        rc, err = self.blind(**{sa.SOCKET_C: head + sep + tail.replace(
            "\tcase LINUX_", "\tcase RENAMED_")})
        self.assertEqual(rc, 1)
        self.assertIn("no dispatch cases", err)

    def test_the_copyin_buffer_gone(self):
        src = sa._read(sa.SOCKET_C).replace("\tl_ulong a[6];", "\tl_ulong *a;")
        rc, err = self.blind(**{sa.SOCKET_C: src})
        self.assertEqual(rc, 1)
        self.assertIn("copyin buffer not found", err)


class TheRealTree(unittest.TestCase):
    def test_every_socketcall_opcode_matches_today(self):
        """The claim the 72 linux_socket.c findings rest on."""
        faults, lines = sa.check(sa.collect())
        self.assertEqual(faults, [])
        self.assertEqual(sum(1 for ln in lines if "copies" in ln), 21)

    def test_the_gate_passes_and_says_the_number(self):
        buf = io.StringIO()
        with contextlib.redirect_stdout(buf):
            rc = sa.main(["--gate"])
        self.assertEqual(rc, 0)
        self.assertIn("21 socketcall opcodes", buf.getvalue())


if __name__ == "__main__":
    unittest.main()
