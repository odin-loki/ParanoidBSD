/**
 * Sees a copyin() call. Not a taint path, not a tree run.
 */
import cpp

from FunctionCall c
where c.getTarget().getName() = "copyin"
select c, "copyin call in " + c.getEnclosingFunction().getName()
