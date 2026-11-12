static void sq_release_buffers(struct sound_queue *sq)
{
	int i;

	if (sq->buffers) {
		for (i = 0; i < sq->numBufs; i++)
			dmasound.mach.dma_free(sq->buffers[i], sq->bufSize);
		kfree(sq->buffers);
		sq->buffers = NULL;
	}
}
