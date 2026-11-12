# cls.awk  append a class column: ptr when the contract mentions a pointer/field/array/quantifier, else scalar
BEGIN { FS = OFS = "\t" }
{ c = $8; gsub(/\/\*@|\*\//, "", c); cls = (c ~ /\\valid|->|\*|\[|\\forall|\\exists/) ? "ptr" : "scalar"; print $0, cls }
