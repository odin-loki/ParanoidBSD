static int walEnableBlockingMs(Wal *pWal, int nMs){
  int rc = sqlite3OsFileControl(
      pWal->pDbFd, SQLITE_FCNTL_LOCK_TIMEOUT, (void*)&nMs
  );
  return (rc==SQLITE_OK);
}
