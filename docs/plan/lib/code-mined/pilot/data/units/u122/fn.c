static int
efi_enter(void)
{
	struct thread *td;
	pmap_t curpmap;
	int error;

	if (efi_runtime == NULL)
		return (ENXIO);
	td = curthread;
	curpmap = &td->td_proc->p_vmspace->vm_pmap;
	PMAP_LOCK(curpmap);
	mtx_lock(&efi_lock);
	fpu_kern_enter(td, NULL, FPU_KERN_NOCTX);
	error = efi_arch_enter();
	if (error != 0) {
		fpu_kern_leave(td, NULL);
		mtx_unlock(&efi_lock);
		PMAP_UNLOCK(curpmap);
	} else {
		MPASS((td->td_pflags & TDP_EFIRT) == 0);
		td->td_pflags |= TDP_EFIRT;
	}
	return (error);
}
