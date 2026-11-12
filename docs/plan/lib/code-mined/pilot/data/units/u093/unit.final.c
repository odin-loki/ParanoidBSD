#include "prelude.h"
#undef BUFFER_GROWFACTOR
#define BUFFER_GROWFACTOR	1.618
#undef BUFFER_GROW
#define BUFFER_GROW(x)		(((x)+0.5)*BUFFER_GROWFACTOR)
#undef ELFTC_FAILURE
#define	ELFTC_FAILURE		0
#undef ELFTC_ISDIGIT
#define	ELFTC_ISDIGIT(C) 	(isdigit((C) & 0xFF))
#undef ELFTC_SUCCESS
#define	ELFTC_SUCCESS		1
#undef VECTOR_DEF_CAPACITY
#define VECTOR_DEF_CAPACITY	8
#undef CPP_DEMANGLE_TRY_LIMIT
#define	CPP_DEMANGLE_TRY_LIMIT	128
#undef FLOAT_SPRINTF_TRY_LIMIT
#define	FLOAT_SPRINTF_TRY_LIMIT	5
#undef FLOAT_QUADRUPLE_BYTES
#define	FLOAT_QUADRUPLE_BYTES	16
#undef FLOAT_EXTENED_BYTES
#define	FLOAT_EXTENED_BYTES	10
#undef SIMPLE_HASH
#define SIMPLE_HASH(x,y)	(64 * x + y)
#undef DEM_PUSH_STR
#define DEM_PUSH_STR(d,s)	cpp_demangle_push_str((d), (s), strlen((s)))
#undef VEC_PUSH_STR
#define VEC_PUSH_STR(d,s)	vector_str_push((d), (s), strlen((s)))
struct vector_str { 	 	size_t		size; 	 	size_t		capacity; 	 	char		**container; };
enum type_qualifier { 	TYPE_PTR, TYPE_REF, TYPE_CMX, TYPE_IMG, TYPE_EXT, TYPE_RST, TYPE_VAT, 	TYPE_CST, TYPE_VEC, TYPE_RREF };
struct vector_type_qualifier { 	size_t size, capacity; 	enum type_qualifier *q_container; 	struct vector_str ext_name; };
enum read_cmd { 	READ_FAIL, READ_NEST, READ_TMPL, READ_EXPR, READ_EXPL, READ_LOCAL, 	READ_TYPE, READ_FUNC, READ_PTRMEM };
struct read_cmd_item { 	enum read_cmd cmd; 	void *data; };
struct vector_read_cmd { 	size_t size, capacity; 	struct read_cmd_item *r_container; };
enum push_qualifier { 	PUSH_ALL_QUALIFIER, 	PUSH_CV_QUALIFIER, 	PUSH_NON_CV_QUALIFIER, };
struct cpp_demangle_data { 	struct vector_str	 output;	 	struct vector_str	 subst;		 	struct vector_str	 tmpl; 	struct vector_str	 class_type; 	struct vector_str	*cur_output;	 	struct vector_read_cmd	 cmd; 	bool			 mem_rst;	 	bool			 mem_vat;	 	bool			 mem_cst;	 	bool			 mem_ref;	 	bool			 mem_rref;	 	bool			 is_tmpl;	 	bool			 is_functype;	 	bool			 ref_qualifier;  	enum type_qualifier	 ref_qualifier_type;  	enum push_qualifier	 push_qualifier;  	int			 func_type; 	const char		*cur;		 	const char		*last_sname;	 };
struct type_delimit { 	bool paren; 	bool firstp; };
/*@ requires ddata != NULL && v != NULL; ensures \result == 1 || \result == 0; */
static int
cpp_demangle_push_type_qualifier(struct cpp_demangle_data *ddata,
    struct vector_type_qualifier *v, const char *type_str)
{
	struct vector_str subst_v;
	enum type_qualifier t;
	size_t idx, e_idx, e_len;
	char *buf;
	int rtn;
	bool cv;

	if (ddata == NULL || v == NULL)
		return (0);

	if ((idx = v->size) == 0)
		return (1);

	rtn = 0;
	if (type_str != NULL) {
		if (!vector_str_init(&subst_v))
			return (0);
		if (!VEC_PUSH_STR(&subst_v, type_str))
			goto clean;
	}

	cv = true;
	e_idx = 0;
	while (idx > 0) {
		switch (v->q_container[idx - 1]) {
		case TYPE_PTR:
			cv = false;
			if (ddata->push_qualifier == PUSH_CV_QUALIFIER)
				break;
			if (!DEM_PUSH_STR(ddata, "*"))
				goto clean;
			if (type_str != NULL) {
				if (!VEC_PUSH_STR(&subst_v, "*"))
					goto clean;
				if (!cpp_demangle_push_subst_v(ddata,
				    &subst_v))
					goto clean;
			}
			break;

		case TYPE_REF:
			cv = false;
			if (ddata->push_qualifier == PUSH_CV_QUALIFIER)
				break;
			if (!DEM_PUSH_STR(ddata, "&"))
				goto clean;
			if (type_str != NULL) {
				if (!VEC_PUSH_STR(&subst_v, "&"))
					goto clean;
				if (!cpp_demangle_push_subst_v(ddata,
				    &subst_v))
					goto clean;
			}
			break;

		case TYPE_RREF:
			cv = false;
			if (ddata->push_qualifier == PUSH_CV_QUALIFIER)
				break;
			if (!DEM_PUSH_STR(ddata, "&&"))
				goto clean;
			if (type_str != NULL) {
				if (!VEC_PUSH_STR(&subst_v, "&&"))
					goto clean;
				if (!cpp_demangle_push_subst_v(ddata,
				    &subst_v))
					goto clean;
			}
			break;

		case TYPE_CMX:
			cv = false;
			if (ddata->push_qualifier == PUSH_CV_QUALIFIER)
				break;
			if (!DEM_PUSH_STR(ddata, " complex"))
				goto clean;
			if (type_str != NULL) {
				if (!VEC_PUSH_STR(&subst_v, " complex"))
					goto clean;
				if (!cpp_demangle_push_subst_v(ddata,
				    &subst_v))
					goto clean;
			}
			break;

		case TYPE_IMG:
			cv = false;
			if (ddata->push_qualifier == PUSH_CV_QUALIFIER)
				break;
			if (!DEM_PUSH_STR(ddata, " imaginary"))
				goto clean;
			if (type_str != NULL) {
				if (!VEC_PUSH_STR(&subst_v, " imaginary"))
					goto clean;
				if (!cpp_demangle_push_subst_v(ddata,
				    &subst_v))
					goto clean;
			}
			break;

		case TYPE_EXT:
			cv = false;
			if (ddata->push_qualifier == PUSH_CV_QUALIFIER)
				break;
			if (v->ext_name.size == 0 ||
			    e_idx > v->ext_name.size - 1)
				goto clean;
			if ((e_len = strlen(v->ext_name.container[e_idx])) ==
			    0)
				goto clean;
			if ((buf = malloc(e_len + 2)) == NULL)
				goto clean;
			snprintf(buf, e_len + 2, " %s",
			    v->ext_name.container[e_idx]);

			if (!DEM_PUSH_STR(ddata, buf)) {
				free(buf);
				goto clean;
			}

			if (type_str != NULL) {
				if (!VEC_PUSH_STR(&subst_v, buf)) {
					free(buf);
					goto clean;
				}
				if (!cpp_demangle_push_subst_v(ddata,
				    &subst_v)) {
					free(buf);
					goto clean;
				}
			}
			free(buf);
			++e_idx;
			break;

		case TYPE_RST:
			if (ddata->push_qualifier == PUSH_NON_CV_QUALIFIER &&
			    cv)
				break;
			if (ddata->push_qualifier == PUSH_CV_QUALIFIER && !cv)
				break;
			if (!DEM_PUSH_STR(ddata, " restrict"))
				goto clean;
			if (type_str != NULL) {
				if (!VEC_PUSH_STR(&subst_v, " restrict"))
					goto clean;
				if (idx - 1 > 0) {
					t = v->q_container[idx - 2];
					if (t == TYPE_RST || t == TYPE_VAT ||
					    t == TYPE_CST)
						break;
				}
				if (!cpp_demangle_push_subst_v(ddata,
				    &subst_v))
					goto clean;
			}
			break;

		case TYPE_VAT:
			if (ddata->push_qualifier == PUSH_NON_CV_QUALIFIER &&
			    cv)
				break;
			if (ddata->push_qualifier == PUSH_CV_QUALIFIER && !cv)
				break;
			if (!DEM_PUSH_STR(ddata, " volatile"))
				goto clean;
			if (type_str != NULL) {
				if (!VEC_PUSH_STR(&subst_v, " volatile"))
					goto clean;
				if (idx - 1 > 0) {
					t = v->q_container[idx - 2];
					if (t == TYPE_RST || t == TYPE_VAT ||
					    t == TYPE_CST)
						break;
				}
				if (!cpp_demangle_push_subst_v(ddata,
				    &subst_v))
					goto clean;
			}
			break;

		case TYPE_CST:
			if (ddata->push_qualifier == PUSH_NON_CV_QUALIFIER &&
			    cv)
				break;
			if (ddata->push_qualifier == PUSH_CV_QUALIFIER && !cv)
				break;
			if (!DEM_PUSH_STR(ddata, " const"))
				goto clean;
			if (type_str != NULL) {
				if (!VEC_PUSH_STR(&subst_v, " const"))
					goto clean;
				if (idx - 1 > 0) {
					t = v->q_container[idx - 2];
					if (t == TYPE_RST || t == TYPE_VAT ||
					    t == TYPE_CST)
						break;
				}
				if (!cpp_demangle_push_subst_v(ddata,
				    &subst_v))
					goto clean;
			}
			break;

		case TYPE_VEC:
			cv = false;
			if (ddata->push_qualifier == PUSH_CV_QUALIFIER)
				break;
			if (v->ext_name.size == 0 ||
			    e_idx > v->ext_name.size - 1)
				goto clean;
			if ((e_len = strlen(v->ext_name.container[e_idx])) ==
			    0)
				goto clean;
			if ((buf = malloc(e_len + 12)) == NULL)
				goto clean;
			snprintf(buf, e_len + 12, " __vector(%s)",
			    v->ext_name.container[e_idx]);
			if (!DEM_PUSH_STR(ddata, buf)) {
				free(buf);
				goto clean;
			}
			if (type_str != NULL) {
				if (!VEC_PUSH_STR(&subst_v, buf)) {
					free(buf);
					goto clean;
				}
				if (!cpp_demangle_push_subst_v(ddata,
				    &subst_v)) {
					free(buf);
					goto clean;
				}
			}
			free(buf);
			++e_idx;
			break;
		}
		--idx;
	}

	rtn = 1;
clean:
	if (type_str != NULL)
		vector_str_dest(&subst_v);

	return (rtn);
}
