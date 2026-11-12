#include "prelude.h"
#undef NEED_WGETPARENT
#define NEED_WGETPARENT 1
#undef concat
#define concat(a,b) a##b
#undef UseShadow
#define UseShadow(dw) ((dw) != 0 && (dw)->normal != 0 && (dw)->shadow != 0)
#undef isprivate
#define isprivate(s) ((s) != 0 && strstr(s, "\033[?") != 0)
#undef TTY_DEVICE
#define TTY_DEVICE "/dev/tty"
#undef dialog_tty
#define dialog_tty() (dlg_getenv_num("DIALOG_TTY", (int *)0) > 0)
#undef C_ATTR
#define C_ATTR(x,y) (((x) != 0 ? A_BOLD :  0) | COLOR_PAIR((y)))
#undef ESCAPE_LEN
#define ESCAPE_LEN 3
#undef isOurEscape
#define isOurEscape(p) (((p)[0] == '\\') && ((p)[1] == 'Z') && ((p)[2] != 0))
#undef SearchTopWindows
#define SearchTopWindows(win) find_window(dialog_state.all_windows, win, TRUE)
#undef SearchSubWindows
#define SearchSubWindows(win) find_window(dialog_state.all_subwindows, win, FALSE)
#undef RepaintCell
#define RepaintCell(dw, draw, y, x) repaint_cell(dw, draw, y, x)
#undef FIX_SINGLE
#define FIX_SINGLE "\n\\"
#undef FIX_DOUBLE
#define FIX_DOUBLE FIX_SINGLE "[]{}?*;`~#$^&()|<>"
#undef HELP_PREFIX
#define HELP_PREFIX		"HELP "
/*@ requires item_no >= 0 && group > 0; ensures \result >= 0; */
int
dlg_calc_listw(int item_no, char **items, int group)
{
    int i, len1 = 0, len2 = 0;

    for (i = 0; i < (item_no * group); i += group) {
	int n;

	if ((n = dlg_count_columns(items[i])) > len1)
	    len1 = n;
	if ((n = dlg_count_columns(items[i + 1])) > len2)
	    len2 = n;
    }
    return len1 + len2;
}
