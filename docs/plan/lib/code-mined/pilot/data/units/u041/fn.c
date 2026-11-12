static int
is_struct(Type *t, int isstruct)
{
    size_t i;

    if (t->type == TType)
	return 0;
    if (t->type == TSequence || t->type == TSet || t->type == TChoice)
	return 1;
    if (t->type == TTag)
	return is_struct(t->subtype, isstruct);

    for (i = 0; i < sizeof(types)/sizeof(types[0]); i++) {
	if (t->type == types[i].type) {
	    if (types[i].is_struct == 0)
		return 0;
	    else
		break;
	}
    }

    return isstruct;
}
