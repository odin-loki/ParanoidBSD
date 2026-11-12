#include "prelude.h"
#undef __MLX5E_FS_ETHTOOL_H__
#define __MLX5E_FS_ETHTOOL_H__
struct mlx5e_priv;
struct mlx5e_ethtool_steering;
/*@ requires true; ensures \result == 0; */
static inline int mlx5e_ethtool_alloc(struct mlx5e_ethtool_steering **ethtool)
{ return 0; }
