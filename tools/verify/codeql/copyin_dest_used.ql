/**
 * Local use of a copyin() destination after the call, same function.
 * Not a taint path, not a tree run, not a bug list. Presence of a later
 * access means the extractor recorded both the write and a read.
 */
import cpp

from FunctionCall c, Variable v, VariableAccess a
where
  c.getTarget().getName() = "copyin" and
  c.getArgument(1) = v.getAnAccess() and
  a = v.getAnAccess() and
  a != c.getArgument(1) and
  a.getEnclosingFunction() = c.getEnclosingFunction() and
  a.getLocation().getStartLine() >= c.getLocation().getStartLine()
select a, "copyin dest used later in " + c.getEnclosingFunction().getName()
