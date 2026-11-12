#include "prelude.h"
#undef pr_fmt
#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt
#undef OVS_ATTR_NESTED
#define OVS_ATTR_NESTED -1
#undef OVS_ATTR_VARIABLE
#define OVS_ATTR_VARIABLE -2
#undef OVS_COPY_ACTIONS_MAX_DEPTH
#define OVS_COPY_ACTIONS_MAX_DEPTH 16
#undef SW_FLOW_KEY_PUT
#define SW_FLOW_KEY_PUT(match, field, value, is_mask)  	do {  		update_range(match, offsetof(struct sw_flow_key, field),     			     sizeof((match)->key->field), is_mask);	     		if (is_mask)						     			(match)->mask->key.field = value;		     		else							     			(match)->key->field = value;		             	} while (0)
#undef SW_FLOW_KEY_MEMCPY_OFFSET
#define SW_FLOW_KEY_MEMCPY_OFFSET(match, offset, value_p, len, is_mask)	     	do {								     		update_range(match, offset, len, is_mask);		     		if (is_mask)						     			memcpy((u8 *)&(match)->mask->key + offset, value_p,  			       len);					    		else							     			memcpy((u8 *)(match)->key + offset, value_p, len);   	} while (0)
#undef SW_FLOW_KEY_MEMCPY
#define SW_FLOW_KEY_MEMCPY(match, field, value_p, len, is_mask)		       	SW_FLOW_KEY_MEMCPY_OFFSET(match, offsetof(struct sw_flow_key, field),  				  value_p, len, is_mask)
#undef SW_FLOW_KEY_MEMSET_FIELD
#define SW_FLOW_KEY_MEMSET_FIELD(match, field, value, is_mask)		     	do {								     		update_range(match, offsetof(struct sw_flow_key, field),     			     sizeof((match)->key->field), is_mask);	     		if (is_mask)						     			memset((u8 *)&(match)->mask->key.field, value,       			       sizeof((match)->mask->key.field));	     		else							     			memset((u8 *)&(match)->key->field, value,            			       sizeof((match)->key->field));                 	} while (0)
#undef SW_FLOW_KEY_BITMAP_COPY
#define SW_FLOW_KEY_BITMAP_COPY(match, field, value_p, nbits, is_mask) ({      	update_range(match, offsetof(struct sw_flow_key, field),	       		     bitmap_size(nbits), is_mask);			       	bitmap_copy(is_mask ? (match)->mask->key.field : (match)->key->field,  		    value_p, nbits);					       })
struct ovs_len_tbl { 	int len; 	const struct ovs_len_tbl *next; };
/*@ requires \true; ensures \result == 1 <==> (expected_len == attr_len || expected_len == OVS_ATTR_NESTED || expected_len == OVS_ATTR_VARIABLE); */
static bool check_attr_len(unsigned int attr_len, unsigned int expected_len)
{
	return expected_len == attr_len ||
	       expected_len == OVS_ATTR_NESTED ||
	       expected_len == OVS_ATTR_VARIABLE;
}
