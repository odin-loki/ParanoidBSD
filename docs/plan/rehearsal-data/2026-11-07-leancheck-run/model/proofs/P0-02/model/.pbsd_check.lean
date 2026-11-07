import Model
import Spec
example : Spec.narrow_subset_parent := Model.narrow_subset_parent
#print axioms Model.narrow_subset_parent
example : Spec.derive_keeps_generation := Model.derive_keeps_generation
#print axioms Model.derive_keeps_generation
example : Spec.derive_rights_narrowed := Model.derive_rights_narrowed
#print axioms Model.derive_rights_narrowed
example : Spec.revoked_use_fails := Model.revoked_use_fails
#print axioms Model.revoked_use_fails
example : Spec.stale_fails_closed := Model.stale_fails_closed
#print axioms Model.stale_fails_closed
example : Spec.live_subset_use_ok := Model.live_subset_use_ok
#print axioms Model.live_subset_use_ok
example : Spec.revoke_bumps_generation := Model.revoke_bumps_generation
#print axioms Model.revoke_bumps_generation
example : Spec.restart_clears_and_bumps := Model.restart_clears_and_bumps
#print axioms Model.restart_clears_and_bumps
example : Spec.restart_token_stale_fails := Model.restart_token_stale_fails
#print axioms Model.restart_token_stale_fails
example : Spec.restart_new_token_live := Model.restart_new_token_live
#print axioms Model.restart_new_token_live
example : Spec.posix_read_iff := Model.posix_read_iff
#print axioms Model.posix_read_iff
example : Spec.nt_delete_iff := Model.nt_delete_iff
#print axioms Model.nt_delete_iff
example : Spec.android_untrusted_rights_empty := Model.android_untrusted_rights_empty
#print axioms Model.android_untrusted_rights_empty
