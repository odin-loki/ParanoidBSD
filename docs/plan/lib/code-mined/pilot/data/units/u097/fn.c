static int do_meta_command(char *zLine, ShellState *p){
  int h = 1;
  int nArg = 0;
  int n, c;
  int rc = 0;
  char *azArg[52];

#ifndef SQLITE_OMIT_VIRTUALTABLE
  if( p->expert.pExpert ){
    expertFinish(p, 1, 0);
  }
#endif

  /* Parse the input line into tokens.
  */
  while( zLine[h] && nArg<ArraySize(azArg)-1 ){
    while( IsSpace(zLine[h]) ){ h++; }
    if( zLine[h]==0 ) break;
    if( zLine[h]=='\'' || zLine[h]=='"' ){
      int delim = zLine[h++];
      azArg[nArg++] = &zLine[h];
      while( zLine[h] && zLine[h]!=delim ){
        if( zLine[h]=='\\' && delim=='"' && zLine[h+1]!=0 ) h++;
        h++;
      }
      if( zLine[h]==delim ){
        zLine[h++] = 0;
      }
      if( delim=='"' ) resolve_backslashes(azArg[nArg-1]);
    }else{
      azArg[nArg++] = &zLine[h];
      while( zLine[h] && !IsSpace(zLine[h]) ){ h++; }
      if( zLine[h] ) zLine[h++] = 0;
    }
  }
  azArg[nArg] = 0;

  /* Process the input line.
  */
  if( nArg==0 ) return 0; /* no tokens, no error */
  n = strlen30(azArg[0]);
  c = azArg[0][0];
  clearTempFile(p);

#ifndef SQLITE_OMIT_AUTHORIZATION
  if( c=='a' && cli_strncmp(azArg[0], "auth", n)==0 ){
    if( nArg!=2 ){
      sqlite3_fprintf(stderr, "Usage: .auth ON|OFF\n");
      rc = 1;
      goto meta_command_exit;
    }
    open_db(p, 0);
    if( booleanValue(azArg[1]) ){
      sqlite3_set_authorizer(p->db, shellAuth, p);
    }else if( p->bSafeModePersist ){
      sqlite3_set_authorizer(p->db, safeModeAuth, p);
    }else{
      sqlite3_set_authorizer(p->db, 0, 0);
    }
  }else
#endif

#if !defined(SQLITE_OMIT_VIRTUALTABLE) && defined(SQLITE_HAVE_ZLIB) \
  && !defined(SQLITE_SHELL_FIDDLE)
  if( c=='a' && cli_strncmp(azArg[0], "archive", n)==0 ){
    open_db(p, 0);
    failIfSafeMode(p, "cannot run .archive in safe mode");
    rc = arDotCommand(p, 0, azArg, nArg);
  }else
#endif

#ifndef SQLITE_SHELL_FIDDLE
  if( (c=='b' && n>=3 && cli_strncmp(azArg[0], "backup", n)==0)
   || (c=='s' && n>=3 && cli_strncmp(azArg[0], "save", n)==0)
  ){
    const char *zDestFile = 0;
    const char *zDb = 0;
    sqlite3 *pDest;
    sqlite3_backup *pBackup;
    int j;
    int bAsync = 0;
    const char *zVfs = 0;
    failIfSafeMode(p, "cannot run .%s in safe mode", azArg[0]);
    for(j=1; j<nArg; j++){
      const char *z = azArg[j];
      if( z[0]=='-' ){
        if( z[1]=='-' ) z++;
        if( cli_strcmp(z, "-append")==0 ){
          zVfs = "apndvfs";
        }else
        if( cli_strcmp(z, "-async")==0 ){
          bAsync = 1;
        }else
        {
          sqlite3_fprintf(stderr,"unknown option: %s\n", azArg[j]);
          return 1;
        }
      }else if( zDestFile==0 ){
        zDestFile = azArg[j];
      }else if( zDb==0 ){
        zDb = zDestFile;
        zDestFile = azArg[j];
      }else{
        sqlite3_fprintf(stderr, "Usage: .backup ?DB? ?OPTIONS? FILENAME\n");
        return 1;
      }
    }
    if( zDestFile==0 ){
      sqlite3_fprintf(stderr, "missing FILENAME argument on .backup\n");
      return 1;
    }
    if( zDb==0 ) zDb = "main";
    rc = sqlite3_open_v2(zDestFile, &pDest,
                  SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE, zVfs);
    if( rc!=SQLITE_OK ){
      sqlite3_fprintf(stderr,"Error: cannot open \"%s\"\n", zDestFile);
      close_db(pDest);
      return 1;
    }
    if( bAsync ){
      sqlite3_exec(pDest, "PRAGMA synchronous=OFF; PRAGMA journal_mode=OFF;",
                   0, 0, 0);
    }
    open_db(p, 0);
    pBackup = sqlite3_backup_init(pDest, "main", p->db, zDb);
    if( pBackup==0 ){
      shellDatabaseError(pDest);
      close_db(pDest);
      return 1;
    }
    while(  (rc = sqlite3_backup_step(pBackup,100))==SQLITE_OK ){}
    sqlite3_backup_finish(pBackup);
    if( rc==SQLITE_DONE ){
      rc = 0;
    }else{
      shellDatabaseError(pDest);
      rc = 1;
    }
    close_db(pDest);
  }else
#endif /* !defined(SQLITE_SHELL_FIDDLE) */

  if( c=='b' && n>=3 && cli_strncmp(azArg[0], "bail", n)==0 ){
    if( nArg==2 ){
      bail_on_error = booleanValue(azArg[1]);
    }else{
      eputz("Usage: .bail on|off\n");
      rc = 1;
    }
  }else

  /* Undocumented.  Legacy only.  See "crlf" below */
  if( c=='b' && n>=3 && cli_strncmp(azArg[0], "binary", n)==0 ){
    eputz("The \".binary\" command is deprecated.\n");
    rc = 1;
  }else

  /* The undocumented ".breakpoint" command causes a call to the no-op
  ** routine named test_breakpoint().
  */
  if( c=='b' && n>=3 && cli_strncmp(azArg[0], "breakpoint", n)==0 ){
    test_breakpoint();
  }else

#ifndef SQLITE_SHELL_FIDDLE
  if( c=='c' && cli_strcmp(azArg[0],"cd")==0 ){
    failIfSafeMode(p, "cannot run .cd in safe mode");
    if( nArg==2 ){
#if defined(_WIN32) || defined(WIN32)
      wchar_t *z = sqlite3_win32_utf8_to_unicode(azArg[1]);
      rc = !SetCurrentDirectoryW(z);
      sqlite3_free(z);
#else
      rc = chdir(azArg[1]);
#endif
      if( rc ){
        sqlite3_fprintf(stderr,"Cannot change to directory \"%s\"\n", azArg[1]);
        rc = 1;
      }
    }else{
      eputz("Usage: .cd DIRECTORY\n");
      rc = 1;
    }
  }else
#endif /* !defined(SQLITE_SHELL_FIDDLE) */

  if( c=='c' && n>=3 && cli_strncmp(azArg[0], "changes", n)==0 ){
    if( nArg==2 ){
      setOrClearFlag(p, SHFLG_CountChanges, azArg[1]);
    }else{
      eputz("Usage: .changes on|off\n");
      rc = 1;
    }
  }else

#ifndef SQLITE_SHELL_FIDDLE
  /* Cancel output redirection, if it is currently set (by .testcase)
  ** Then read the content of the testcase-out.txt file and compare against
  ** azArg[1].  If there are differences, report an error and exit.
  */
  if( c=='c' && n>=3 && cli_strncmp(azArg[0], "check", n)==0 ){
    char *zRes = 0;
    output_reset(p);
    if( nArg!=2 ){
      eputz("Usage: .check GLOB-PATTERN\n");
      rc = 2;
    }else if( (zRes = readFile("testcase-out.txt", 0))==0 ){
      rc = 2;
    }else if( testcase_glob(azArg[1],zRes)==0 ){
      sqlite3_fprintf(stderr,
            "testcase-%s FAILED\n Expected: [%s]\n      Got: [%s]\n",
            p->zTestcase, azArg[1], zRes);
      rc = 1;
    }else{
      sqlite3_fprintf(p->out, "testcase-%s ok\n", p->zTestcase);
      p->nCheck++;
    }
    sqlite3_free(zRes);
  }else
#endif /* !defined(SQLITE_SHELL_FIDDLE) */

#ifndef SQLITE_SHELL_FIDDLE
  if( c=='c' && cli_strncmp(azArg[0], "clone", n)==0 ){
    failIfSafeMode(p, "cannot run .clone in safe mode");
    if( nArg==2 ){
      tryToClone(p, azArg[1]);
    }else{
      eputz("Usage: .clone FILENAME\n");
      rc = 1;
    }
  }else
#endif /* !defined(SQLITE_SHELL_FIDDLE) */

  if( c=='c' && cli_strncmp(azArg[0], "connection", n)==0 ){
    if( nArg==1 ){
      /* List available connections */
      int i;
      for(i=0; i<ArraySize(p->aAuxDb); i++){
        const char *zFile = p->aAuxDb[i].zDbFilename;
        if( p->aAuxDb[i].db==0 && p->pAuxDb!=&p->aAuxDb[i] ){
          zFile = "(not open)";
        }else if( zFile==0 ){
          zFile = "(memory)";
        }else if( zFile[0]==0 ){
          zFile = "(temporary-file)";
        }
        if( p->pAuxDb == &p->aAuxDb[i] ){
          sqlite3_fprintf(stdout, "ACTIVE %d: %s\n", i, zFile);
        }else if( p->aAuxDb[i].db!=0 ){
          sqlite3_fprintf(stdout, "       %d: %s\n", i, zFile);
        }
      }
    }else if( nArg==2 && IsDigit(azArg[1][0]) && azArg[1][1]==0 ){
      int i = azArg[1][0] - '0';
      if( p->pAuxDb != &p->aAuxDb[i] && i>=0 && i<ArraySize(p->aAuxDb) ){
        p->pAuxDb->db = p->db;
        p->pAuxDb = &p->aAuxDb[i];
        globalDb = p->db = p->pAuxDb->db;
        p->pAuxDb->db = 0;
      }
    }else if( nArg==3 && cli_strcmp(azArg[1], "close")==0
           && IsDigit(azArg[2][0]) && azArg[2][1]==0 ){
      int i = azArg[2][0] - '0';
      if( i<0 || i>=ArraySize(p->aAuxDb) ){
        /* No-op */
      }else if( p->pAuxDb == &p->aAuxDb[i] ){
        eputz("cannot close the active database connection\n");
        rc = 1;
      }else if( p->aAuxDb[i].db ){
        session_close_all(p, i);
        close_db(p->aAuxDb[i].db);
        p->aAuxDb[i].db = 0;
      }
    }else{
      eputz("Usage: .connection [close] [CONNECTION-NUMBER]\n");
      rc = 1;
    }
  }else

  if( c=='c' && n==4
   && (cli_strncmp(azArg[0], "crlf", n)==0
       || cli_strncmp(azArg[0], "crnl",n)==0)
  ){
    if( nArg==2 ){
#ifdef _WIN32
      p->crlfMode = booleanValue(azArg[1]);
#else
      p->crlfMode = 0;
#endif
    }
    sqlite3_fprintf(stderr, "crlf is %s\n", p->crlfMode ? "ON" : "OFF");
  }else

  if( c=='d' && n>1 && cli_strncmp(azArg[0], "databases", n)==0 ){
    char **azName = 0;
    int nName = 0;
    sqlite3_stmt *pStmt;
    int i;
    open_db(p, 0);
    rc = sqlite3_prepare_v2(p->db, "PRAGMA database_list", -1, &pStmt, 0);
    if( rc ){
      shellDatabaseError(p->db);
      rc = 1;
    }else{
      while( sqlite3_step(pStmt)==SQLITE_ROW ){
        const char *zSchema = (const char *)sqlite3_column_text(pStmt,1);
        const char *zFile = (const char*)sqlite3_column_text(pStmt,2);
        if( zSchema==0 || zFile==0 ) continue;
        azName = sqlite3_realloc(azName, (nName+1)*2*sizeof(char*));
        shell_check_oom(azName);
        azName[nName*2] = strdup(zSchema);
        azName[nName*2+1] = strdup(zFile);
        nName++;
      }
    }
    sqlite3_finalize(pStmt);
    for(i=0; i<nName; i++){
      int eTxn = sqlite3_txn_state(p->db, azName[i*2]);
      int bRdonly = sqlite3_db_readonly(p->db, azName[i*2]);
      const char *z = azName[i*2+1];
      sqlite3_fprintf(p->out, "%s: %s %s%s\n",
            azName[i*2], z && z[0] ? z : "\"\"", bRdonly ? "r/o" : "r/w",
            eTxn==SQLITE_TXN_NONE ? "" :
            eTxn==SQLITE_TXN_READ ? " read-txn" : " write-txn");
      free(azName[i*2]);
      free(azName[i*2+1]);
    }
    sqlite3_free(azName);
  }else

  if( c=='d' && n>=3 && cli_strncmp(azArg[0], "dbconfig", n)==0 ){
    static const struct DbConfigChoices {
      const char *zName;
      int op;
    } aDbConfig[] = {
        { "attach_create",      SQLITE_DBCONFIG_ENABLE_ATTACH_CREATE  },
        { "attach_write",       SQLITE_DBCONFIG_ENABLE_ATTACH_WRITE   },
        { "comments",           SQLITE_DBCONFIG_ENABLE_COMMENTS       },
        { "defensive",          SQLITE_DBCONFIG_DEFENSIVE             },
        { "dqs_ddl",            SQLITE_DBCONFIG_DQS_DDL               },
        { "dqs_dml",            SQLITE_DBCONFIG_DQS_DML               },
        { "enable_fkey",        SQLITE_DBCONFIG_ENABLE_FKEY           },
        { "enable_qpsg",        SQLITE_DBCONFIG_ENABLE_QPSG           },
        { "enable_trigger",     SQLITE_DBCONFIG_ENABLE_TRIGGER        },
        { "enable_view",        SQLITE_DBCONFIG_ENABLE_VIEW           },
        { "fts3_tokenizer",     SQLITE_DBCONFIG_ENABLE_FTS3_TOKENIZER },
        { "legacy_alter_table", SQLITE_DBCONFIG_LEGACY_ALTER_TABLE    },
        { "legacy_file_format", SQLITE_DBCONFIG_LEGACY_FILE_FORMAT    },
        { "load_extension",     SQLITE_DBCONFIG_ENABLE_LOAD_EXTENSION },
        { "no_ckpt_on_close",   SQLITE_DBCONFIG_NO_CKPT_ON_CLOSE      },
        { "reset_database",     SQLITE_DBCONFIG_RESET_DATABASE        },
        { "reverse_scanorder",  SQLITE_DBCONFIG_REVERSE_SCANORDER     },
        { "stmt_scanstatus",    SQLITE_DBCONFIG_STMT_SCANSTATUS       },
        { "trigger_eqp",        SQLITE_DBCONFIG_TRIGGER_EQP           },
        { "trusted_schema",     SQLITE_DBCONFIG_TRUSTED_SCHEMA        },
        { "writable_schema",    SQLITE_DBCONFIG_WRITABLE_SCHEMA       },
    };
    int ii, v;
    open_db(p, 0);
    for(ii=0; ii<ArraySize(aDbConfig); ii++){
      if( nArg>1 && cli_strcmp(azArg[1], aDbConfig[ii].zName)!=0 ) continue;
      if( nArg>=3 ){
        sqlite3_db_config(p->db, aDbConfig[ii].op, booleanValue(azArg[2]), 0);
      }
      sqlite3_db_config(p->db, aDbConfig[ii].op, -1, &v);
      sqlite3_fprintf(p->out, "%19s %s\n",
                      aDbConfig[ii].zName, v ? "on" : "off");
      if( nArg>1 ) break;
    }
    if( nArg>1 && ii==ArraySize(aDbConfig) ){
      sqlite3_fprintf(stderr,"Error: unknown dbconfig \"%s\"\n", azArg[1]);
      eputz("Enter \".dbconfig\" with no arguments for a list\n");
    }
  }else

#if SQLITE_SHELL_HAVE_RECOVER
  if( c=='d' && n>=3 && cli_strncmp(azArg[0], "dbinfo", n)==0 ){
    rc = shell_dbinfo_command(p, nArg, azArg);
  }else

  if( c=='r' && cli_strncmp(azArg[0], "recover", n)==0 ){
    open_db(p, 0);
    rc = recoverDatabaseCmd(p, nArg, azArg);
  }else
#endif /* SQLITE_SHELL_HAVE_RECOVER */

  if( c=='d' && cli_strncmp(azArg[0], "dump", n)==0 ){
    char *zLike = 0;
    char *zSql;
    int i;
    int savedShowHeader = p->showHeader;
    int savedShellFlags = p->shellFlgs;
    ShellClearFlag(p,
       SHFLG_PreserveRowid|SHFLG_Newlines|SHFLG_Echo
       |SHFLG_DumpDataOnly|SHFLG_DumpNoSys);
    for(i=1; i<nArg; i++){
      if( azArg[i][0]=='-' ){
        const char *z = azArg[i]+1;
        if( z[0]=='-' ) z++;
        if( cli_strcmp(z,"preserve-rowids")==0 ){
#ifdef SQLITE_OMIT_VIRTUALTABLE
          eputz("The --preserve-rowids option is not compatible"
                " with SQLITE_OMIT_VIRTUALTABLE\n");
          rc = 1;
          sqlite3_free(zLike);
          goto meta_command_exit;
#else
          ShellSetFlag(p, SHFLG_PreserveRowid);
#endif
        }else
        if( cli_strcmp(z,"newlines")==0 ){
          ShellSetFlag(p, SHFLG_Newlines);
        }else
        if( cli_strcmp(z,"data-only")==0 ){
          ShellSetFlag(p, SHFLG_DumpDataOnly);
        }else
        if( cli_strcmp(z,"nosys")==0 ){
          ShellSetFlag(p, SHFLG_DumpNoSys);
        }else
        {
          sqlite3_fprintf(stderr,
               "Unknown option \"%s\" on \".dump\"\n", azArg[i]);
          rc = 1;
          sqlite3_free(zLike);
          goto meta_command_exit;
        }
      }else{
        /* azArg[i] contains a LIKE pattern. This ".dump" request should
        ** only dump data for tables for which either the table name matches
        ** the LIKE pattern, or the table appears to be a shadow table of
        ** a virtual table for which the name matches the LIKE pattern.
        */
        char *zExpr = sqlite3_mprintf(
            "name LIKE %Q ESCAPE '\\' OR EXISTS ("
            "  SELECT 1 FROM sqlite_schema WHERE "
            "    name LIKE %Q ESCAPE '\\' AND"
            "    sql LIKE 'CREATE VIRTUAL TABLE%%' AND"
            "    substr(o.name, 1, length(name)+1) == (name||'_')"
            ")", azArg[i], azArg[i]
        );

        if( zLike ){
          zLike = sqlite3_mprintf("%z OR %z", zLike, zExpr);
        }else{
          zLike = zExpr;
        }
      }
    }

    open_db(p, 0);

    outputDumpWarning(p, zLike);
    if( (p->shellFlgs & SHFLG_DumpDataOnly)==0 ){
      /* When playing back a "dump", the content might appear in an order
      ** which causes immediate foreign key constraints to be violated.
      ** So disable foreign-key constraint enforcement to prevent problems. */
      sqlite3_fputs("PRAGMA foreign_keys=OFF;\n", p->out);
      sqlite3_fputs("BEGIN TRANSACTION;\n", p->out);
    }
    p->writableSchema = 0;
    p->showHeader = 0;
    /* Set writable_schema=ON since doing so forces SQLite to initialize
    ** as much of the schema as it can even if the sqlite_schema table is
    ** corrupt. */
    sqlite3_exec(p->db, "SAVEPOINT dump; PRAGMA writable_schema=ON", 0, 0, 0);
    p->nErr = 0;
    if( zLike==0 ) zLike = sqlite3_mprintf("true");
    zSql = sqlite3_mprintf(
      "SELECT name, type, sql FROM sqlite_schema AS o "
      "WHERE (%s) AND type=='table'"
      "  AND sql NOT NULL"
      " ORDER BY tbl_name='sqlite_sequence', rowid",
      zLike
    );
    run_schema_dump_query(p,zSql);
    sqlite3_free(zSql);
    if( (p->shellFlgs & SHFLG_DumpDataOnly)==0 ){
      zSql = sqlite3_mprintf(
        "SELECT sql FROM sqlite_schema AS o "
        "WHERE (%s) AND sql NOT NULL"
        "  AND type IN ('index','trigger','view') "
        "ORDER BY type COLLATE NOCASE DESC",
        zLike
      );
      run_table_dump_query(p, zSql);
      sqlite3_free(zSql);
    }
    sqlite3_free(zLike);
    if( p->writableSchema ){
      sqlite3_fputs("PRAGMA writable_schema=OFF;\n", p->out);
      p->writableSchema = 0;
    }
    sqlite3_exec(p->db, "PRAGMA writable_schema=OFF;", 0, 0, 0);
    sqlite3_exec(p->db, "RELEASE dump;", 0, 0, 0);
    if( (p->shellFlgs & SHFLG_DumpDataOnly)==0 ){
      sqlite3_fputs(p->nErr?"ROLLBACK; -- due to errors\n":"COMMIT;\n", p->out);
    }
    p->showHeader = savedShowHeader;
    p->shellFlgs = savedShellFlags;
  }else

  if( c=='e' && cli_strncmp(azArg[0], "echo", n)==0 ){
    if( nArg==2 ){
      setOrClearFlag(p, SHFLG_Echo, azArg[1]);
    }else{
      eputz("Usage: .echo on|off\n");
      rc = 1;
    }
  }else

  if( c=='d' && n>=3 && cli_strncmp(azArg[0], "dbtotxt", n)==0 ){
    rc = shell_dbtotxt_command(p, nArg, azArg);
  }else

  if( c=='e' && cli_strncmp(azArg[0], "eqp", n)==0 ){
    if( nArg==2 ){
      p->autoEQPtest = 0;
      if( p->autoEQPtrace ){
        if( p->db ) sqlite3_exec(p->db, "PRAGMA vdbe_trace=OFF;", 0, 0, 0);
        p->autoEQPtrace = 0;
      }
      if( cli_strcmp(azArg[1],"full")==0 ){
        p->autoEQP = AUTOEQP_full;
      }else if( cli_strcmp(azArg[1],"trigger")==0 ){
        p->autoEQP = AUTOEQP_trigger;
#ifdef SQLITE_DEBUG
      }else if( cli_strcmp(azArg[1],"test")==0 ){
        p->autoEQP = AUTOEQP_on;
        p->autoEQPtest = 1;
      }else if( cli_strcmp(azArg[1],"trace")==0 ){
        p->autoEQP = AUTOEQP_full;
        p->autoEQPtrace = 1;
        open_db(p, 0);
        sqlite3_exec(p->db, "SELECT name FROM sqlite_schema LIMIT 1", 0, 0, 0);
        sqlite3_exec(p->db, "PRAGMA vdbe_trace=ON;", 0, 0, 0);
#endif
      }else{
        p->autoEQP = (u8)booleanValue(azArg[1]);
      }
    }else{
      eputz("Usage: .eqp off|on|trace|trigger|full\n");
      rc = 1;
    }
  }else

#ifndef SQLITE_SHELL_FIDDLE
  if( c=='e' && cli_strncmp(azArg[0], "exit", n)==0 ){
    if( nArg>1 && (rc = (int)integerValue(azArg[1]))!=0 ) exit(rc);
    rc = 2;
  }else
#endif

  /* The ".explain" command is automatic now.  It is largely pointless.  It
  ** retained purely for backwards compatibility */
  if( c=='e' && cli_strncmp(azArg[0], "explain", n)==0 ){
    int val = 1;
    if( nArg>=2 ){
      if( cli_strcmp(azArg[1],"auto")==0 ){
        val = 99;
      }else{
        val =  booleanValue(azArg[1]);
      }
    }
    if( val==1 && p->mode!=MODE_Explain ){
      p->normalMode = p->mode;
      p->mode = MODE_Explain;
      p->autoExplain = 0;
    }else if( val==0 ){
      if( p->mode==MODE_Explain ) p->mode = p->normalMode;
      p->autoExplain = 0;
    }else if( val==99 ){
      if( p->mode==MODE_Explain ) p->mode = p->normalMode;
      p->autoExplain = 1;
    }
  }else

#ifndef SQLITE_OMIT_VIRTUALTABLE
  if( c=='e' && cli_strncmp(azArg[0], "expert", n)==0 ){
    if( p->bSafeMode ){
      sqlite3_fprintf(stderr,
            "Cannot run experimental commands such as \"%s\" in safe mode\n",
            azArg[0]);
      rc = 1;
    }else{
      open_db(p, 0);
      expertDotCommand(p, azArg, nArg);
    }
  }else
#endif

  if( c=='f' && cli_strncmp(azArg[0], "filectrl", n)==0 ){
    static const struct {
       const char *zCtrlName;   /* Name of a test-control option */
       int ctrlCode;            /* Integer code for that option */
       const char *zUsage;      /* Usage notes */
    } aCtrl[] = {
      { "chunk_size",     SQLITE_FCNTL_CHUNK_SIZE,      "SIZE"           },
      { "data_version",   SQLITE_FCNTL_DATA_VERSION,    ""               },
      { "has_moved",      SQLITE_FCNTL_HAS_MOVED,       ""               },
      { "lock_timeout",   SQLITE_FCNTL_LOCK_TIMEOUT,    "MILLISEC"       },
      { "persist_wal",    SQLITE_FCNTL_PERSIST_WAL,     "[BOOLEAN]"      },
   /* { "pragma",         SQLITE_FCNTL_PRAGMA,          "NAME ARG"       },*/
      { "psow",       SQLITE_FCNTL_POWERSAFE_OVERWRITE, "[BOOLEAN]"      },
      { "reserve_bytes",  SQLITE_FCNTL_RESERVE_BYTES,   "[N]"            },
      { "size_limit",     SQLITE_FCNTL_SIZE_LIMIT,      "[LIMIT]"        },
      { "tempfilename",   SQLITE_FCNTL_TEMPFILENAME,    ""               },
   /* { "win32_av_retry", SQLITE_FCNTL_WIN32_AV_RETRY,  "COUNT DELAY"    },*/
    };
    int filectrl = -1;
    int iCtrl = -1;
    sqlite3_int64 iRes = 0;  /* Integer result to display if rc2==1 */
    int isOk = 0;            /* 0: usage  1: %lld  2: no-result */
    int n2, i;
    const char *zCmd = 0;
    const char *zSchema = 0;

    open_db(p, 0);
    zCmd = nArg>=2 ? azArg[1] : "help";

    if( zCmd[0]=='-'
     && (cli_strcmp(zCmd,"--schema")==0 || cli_strcmp(zCmd,"-schema")==0)
     && nArg>=4
    ){
      zSchema = azArg[2];
      for(i=3; i<nArg; i++) azArg[i-2] = azArg[i];
      nArg -= 2;
      zCmd = azArg[1];
    }

    /* The argument can optionally begin with "-" or "--" */
    if( zCmd[0]=='-' && zCmd[1] ){
      zCmd++;
      if( zCmd[0]=='-' && zCmd[1] ) zCmd++;
    }

    /* --help lists all file-controls */
    if( cli_strcmp(zCmd,"help")==0 ){
      sqlite3_fputs("Available file-controls:\n", p->out);
      for(i=0; i<ArraySize(aCtrl); i++){
        sqlite3_fprintf(p->out,
               "  .filectrl %s %s\n", aCtrl[i].zCtrlName, aCtrl[i].zUsage);
      }
      rc = 1;
      goto meta_command_exit;
    }

    /* convert filectrl text option to value. allow any unique prefix
    ** of the option name, or a numerical value. */
    n2 = strlen30(zCmd);
    for(i=0; i<ArraySize(aCtrl); i++){
      if( cli_strncmp(zCmd, aCtrl[i].zCtrlName, n2)==0 ){
        if( filectrl<0 ){
          filectrl = aCtrl[i].ctrlCode;
          iCtrl = i;
        }else{
          sqlite3_fprintf(stderr,"Error: ambiguous file-control: \"%s\"\n"
                "Use \".filectrl --help\" for help\n", zCmd);
          rc = 1;
          goto meta_command_exit;
        }
      }
    }
    if( filectrl<0 ){
      sqlite3_fprintf(stderr,"Error: unknown file-control: %s\n"
            "Use \".filectrl --help\" for help\n", zCmd);
    }else{
      switch(filectrl){
        case SQLITE_FCNTL_SIZE_LIMIT: {
          if( nArg!=2 && nArg!=3 ) break;
          iRes = nArg==3 ? integerValue(azArg[2]) : -1;
          sqlite3_file_control(p->db, zSchema, SQLITE_FCNTL_SIZE_LIMIT, &iRes);
          isOk = 1;
          break;
        }
        case SQLITE_FCNTL_LOCK_TIMEOUT:
        case SQLITE_FCNTL_CHUNK_SIZE: {
          int x;
          if( nArg!=3 ) break;
          x = (int)integerValue(azArg[2]);
          sqlite3_file_control(p->db, zSchema, filectrl, &x);
          isOk = 2;
          break;
        }
        case SQLITE_FCNTL_PERSIST_WAL:
        case SQLITE_FCNTL_POWERSAFE_OVERWRITE: {
          int x;
          if( nArg!=2 && nArg!=3 ) break;
          x = nArg==3 ? booleanValue(azArg[2]) : -1;
          sqlite3_file_control(p->db, zSchema, filectrl, &x);
          iRes = x;
          isOk = 1;
          break;
        }
        case SQLITE_FCNTL_DATA_VERSION:
        case SQLITE_FCNTL_HAS_MOVED: {
          int x;
          if( nArg!=2 ) break;
          sqlite3_file_control(p->db, zSchema, filectrl, &x);
          iRes = x;
          isOk = 1;
          break;
        }
        case SQLITE_FCNTL_TEMPFILENAME: {
          char *z = 0;
          if( nArg!=2 ) break;
          sqlite3_file_control(p->db, zSchema, filectrl, &z);
          if( z ){
            sqlite3_fprintf(p->out, "%s\n", z);
            sqlite3_free(z);
          }
          isOk = 2;
          break;
        }
        case SQLITE_FCNTL_RESERVE_BYTES: {
          int x;
          if( nArg>=3 ){
            x = atoi(azArg[2]);
            sqlite3_file_control(p->db, zSchema, filectrl, &x);
          }
          x = -1;
          sqlite3_file_control(p->db, zSchema, filectrl, &x);
          sqlite3_fprintf(p->out, "%d\n", x);
          isOk = 2;
          break;
        }
      }
    }
    if( isOk==0 && iCtrl>=0 ){
      sqlite3_fprintf(p->out, "Usage: .filectrl %s %s\n",
                      zCmd, aCtrl[iCtrl].zUsage);
      rc = 1;
    }else if( isOk==1 ){
      char zBuf[100];
      sqlite3_snprintf(sizeof(zBuf), zBuf, "%lld", iRes);
      sqlite3_fprintf(p->out, "%s\n", zBuf);
    }
  }else

  if( c=='f' && cli_strncmp(azArg[0], "fullschema", n)==0 ){
    ShellState data;
    int doStats = 0;
    memcpy(&data, p, sizeof(data));
    data.showHeader = 0;
    data.cMode = data.mode = MODE_Semi;
    if( nArg==2 && optionMatch(azArg[1], "indent") ){
      data.cMode = data.mode = MODE_Pretty;
      nArg = 1;
    }
    if( nArg!=1 ){
      eputz("Usage: .fullschema ?--indent?\n");
      rc = 1;
      goto meta_command_exit;
    }
    open_db(p, 0);
    rc = sqlite3_exec(p->db,
       "SELECT sql FROM"
       "  (SELECT sql sql, type type, tbl_name tbl_name, name name, rowid x"
       "     FROM sqlite_schema UNION ALL"
       "   SELECT sql, type, tbl_name, name, rowid FROM sqlite_temp_schema) "
       "WHERE type!='meta' AND sql NOTNULL AND name NOT LIKE 'sqlite_%' "
       "ORDER BY x",
       callback, &data, 0
    );
    if( rc==SQLITE_OK ){
      sqlite3_stmt *pStmt;
      rc = sqlite3_prepare_v2(p->db,
               "SELECT rowid FROM sqlite_schema"
               " WHERE name GLOB 'sqlite_stat[134]'",
               -1, &pStmt, 0);
      if( rc==SQLITE_OK ){
        doStats = sqlite3_step(pStmt)==SQLITE_ROW;
        sqlite3_finalize(pStmt);
      }
    }
    if( doStats==0 ){
      sqlite3_fputs("/* No STAT tables available */\n", p->out);
    }else{
      sqlite3_fputs("ANALYZE sqlite_schema;\n", p->out);
      data.cMode = data.mode = MODE_Insert;
      data.zDestTable = "sqlite_stat1";
      shell_exec(&data, "SELECT * FROM sqlite_stat1", 0);
      data.zDestTable = "sqlite_stat4";
      shell_exec(&data, "SELECT * FROM sqlite_stat4", 0);
      sqlite3_fputs("ANALYZE sqlite_schema;\n", p->out);
    }
  }else

  if( c=='h' && cli_strncmp(azArg[0], "headers", n)==0 ){
    if( nArg==2 ){
      p->showHeader = booleanValue(azArg[1]);
      p->shellFlgs |= SHFLG_HeaderSet;
    }else{
      eputz("Usage: .headers on|off\n");
      rc = 1;
    }
  }else

  if( c=='h' && cli_strncmp(azArg[0], "help", n)==0 ){
    if( nArg>=2 ){
      n = showHelp(p->out, azArg[1]);
      if( n==0 ){
        sqlite3_fprintf(p->out, "Nothing matches '%s'\n", azArg[1]);
      }
    }else{
      showHelp(p->out, 0);
    }
  }else

#ifndef SQLITE_SHELL_FIDDLE
  if( c=='i' && cli_strncmp(azArg[0], "import", n)==0 ){
    char *zTable = 0;           /* Insert data into this table */
    char *zSchema = 0;          /* Schema of zTable */
    char *zFile = 0;            /* Name of file to extra content from */
    sqlite3_stmt *pStmt = NULL; /* A statement */
    int nCol;                   /* Number of columns in the table */
    i64 nByte;                  /* Number of bytes in an SQL string */
    int i, j;                   /* Loop counters */
    int needCommit;             /* True to COMMIT or ROLLBACK at end */
    int nSep;                   /* Number of bytes in p->colSeparator[] */
    char *zSql = 0;             /* An SQL statement */
    ImportCtx sCtx;             /* Reader context */
    char *(SQLITE_CDECL *xRead)(ImportCtx*); /* Func to read one value */
    int eVerbose = 0;           /* Larger for more console output */
    int nSkip = 0;              /* Initial lines to skip */
    int useOutputMode = 1;      /* Use output mode to determine separators */
    char *zCreate = 0;          /* CREATE TABLE statement text */

    failIfSafeMode(p, "cannot run .import in safe mode");
    memset(&sCtx, 0, sizeof(sCtx));
    if( p->mode==MODE_Ascii ){
      xRead = ascii_read_one_field;
    }else{
      xRead = csv_read_one_field;
    }
    rc = 1;
    for(i=1; i<nArg; i++){
      char *z = azArg[i];
      if( z[0]=='-' && z[1]=='-' ) z++;
      if( z[0]!='-' ){
        if( zFile==0 ){
          zFile = z;
        }else if( zTable==0 ){
          zTable = z;
        }else{
          sqlite3_fprintf(p->out, "ERROR: extra argument: \"%s\". Usage:\n",z);
          showHelp(p->out, "import");
          goto meta_command_exit;
        }
      }else if( cli_strcmp(z,"-v")==0 ){
        eVerbose++;
      }else if( cli_strcmp(z,"-schema")==0 && i<nArg-1 ){
        zSchema = azArg[++i];
      }else if( cli_strcmp(z,"-skip")==0 && i<nArg-1 ){
        nSkip = integerValue(azArg[++i]);
      }else if( cli_strcmp(z,"-ascii")==0 ){
        sCtx.cColSep = SEP_Unit[0];
        sCtx.cRowSep = SEP_Record[0];
        xRead = ascii_read_one_field;
        useOutputMode = 0;
      }else if( cli_strcmp(z,"-csv")==0 ){
        sCtx.cColSep = ',';
        sCtx.cRowSep = '\n';
        xRead = csv_read_one_field;
        useOutputMode = 0;
      }else{
        sqlite3_fprintf(p->out, "ERROR: unknown option: \"%s\".  Usage:\n", z);
        showHelp(p->out, "import");
        goto meta_command_exit;
      }
    }
    if( zTable==0 ){
      sqlite3_fprintf(p->out, "ERROR: missing %s argument. Usage:\n",
            zFile==0 ? "FILE" : "TABLE");
      showHelp(p->out, "import");
      goto meta_command_exit;
    }
    seenInterrupt = 0;
    open_db(p, 0);
    if( useOutputMode ){
      /* If neither the --csv or --ascii options are specified, then set
      ** the column and row separator characters from the output mode. */
      nSep = strlen30(p->colSeparator);
      if( nSep==0 ){
        eputz("Error: non-null column separator required for import\n");
        goto meta_command_exit;
      }
      if( nSep>1 ){
        eputz("Error: multi-character column separators not allowed"
              " for import\n");
        goto meta_command_exit;
      }
      nSep = strlen30(p->rowSeparator);
      if( nSep==0 ){
        eputz("Error: non-null row separator required for import\n");
        goto meta_command_exit;
      }
      if( nSep==2 && p->mode==MODE_Csv
       && cli_strcmp(p->rowSeparator,SEP_CrLf)==0
      ){
        /* When importing CSV (only), if the row separator is set to the
        ** default output row separator, change it to the default input
        ** row separator.  This avoids having to maintain different input
        ** and output row separators. */
        sqlite3_snprintf(sizeof(p->rowSeparator), p->rowSeparator, SEP_Row);
        nSep = strlen30(p->rowSeparator);
      }
      if( nSep>1 ){
        eputz("Error: multi-character row separators not allowed"
              " for import\n");
        goto meta_command_exit;
      }
      sCtx.cColSep = (u8)p->colSeparator[0];
      sCtx.cRowSep = (u8)p->rowSeparator[0];
    }
    sCtx.zFile = zFile;
    sCtx.nLine = 1;
    if( sCtx.zFile[0]=='|' ){
#ifdef SQLITE_OMIT_POPEN
      eputz("Error: pipes are not supported in this OS\n");
      goto meta_command_exit;
#else
      sCtx.in = sqlite3_popen(sCtx.zFile+1, "r");
      sCtx.zFile = "<pipe>";
      sCtx.xCloser = pclose;
#endif
    }else{
      sCtx.in = sqlite3_fopen(sCtx.zFile, "rb");
      sCtx.xCloser = fclose;
    }
    if( sCtx.in==0 ){
      sqlite3_fprintf(stderr,"Error: cannot open \"%s\"\n", zFile);
      goto meta_command_exit;
    }
    if( eVerbose>=2 || (eVerbose>=1 && useOutputMode) ){
      char zSep[2];
      zSep[1] = 0;
      zSep[0] = sCtx.cColSep;
      sqlite3_fputs("Column separator ", p->out);
      output_c_string(p->out, zSep);
      sqlite3_fputs(", row separator ", p->out);
      zSep[0] = sCtx.cRowSep;
      output_c_string(p->out, zSep);
      sqlite3_fputs("\n", p->out);
    }
    sCtx.z = sqlite3_malloc64(120);
    if( sCtx.z==0 ){
      import_cleanup(&sCtx);
      shell_out_of_memory();
    }
    /* Below, resources must be freed before exit. */
    while( (nSkip--)>0 ){
      while( xRead(&sCtx) && sCtx.cTerm==sCtx.cColSep ){}
    }
    import_append_char(&sCtx, 0);    /* To ensure sCtx.z is allocated */
    if( sqlite3_table_column_metadata(p->db, zSchema, zTable,0,0,0,0,0,0) 
     && 0==db_int(p->db, "SELECT count(*) FROM \"%w\".sqlite_schema"
                         " WHERE name=%Q AND type='view'",
                         zSchema ? zSchema : "main", zTable)
    ){
      /* Table does not exist.  Create it. */
      sqlite3 *dbCols = 0;
      char *zRenames = 0;
      char *zColDefs;
      zCreate = sqlite3_mprintf("CREATE TABLE \"%w\".\"%w\"", 
                    zSchema ? zSchema : "main", zTable);
      while( xRead(&sCtx) ){
        zAutoColumn(sCtx.z, &dbCols, 0);
        if( sCtx.cTerm!=sCtx.cColSep ) break;
      }
      zColDefs = zAutoColumn(0, &dbCols, &zRenames);
      if( zRenames!=0 ){
        sqlite3_fprintf((stdin_is_interactive && p->in==stdin)? p->out : stderr,
              "Columns renamed during .import %s due to duplicates:\n"
              "%s\n", sCtx.zFile, zRenames);
        sqlite3_free(zRenames);
      }
      assert(dbCols==0);
      if( zColDefs==0 ){
        sqlite3_fprintf(stderr,"%s: empty file\n", sCtx.zFile);
        import_cleanup(&sCtx);
        rc = 1;
        sqlite3_free(zCreate);
        goto meta_command_exit;
      }
      zCreate = sqlite3_mprintf("%z%z\n", zCreate, zColDefs);
      if( zCreate==0 ){
        import_cleanup(&sCtx);
        shell_out_of_memory();
      }
      if( eVerbose>=1 ){
        sqlite3_fprintf(p->out, "%s\n", zCreate);
      }
      rc = sqlite3_exec(p->db, zCreate, 0, 0, 0);
      if( rc ){
        sqlite3_fprintf(stderr,
             "%s failed:\n%s\n", zCreate, sqlite3_errmsg(p->db));
      }
      sqlite3_free(zCreate);
      zCreate = 0;
      if( rc ){
        import_cleanup(&sCtx);
        rc = 1;
        goto meta_command_exit;
      }
    }
    zSql = sqlite3_mprintf("SELECT count(*) FROM pragma_table_info(%Q,%Q);",
                           zTable, zSchema);
    if( zSql==0 ){
      import_cleanup(&sCtx);
      shell_out_of_memory();
    }
    rc =  sqlite3_prepare_v2(p->db, zSql, -1, &pStmt, 0);
    sqlite3_free(zSql);
    zSql = 0;
    if( rc ){
      if (pStmt) sqlite3_finalize(pStmt);
      shellDatabaseError(p->db);
      import_cleanup(&sCtx);
      rc = 1;
      goto meta_command_exit;
    }
    if( sqlite3_step(pStmt)==SQLITE_ROW ){
      nCol = sqlite3_column_int(pStmt, 0);
    }else{
      nCol = 0;
    }
    sqlite3_finalize(pStmt);
    pStmt = 0;
    if( nCol==0 ) return 0; /* no columns, no error */

    nByte = 64                 /* space for "INSERT INTO", "VALUES(", ")\0" */
          + (zSchema ? strlen(zSchema)*2 + 2: 0)  /* Quoted schema name */
          + strlen(zTable)*2 + 2                  /* Quoted table name */
          + nCol*2;            /* Space for ",?" for each column */
    zSql = sqlite3_malloc64( nByte );
    if( zSql==0 ){
      import_cleanup(&sCtx);
      shell_out_of_memory();
    }
    if( zSchema ){
      sqlite3_snprintf(nByte, zSql, "INSERT INTO \"%w\".\"%w\" VALUES(?", 
                       zSchema, zTable);
    }else{
      sqlite3_snprintf(nByte, zSql, "INSERT INTO \"%w\" VALUES(?", zTable);
    }
    j = strlen30(zSql);
    for(i=1; i<nCol; i++){
      zSql[j++] = ',';
      zSql[j++] = '?';
    }
    zSql[j++] = ')';
    zSql[j] = 0;
    assert( j<nByte );
    if( eVerbose>=2 ){
      sqlite3_fprintf(p->out, "Insert using: %s\n", zSql);
    }
    rc = sqlite3_prepare_v2(p->db, zSql, -1, &pStmt, 0);
    sqlite3_free(zSql);
    zSql = 0;
    if( rc ){
      shellDatabaseError(p->db);
      if (pStmt) sqlite3_finalize(pStmt);
      import_cleanup(&sCtx);
      rc = 1;
      goto meta_command_exit;
    }
    needCommit = sqlite3_get_autocommit(p->db);
    if( needCommit ) sqlite3_exec(p->db, "BEGIN", 0, 0, 0);
    do{
      int startLine = sCtx.nLine;
      for(i=0; i<nCol; i++){
        char *z = xRead(&sCtx);
        /*
        ** Did we reach end-of-file before finding any columns?
        ** If so, stop instead of NULL filling the remaining columns.
        */
        if( z==0 && i==0 ) break;
        /*
        ** Did we reach end-of-file OR end-of-line before finding any
        ** columns in ASCII mode?  If so, stop instead of NULL filling
        ** the remaining columns.
        */
        if( p->mode==MODE_Ascii && (z==0 || z[0]==0) && i==0 ) break;
        /*
        ** For CSV mode, per RFC 4180, accept EOF in lieu of final
        ** record terminator but only for last field of multi-field row.
        ** (If there are too few fields, it's not valid CSV anyway.)
        */
        if( z==0 && (xRead==csv_read_one_field) && i==nCol-1 && i>0 ){
          z = "";
        }
        sqlite3_bind_text(pStmt, i+1, z, -1, SQLITE_TRANSIENT);
        if( i<nCol-1 && sCtx.cTerm!=sCtx.cColSep ){
          sqlite3_fprintf(stderr,"%s:%d: expected %d columns but found %d"
                " - filling the rest with NULL\n",
                sCtx.zFile, startLine, nCol, i+1);
          i += 2;
          while( i<=nCol ){ sqlite3_bind_null(pStmt, i); i++; }
        }
      }
      if( sCtx.cTerm==sCtx.cColSep ){
        do{
          xRead(&sCtx);
          i++;
        }while( sCtx.cTerm==sCtx.cColSep );
        sqlite3_fprintf(stderr,
              "%s:%d: expected %d columns but found %d - extras ignored\n",
              sCtx.zFile, startLine, nCol, i);
      }
      if( i>=nCol ){
        sqlite3_step(pStmt);
        rc = sqlite3_reset(pStmt);
        if( rc!=SQLITE_OK ){
          sqlite3_fprintf(stderr,"%s:%d: INSERT failed: %s\n",
                sCtx.zFile, startLine, sqlite3_errmsg(p->db));
          sCtx.nErr++;
        }else{
          sCtx.nRow++;
        }
      }
    }while( sCtx.cTerm!=EOF );

    import_cleanup(&sCtx);
    sqlite3_finalize(pStmt);
    if( needCommit ) sqlite3_exec(p->db, "COMMIT", 0, 0, 0);
    if( eVerbose>0 ){
      sqlite3_fprintf(p->out,
            "Added %d rows with %d errors using %d lines of input\n",
            sCtx.nRow, sCtx.nErr, sCtx.nLine-1);
    }
  }else
#endif /* !defined(SQLITE_SHELL_FIDDLE) */

#ifndef SQLITE_UNTESTABLE
  if( c=='i' && cli_strncmp(azArg[0], "imposter", n)==0 ){
    char *zSql;
    char *zCollist = 0;
    sqlite3_stmt *pStmt;
    int tnum = 0;
    int isWO = 0;  /* True if making an imposter of a WITHOUT ROWID table */
    int lenPK = 0; /* Length of the PRIMARY KEY string for isWO tables */
    int i;
    if( !ShellHasFlag(p,SHFLG_TestingMode) ){
      sqlite3_fprintf(stderr,".%s unavailable without --unsafe-testing\n",
            "imposter");
      rc = 1;
      goto meta_command_exit;
    }
    if( !(nArg==3 || (nArg==2 && sqlite3_stricmp(azArg[1],"off")==0)) ){
      eputz("Usage: .imposter INDEX IMPOSTER\n"
            "       .imposter off\n");
      /* Also allowed, but not documented:
      **
      **    .imposter TABLE IMPOSTER
      **
      ** where TABLE is a WITHOUT ROWID table.  In that case, the
      ** imposter is another WITHOUT ROWID table with the columns in
      ** storage order. */
      rc = 1;
      goto meta_command_exit;
    }
    open_db(p, 0);
    if( nArg==2 ){
      sqlite3_test_control(SQLITE_TESTCTRL_IMPOSTER, p->db, "main", 0, 1);
      goto meta_command_exit;
    }
    zSql = sqlite3_mprintf(
      "SELECT rootpage, 0 FROM sqlite_schema"
      " WHERE name='%q' AND type='index'"
      "UNION ALL "
      "SELECT rootpage, 1 FROM sqlite_schema"
      " WHERE name='%q' AND type='table'"
      "   AND sql LIKE '%%without%%rowid%%'",
      azArg[1], azArg[1]
    );
    sqlite3_prepare_v2(p->db, zSql, -1, &pStmt, 0);
    sqlite3_free(zSql);
    if( sqlite3_step(pStmt)==SQLITE_ROW ){
      tnum = sqlite3_column_int(pStmt, 0);
      isWO = sqlite3_column_int(pStmt, 1);
    }
    sqlite3_finalize(pStmt);
    zSql = sqlite3_mprintf("PRAGMA index_xinfo='%q'", azArg[1]);
    rc = sqlite3_prepare_v2(p->db, zSql, -1, &pStmt, 0);
    sqlite3_free(zSql);
    i = 0;
    while( rc==SQLITE_OK && sqlite3_step(pStmt)==SQLITE_ROW ){
      char zLabel[20];
      const char *zCol = (const char*)sqlite3_column_text(pStmt,2);
      i++;
      if( zCol==0 ){
        if( sqlite3_column_int(pStmt,1)==-1 ){
          zCol = "_ROWID_";
        }else{
          sqlite3_snprintf(sizeof(zLabel),zLabel,"expr%d",i);
          zCol = zLabel;
        }
      }
      if( isWO && lenPK==0 && sqlite3_column_int(pStmt,5)==0 && zCollist ){
        lenPK = (int)strlen(zCollist);
      }
      if( zCollist==0 ){
        zCollist = sqlite3_mprintf("\"%w\"", zCol);
      }else{
        zCollist = sqlite3_mprintf("%z,\"%w\"", zCollist, zCol);
      }
    }
    sqlite3_finalize(pStmt);
    if( i==0 || tnum==0 ){
      sqlite3_fprintf(stderr,"no such index: \"%s\"\n", azArg[1]);
      rc = 1;
      sqlite3_free(zCollist);
      goto meta_command_exit;
    }
    if( lenPK==0 ) lenPK = 100000;
    zSql = sqlite3_mprintf(
          "CREATE TABLE \"%w\"(%s,PRIMARY KEY(%.*s))WITHOUT ROWID",
          azArg[2], zCollist, lenPK, zCollist);
    sqlite3_free(zCollist);
    rc = sqlite3_test_control(SQLITE_TESTCTRL_IMPOSTER, p->db, "main", 1, tnum);
    if( rc==SQLITE_OK ){
      rc = sqlite3_exec(p->db, zSql, 0, 0, 0);
      sqlite3_test_control(SQLITE_TESTCTRL_IMPOSTER, p->db, "main", 0, 0);
      if( rc ){
        sqlite3_fprintf(stderr,
              "Error in [%s]: %s\n", zSql, sqlite3_errmsg(p->db));
      }else{
        sqlite3_fprintf(stdout, "%s;\n", zSql);
        sqlite3_fprintf(stdout,
              "WARNING: writing to an imposter table will corrupt"
              " the \"%s\" %s!\n", azArg[1], isWO ? "table" : "index");
      }
    }else{
      sqlite3_fprintf(stderr,"SQLITE_TESTCTRL_IMPOSTER returns %d\n", rc);
      rc = 1;
    }
    sqlite3_free(zSql);
  }else
#endif /* !defined(SQLITE_OMIT_TEST_CONTROL) */

  if( c=='i' && cli_strncmp(azArg[0], "intck", n)==0 ){
    i64 iArg = 0;
    if( nArg==2 ){
      iArg = integerValue(azArg[1]);
      if( iArg==0 ) iArg = -1;
    }
    if( (nArg!=1 && nArg!=2) || iArg<0 ){
      sqlite3_fprintf(stderr,"%s","Usage: .intck STEPS_PER_UNLOCK\n");
      rc = 1;
      goto meta_command_exit;
    }
    open_db(p, 0);
    rc = intckDatabaseCmd(p, iArg);
  }else

#ifdef SQLITE_ENABLE_IOTRACE
  if( c=='i' && cli_strncmp(azArg[0], "iotrace", n)==0 ){
    SQLITE_API extern void (SQLITE_CDECL *sqlite3IoTrace)(const char*, ...);
    if( iotrace && iotrace!=stdout ) fclose(iotrace);
    iotrace = 0;
    if( nArg<2 ){
      sqlite3IoTrace = 0;
    }else if( cli_strcmp(azArg[1], "-")==0 ){
      sqlite3IoTrace = iotracePrintf;
      iotrace = stdout;
    }else{
      iotrace = sqlite3_fopen(azArg[1], "w");
      if( iotrace==0 ){
        sqlite3_fprintf(stderr,"Error: cannot open \"%s\"\n", azArg[1]);
        sqlite3IoTrace = 0;
        rc = 1;
      }else{
        sqlite3IoTrace = iotracePrintf;
      }
    }
  }else
#endif

  if( c=='l' && n>=5 && cli_strncmp(azArg[0], "limits", n)==0 ){
    static const struct {
       const char *zLimitName;   /* Name of a limit */
       int limitCode;            /* Integer code for that limit */
    } aLimit[] = {
      { "length",                SQLITE_LIMIT_LENGTH                    },
      { "sql_length",            SQLITE_LIMIT_SQL_LENGTH                },
      { "column",                SQLITE_LIMIT_COLUMN                    },
      { "expr_depth",            SQLITE_LIMIT_EXPR_DEPTH                },
      { "compound_select",       SQLITE_LIMIT_COMPOUND_SELECT           },
      { "vdbe_op",               SQLITE_LIMIT_VDBE_OP                   },
      { "function_arg",          SQLITE_LIMIT_FUNCTION_ARG              },
      { "attached",              SQLITE_LIMIT_ATTACHED                  },
      { "like_pattern_length",   SQLITE_LIMIT_LIKE_PATTERN_LENGTH       },
      { "variable_number",       SQLITE_LIMIT_VARIABLE_NUMBER           },
      { "trigger_depth",         SQLITE_LIMIT_TRIGGER_DEPTH             },
      { "worker_threads",        SQLITE_LIMIT_WORKER_THREADS            },
    };
    int i, n2;
    open_db(p, 0);
    if( nArg==1 ){
      for(i=0; i<ArraySize(aLimit); i++){
        sqlite3_fprintf(stdout, "%20s %d\n", aLimit[i].zLimitName,
              sqlite3_limit(p->db, aLimit[i].limitCode, -1));
      }
    }else if( nArg>3 ){
      eputz("Usage: .limit NAME ?NEW-VALUE?\n");
      rc = 1;
      goto meta_command_exit;
    }else{
      int iLimit = -1;
      n2 = strlen30(azArg[1]);
      for(i=0; i<ArraySize(aLimit); i++){
        if( sqlite3_strnicmp(aLimit[i].zLimitName, azArg[1], n2)==0 ){
          if( iLimit<0 ){
            iLimit = i;
          }else{
            sqlite3_fprintf(stderr,"ambiguous limit: \"%s\"\n", azArg[1]);
            rc = 1;
            goto meta_command_exit;
          }
        }
      }
      if( iLimit<0 ){
        sqlite3_fprintf(stderr,"unknown limit: \"%s\"\n"
              "enter \".limits\" with no arguments for a list.\n",
              azArg[1]);
        rc = 1;
        goto meta_command_exit;
      }
      if( nArg==3 ){
        sqlite3_limit(p->db, aLimit[iLimit].limitCode,
                      (int)integerValue(azArg[2]));
      }
      sqlite3_fprintf(stdout, "%20s %d\n", aLimit[iLimit].zLimitName,
            sqlite3_limit(p->db, aLimit[iLimit].limitCode, -1));
    }
  }else

  if( c=='l' && n>2 && cli_strncmp(azArg[0], "lint", n)==0 ){
    open_db(p, 0);
    lintDotCommand(p, azArg, nArg);
  }else

#if !defined(SQLITE_OMIT_LOAD_EXTENSION) && !defined(SQLITE_SHELL_FIDDLE)
  if( c=='l' && cli_strncmp(azArg[0], "load", n)==0 ){
    const char *zFile, *zProc;
    char *zErrMsg = 0;
    failIfSafeMode(p, "cannot run .load in safe mode");
    if( nArg<2 || azArg[1][0]==0 ){
      /* Must have a non-empty FILE. (Will not load self.) */
      eputz("Usage: .load FILE ?ENTRYPOINT?\n");
      rc = 1;
      goto meta_command_exit;
    }
    zFile = azArg[1];
    zProc = nArg>=3 ? azArg[2] : 0;
    open_db(p, 0);
    rc = sqlite3_load_extension(p->db, zFile, zProc, &zErrMsg);
    if( rc!=SQLITE_OK ){
      shellEmitError(zErrMsg);
      sqlite3_free(zErrMsg);
      rc = 1;
    }
  }else
#endif

  if( c=='l' && cli_strncmp(azArg[0], "log", n)==0 ){
    if( nArg!=2 ){
      eputz("Usage: .log FILENAME\n");
      rc = 1;
    }else{
      const char *zFile = azArg[1];
      if( p->bSafeMode
       && cli_strcmp(zFile,"on")!=0
       && cli_strcmp(zFile,"off")!=0
      ){
        sputz(stdout, "cannot set .log to anything other"
              " than \"on\" or \"off\"\n");
        zFile = "off";
      }
      output_file_close(p->pLog);
      if( cli_strcmp(zFile,"on")==0 ) zFile = "stdout";
      p->pLog = output_file_open(zFile);
    }
  }else

  if( c=='m' && cli_strncmp(azArg[0], "mode", n)==0 ){
    const char *zMode = 0;
    const char *zTabname = 0;
    int i, n2;
    int chng = 0;       /* 0x01:  change to cmopts.  0x02:  Any other change */
    ColModeOpts cmOpts = ColModeOpts_default;
    for(i=1; i<nArg; i++){
      const char *z = azArg[i];
      if( optionMatch(z,"wrap") && i+1<nArg ){
        cmOpts.iWrap = integerValue(azArg[++i]);
        chng |= 1;
      }else if( optionMatch(z,"ww") ){
        cmOpts.bWordWrap = 1;
        chng |= 1;
      }else if( optionMatch(z,"wordwrap") && i+1<nArg ){
        cmOpts.bWordWrap = (u8)booleanValue(azArg[++i]);
        chng |= 1;
      }else if( optionMatch(z,"quote") ){
        cmOpts.bQuote = 1;
        chng |= 1;
      }else if( optionMatch(z,"noquote") ){
        cmOpts.bQuote = 0;
        chng |= 1;
      }else if( optionMatch(z,"escape") && i+1<nArg ){
        /* See similar code at tag-20250224-1 */
        const char *zEsc = azArg[++i];
        int k;
        for(k=0; k<ArraySize(shell_EscModeNames); k++){
          if( sqlite3_stricmp(zEsc,shell_EscModeNames[k])==0 ){
            p->eEscMode = k;
            chng |= 2;
            break;
          }
        }
        if( k>=ArraySize(shell_EscModeNames) ){
          sqlite3_fprintf(stderr, "unknown control character escape mode \"%s\""
                                  " - choices:", zEsc);
          for(k=0; k<ArraySize(shell_EscModeNames); k++){
            sqlite3_fprintf(stderr, " %s", shell_EscModeNames[k]);
          }
          sqlite3_fprintf(stderr, "\n");
          rc = 1;
          goto meta_command_exit;
        }
      }else if( zMode==0 ){
        zMode = z;
        /* Apply defaults for qbox pseudo-mode.  If that
         * overwrites already-set values, user was informed of this.
         */
        chng |= 1;
        if( cli_strcmp(z, "qbox")==0 ){
          ColModeOpts cmo = ColModeOpts_default_qbox;
          zMode = "box";
          cmOpts = cmo;
        }
      }else if( zTabname==0 ){
        zTabname = z;
      }else if( z[0]=='-' ){
        sqlite3_fprintf(stderr,"unknown option: %s\n", z);
        eputz("options:\n"
              "  --escape MODE\n"
              "  --noquote\n"
              "  --quote\n"
              "  --wordwrap on/off\n"
              "  --wrap N\n"
              "  --ww\n");
        rc = 1;
        goto meta_command_exit;
      }else{
        sqlite3_fprintf(stderr,"extra argument: \"%s\"\n", z);
        rc = 1;
        goto meta_command_exit;
      }
    }
    if( !chng ){
      if( p->mode==MODE_Column
       || (p->mode>=MODE_Markdown && p->mode<=MODE_Box)
      ){
        sqlite3_fprintf(p->out,
              "current output mode: %s --wrap %d --wordwrap %s "
              "--%squote --escape %s\n",
              modeDescr[p->mode], p->cmOpts.iWrap,
              p->cmOpts.bWordWrap ? "on" : "off",
              p->cmOpts.bQuote ? "" : "no",
              shell_EscModeNames[p->eEscMode]
        );
      }else{
        sqlite3_fprintf(p->out,
              "current output mode: %s --escape %s\n",
              modeDescr[p->mode],
              shell_EscModeNames[p->eEscMode]
        );
      }
    }
    if( zMode==0 ){
      zMode = modeDescr[p->mode];
      if( (chng&1)==0 ) cmOpts = p->cmOpts;
    }
    n2 = strlen30(zMode);
    if( cli_strncmp(zMode,"lines",n2)==0 ){
      p->mode = MODE_Line;
      sqlite3_snprintf(sizeof(p->rowSeparator), p->rowSeparator, SEP_Row);
    }else if( cli_strncmp(zMode,"columns",n2)==0 ){
      p->mode = MODE_Column;
      if( (p->shellFlgs & SHFLG_HeaderSet)==0 ){
        p->showHeader = 1;
      }
      sqlite3_snprintf(sizeof(p->rowSeparator), p->rowSeparator, SEP_Row);
      p->cmOpts = cmOpts;
    }else if( cli_strncmp(zMode,"list",n2)==0 ){
      p->mode = MODE_List;
      sqlite3_snprintf(sizeof(p->colSeparator), p->colSeparator, SEP_Column);
      sqlite3_snprintf(sizeof(p->rowSeparator), p->rowSeparator, SEP_Row);
    }else if( cli_strncmp(zMode,"html",n2)==0 ){
      p->mode = MODE_Html;
    }else if( cli_strncmp(zMode,"tcl",n2)==0 ){
      p->mode = MODE_Tcl;
      sqlite3_snprintf(sizeof(p->colSeparator), p->colSeparator, SEP_Space);
      sqlite3_snprintf(sizeof(p->rowSeparator), p->rowSeparator, SEP_Row);
    }else if( cli_strncmp(zMode,"csv",n2)==0 ){
      p->mode = MODE_Csv;
      sqlite3_snprintf(sizeof(p->colSeparator), p->colSeparator, SEP_Comma);
      sqlite3_snprintf(sizeof(p->rowSeparator), p->rowSeparator, SEP_CrLf);
    }else if( cli_strncmp(zMode,"tabs",n2)==0 ){
      p->mode = MODE_List;
      sqlite3_snprintf(sizeof(p->colSeparator), p->colSeparator, SEP_Tab);
    }else if( cli_strncmp(zMode,"insert",n2)==0 ){
      p->mode = MODE_Insert;
      set_table_name(p, zTabname ? zTabname : "table");
      if( p->eEscMode==SHELL_ESC_OFF ){
        ShellSetFlag(p, SHFLG_Newlines);
      }else{
        ShellClearFlag(p, SHFLG_Newlines);
      }
    }else if( cli_strncmp(zMode,"quote",n2)==0 ){
      p->mode = MODE_Quote;
      sqlite3_snprintf(sizeof(p->colSeparator), p->colSeparator, SEP_Comma);
      sqlite3_snprintf(sizeof(p->rowSeparator), p->rowSeparator, SEP_Row);
    }else if( cli_strncmp(zMode,"ascii",n2)==0 ){
      p->mode = MODE_Ascii;
      sqlite3_snprintf(sizeof(p->colSeparator), p->colSeparator, SEP_Unit);
      sqlite3_snprintf(sizeof(p->rowSeparator), p->rowSeparator, SEP_Record);
    }else if( cli_strncmp(zMode,"markdown",n2)==0 ){
      p->mode = MODE_Markdown;
      p->cmOpts = cmOpts;
    }else if( cli_strncmp(zMode,"table",n2)==0 ){
      p->mode = MODE_Table;
      p->cmOpts = cmOpts;
    }else if( cli_strncmp(zMode,"box",n2)==0 ){
      p->mode = MODE_Box;
      p->cmOpts = cmOpts;
    }else if( cli_strncmp(zMode,"count",n2)==0 ){
      p->mode = MODE_Count;
    }else if( cli_strncmp(zMode,"off",n2)==0 ){
      p->mode = MODE_Off;
    }else if( cli_strncmp(zMode,"json",n2)==0 ){
      p->mode = MODE_Json;
    }else{
      eputz("Error: mode should be one of: "
            "ascii box column csv html insert json line list markdown "
            "qbox quote table tabs tcl\n");
      rc = 1;
    }
    p->cMode = p->mode;
  }else

#ifndef SQLITE_SHELL_FIDDLE
  if( c=='n' && cli_strcmp(azArg[0], "nonce")==0 ){
    if( nArg!=2 ){
      eputz("Usage: .nonce NONCE\n");
      rc = 1;
    }else if( p->zNonce==0 || cli_strcmp(azArg[1],p->zNonce)!=0 ){
      sqlite3_fprintf(stderr,"line %d: incorrect nonce: \"%s\"\n",
            p->lineno, azArg[1]);
      exit(1);
    }else{
      p->bSafeMode = 0;
      return 0;  /* Return immediately to bypass the safe mode reset
                 ** at the end of this procedure */
    }
  }else
#endif /* !defined(SQLITE_SHELL_FIDDLE) */

  if( c=='n' && cli_strncmp(azArg[0], "nullvalue", n)==0 ){
    if( nArg==2 ){
      sqlite3_snprintf(sizeof(p->nullValue), p->nullValue,
                       "%.*s", (int)ArraySize(p->nullValue)-1, azArg[1]);
    }else{
      eputz("Usage: .nullvalue STRING\n");
      rc = 1;
    }
  }else

  if( c=='o' && cli_strncmp(azArg[0], "open", n)==0 && n>=2 ){
    const char *zFN = 0;     /* Pointer to constant filename */
    char *zNewFilename = 0;  /* Name of the database file to open */
    int iName = 1;           /* Index in azArg[] of the filename */
    int newFlag = 0;         /* True to delete file before opening */
    int openMode = SHELL_OPEN_UNSPEC;

    /* Check for command-line arguments */
    for(iName=1; iName<nArg; iName++){
      const char *z = azArg[iName];
#ifndef SQLITE_SHELL_FIDDLE
      if( optionMatch(z,"new") ){
        newFlag = 1;
#ifdef SQLITE_HAVE_ZLIB
      }else if( optionMatch(z, "zip") ){
        openMode = SHELL_OPEN_ZIPFILE;
#endif
      }else if( optionMatch(z, "append") ){
        openMode = SHELL_OPEN_APPENDVFS;
      }else if( optionMatch(z, "readonly") ){
        openMode = SHELL_OPEN_READONLY;
      }else if( optionMatch(z, "nofollow") ){
        p->openFlags |= SQLITE_OPEN_NOFOLLOW;
#ifndef SQLITE_OMIT_DESERIALIZE
      }else if( optionMatch(z, "deserialize") ){
        openMode = SHELL_OPEN_DESERIALIZE;
      }else if( optionMatch(z, "hexdb") ){
        openMode = SHELL_OPEN_HEXDB;
      }else if( optionMatch(z, "maxsize") && iName+1<nArg ){
        p->szMax = integerValue(azArg[++iName]);
#endif /* SQLITE_OMIT_DESERIALIZE */
      }else
#endif /* !SQLITE_SHELL_FIDDLE */
      if( z[0]=='-' ){
        sqlite3_fprintf(stderr,"unknown option: %s\n", z);
        rc = 1;
        goto meta_command_exit;
      }else if( zFN ){
        sqlite3_fprintf(stderr,"extra argument: \"%s\"\n", z);
        rc = 1;
        goto meta_command_exit;
      }else{
        zFN = z;
      }
    }

    /* Close the existing database */
    session_close_all(p, -1);
    close_db(p->db);
    p->db = 0;
    p->pAuxDb->zDbFilename = 0;
    sqlite3_free(p->pAuxDb->zFreeOnClose);
    p->pAuxDb->zFreeOnClose = 0;
    p->openMode = openMode;
    p->openFlags = 0;
    p->szMax = 0;

    /* If a filename is specified, try to open it first */
    if( zFN || p->openMode==SHELL_OPEN_HEXDB ){
      if( newFlag && zFN && !p->bSafeMode ) shellDeleteFile(zFN);
#ifndef SQLITE_SHELL_FIDDLE
      if( p->bSafeMode
       && p->openMode!=SHELL_OPEN_HEXDB
       && zFN
       && cli_strcmp(zFN,":memory:")!=0
      ){
        failIfSafeMode(p, "cannot open disk-based database files in safe mode");
      }
#else
      /* WASM mode has its own sandboxed pseudo-filesystem. */
#endif
      if( zFN ){
        zNewFilename = sqlite3_mprintf("%s", zFN);
        shell_check_oom(zNewFilename);
      }else{
        zNewFilename = 0;
      }
      p->pAuxDb->zDbFilename = zNewFilename;
      open_db(p, OPEN_DB_KEEPALIVE);
      if( p->db==0 ){
        sqlite3_fprintf(stderr,"Error: cannot open '%s'\n", zNewFilename);
        sqlite3_free(zNewFilename);
      }else{
        p->pAuxDb->zFreeOnClose = zNewFilename;
      }
    }
    if( p->db==0 ){
      /* As a fall-back open a TEMP database */
      p->pAuxDb->zDbFilename = 0;
      open_db(p, 0);
    }
  }else

#ifndef SQLITE_SHELL_FIDDLE
  if( (c=='o'
        && (cli_strncmp(azArg[0], "output", n)==0
            || cli_strncmp(azArg[0], "once", n)==0))
   || (c=='e' && n==5 && cli_strcmp(azArg[0],"excel")==0)
   || (c=='w' && n==3 && cli_strcmp(azArg[0],"www")==0)
  ){
    char *zFile = 0;
    int i;
    int eMode = 0;          /* 0: .outout/.once, 'x'=.excel, 'w'=.www */
    int bOnce = 0;          /* 0: .output, 1: .once, 2: .excel/.www */
    int bPlain = 0;         /* --plain option */
    static const char *zBomUtf8 = "\357\273\277";
    const char *zBom = 0;

    failIfSafeMode(p, "cannot run .%s in safe mode", azArg[0]);
    if( c=='e' ){
      eMode = 'x';
      bOnce = 2;
    }else if( c=='w' ){
      eMode = 'w';
      bOnce = 2;
    }else if( cli_strncmp(azArg[0],"once",n)==0 ){
      bOnce = 1;
    }
    for(i=1; i<nArg; i++){
      char *z = azArg[i];
      if( z[0]=='-' ){
        if( z[1]=='-' ) z++;
        if( cli_strcmp(z,"-bom")==0 ){
          zBom = zBomUtf8;
        }else if( cli_strcmp(z,"-plain")==0 ){
          bPlain = 1;
        }else if( c=='o' && cli_strcmp(z,"-x")==0 ){
          eMode = 'x';  /* spreadsheet */
        }else if( c=='o' && cli_strcmp(z,"-e")==0 ){
          eMode = 'e';  /* text editor */
        }else if( c=='o' && cli_strcmp(z,"-w")==0 ){
          eMode = 'w';  /* Web browser */
        }else{
          sqlite3_fprintf(p->out,
                          "ERROR: unknown option: \"%s\". Usage:\n", azArg[i]);
          showHelp(p->out, azArg[0]);
          rc = 1;
          goto meta_command_exit;
        }
      }else if( zFile==0 && eMode==0 ){
        if( cli_strcmp(z, "off")==0 ){
#ifdef _WIN32
          zFile = sqlite3_mprintf("nul");
#else
          zFile = sqlite3_mprintf("/dev/null");
#endif
        }else{
          zFile = sqlite3_mprintf("%s", z);
        }
        if( zFile && zFile[0]=='|' ){
          while( i+1<nArg ) zFile = sqlite3_mprintf("%z %s", zFile, azArg[++i]);
          break;
        }
      }else{
        sqlite3_fprintf(p->out,
            "ERROR: extra parameter: \"%s\".  Usage:\n", azArg[i]);
        showHelp(p->out, azArg[0]);
        rc = 1;
        sqlite3_free(zFile);
        goto meta_command_exit;
      }
    }
    if( zFile==0 ){
      zFile = sqlite3_mprintf("stdout");
    }
    shell_check_oom(zFile);
    if( bOnce ){
      p->outCount = 2;
    }else{
      p->outCount = 0;
    }
    output_reset(p);
#ifndef SQLITE_NOHAVE_SYSTEM
    if( eMode=='e' || eMode=='x' || eMode=='w' ){
      p->doXdgOpen = 1;
      outputModePush(p);
      if( eMode=='x' ){
        /* spreadsheet mode.  Output as CSV. */
        newTempFile(p, "csv");
        ShellClearFlag(p, SHFLG_Echo);
        p->mode = MODE_Csv;
        sqlite3_snprintf(sizeof(p->colSeparator), p->colSeparator, SEP_Comma);
        sqlite3_snprintf(sizeof(p->rowSeparator), p->rowSeparator, SEP_CrLf);
#ifdef _WIN32
        zBom = zBomUtf8;  /* Always include the BOM on Windows, as Excel does
                          ** not work without it. */
#endif
      }else if( eMode=='w' ){
        /* web-browser mode. */
        newTempFile(p, "html");
        if( !bPlain ) p->mode = MODE_Www;
      }else{
        /* text editor mode */
        newTempFile(p, "txt");
      }
      sqlite3_free(zFile);
      zFile = sqlite3_mprintf("%s", p->zTempFile);
    }
#endif /* SQLITE_NOHAVE_SYSTEM */
    shell_check_oom(zFile);
    if( zFile[0]=='|' ){
#ifdef SQLITE_OMIT_POPEN
      eputz("Error: pipes are not supported in this OS\n");
      rc = 1;
      output_redir(p, stdout);
#else
      FILE *pfPipe = sqlite3_popen(zFile + 1, "w");
      if( pfPipe==0 ){
        assert( stderr!=NULL );
        sqlite3_fprintf(stderr,"Error: cannot open pipe \"%s\"\n", zFile + 1);
        rc = 1;
      }else{
        output_redir(p, pfPipe);
        if( zBom ) sqlite3_fputs(zBom, pfPipe);
        sqlite3_snprintf(sizeof(p->outfile), p->outfile, "%s", zFile);
      }
#endif
    }else{
      FILE *pfFile = output_file_open(zFile);
      if( pfFile==0 ){
        if( cli_strcmp(zFile,"off")!=0 ){
         assert( stderr!=NULL );
         sqlite3_fprintf(stderr,"Error: cannot write to \"%s\"\n", zFile);
        }
        rc = 1;
      } else {
        output_redir(p, pfFile);
        if( zBom ) sqlite3_fputs(zBom, pfFile);
        if( bPlain && eMode=='w' ){
          sqlite3_fputs(
            "<!DOCTYPE html>\n<BODY>\n<PLAINTEXT>\n",
            pfFile
          );
        }
        sqlite3_snprintf(sizeof(p->outfile), p->outfile, "%s", zFile);
      }
    }
    sqlite3_free(zFile);
  }else
#endif /* !defined(SQLITE_SHELL_FIDDLE) */

  if( c=='p' && n>=3 && cli_strncmp(azArg[0], "parameter", n)==0 ){
    open_db(p,0);
    if( nArg<=1 ) goto parameter_syntax_error;

    /* .parameter clear
    ** Clear all bind parameters by dropping the TEMP table that holds them.
    */
    if( nArg==2 && cli_strcmp(azArg[1],"clear")==0 ){
      sqlite3_exec(p->db, "DROP TABLE IF EXISTS temp.sqlite_parameters;",
                   0, 0, 0);
    }else

    /* .parameter list
    ** List all bind parameters.
    */
    if( nArg==2 && cli_strcmp(azArg[1],"list")==0 ){
      sqlite3_stmt *pStmt = 0;
      int rx;
      int len = 0;
      rx = sqlite3_prepare_v2(p->db,
             "SELECT max(length(key)) "
             "FROM temp.sqlite_parameters;", -1, &pStmt, 0);
      if( rx==SQLITE_OK && sqlite3_step(pStmt)==SQLITE_ROW ){
        len = sqlite3_column_int(pStmt, 0);
        if( len>40 ) len = 40;
      }
      sqlite3_finalize(pStmt);
      pStmt = 0;
      if( len ){
        rx = sqlite3_prepare_v2(p->db,
             "SELECT key, quote(value) "
             "FROM temp.sqlite_parameters;", -1, &pStmt, 0);
        while( rx==SQLITE_OK && sqlite3_step(pStmt)==SQLITE_ROW ){
          sqlite3_fprintf(p->out,
                "%-*s %s\n", len, sqlite3_column_text(pStmt,0),
                sqlite3_column_text(pStmt,1));
        }
        sqlite3_finalize(pStmt);
      }
    }else

    /* .parameter init
    ** Make sure the TEMP table used to hold bind parameters exists.
    ** Create it if necessary.
    */
    if( nArg==2 && cli_strcmp(azArg[1],"init")==0 ){
      bind_table_init(p);
    }else

    /* .parameter set NAME VALUE
    ** Set or reset a bind parameter.  NAME should be the full parameter
    ** name exactly as it appears in the query.  (ex: $abc, @def).  The
    ** VALUE can be in either SQL literal notation, or if not it will be
    ** understood to be a text string.
    */
    if( nArg==4 && cli_strcmp(azArg[1],"set")==0 ){
      int rx;
      char *zSql;
      sqlite3_stmt *pStmt;
      const char *zKey = azArg[2];
      const char *zValue = azArg[3];
      bind_table_init(p);
      zSql = sqlite3_mprintf(
                  "REPLACE INTO temp.sqlite_parameters(key,value)"
                  "VALUES(%Q,%s);", zKey, zValue);
      shell_check_oom(zSql);
      pStmt = 0;
      rx = sqlite3_prepare_v2(p->db, zSql, -1, &pStmt, 0);
      sqlite3_free(zSql);
      if( rx!=SQLITE_OK ){
        sqlite3_finalize(pStmt);
        pStmt = 0;
        zSql = sqlite3_mprintf(
                   "REPLACE INTO temp.sqlite_parameters(key,value)"
                   "VALUES(%Q,%Q);", zKey, zValue);
        shell_check_oom(zSql);
        rx = sqlite3_prepare_v2(p->db, zSql, -1, &pStmt, 0);
        sqlite3_free(zSql);
        if( rx!=SQLITE_OK ){
          sqlite3_fprintf(p->out, "Error: %s\n", sqlite3_errmsg(p->db));
          sqlite3_finalize(pStmt);
          pStmt = 0;
          rc = 1;
        }
      }
      bind_prepared_stmt(p, pStmt);
      sqlite3_step(pStmt);
      sqlite3_finalize(pStmt);
    }else

    /* .parameter unset NAME
    ** Remove the NAME binding from the parameter binding table, if it
    ** exists.
    */
    if( nArg==3 && cli_strcmp(azArg[1],"unset")==0 ){
      char *zSql = sqlite3_mprintf(
          "DELETE FROM temp.sqlite_parameters WHERE key=%Q", azArg[2]);
      shell_check_oom(zSql);
      sqlite3_exec(p->db, zSql, 0, 0, 0);
      sqlite3_free(zSql);
    }else
    /* If no command name matches, show a syntax error */
    parameter_syntax_error:
    showHelp(p->out, "parameter");
  }else

  if( c=='p' && n>=3 && cli_strncmp(azArg[0], "print", n)==0 ){
    int i;
    for(i=1; i<nArg; i++){
      if( i>1 ) sqlite3_fputs(" ", p->out);
      sqlite3_fputs(azArg[i], p->out);
    }
    sqlite3_fputs("\n", p->out);
  }else

#ifndef SQLITE_OMIT_PROGRESS_CALLBACK
  if( c=='p' && n>=3 && cli_strncmp(azArg[0], "progress", n)==0 ){
    int i;
    int nn = 0;
    p->flgProgress = 0;
    p->mxProgress = 0;
    p->nProgress = 0;
    for(i=1; i<nArg; i++){
      const char *z = azArg[i];
      if( z[0]=='-' ){
        z++;
        if( z[0]=='-' ) z++;
        if( cli_strcmp(z,"quiet")==0 || cli_strcmp(z,"q")==0 ){
          p->flgProgress |= SHELL_PROGRESS_QUIET;
          continue;
        }
        if( cli_strcmp(z,"reset")==0 ){
          p->flgProgress |= SHELL_PROGRESS_RESET;
          continue;
        }
        if( cli_strcmp(z,"once")==0 ){
          p->flgProgress |= SHELL_PROGRESS_ONCE;
          continue;
        }
        if( cli_strcmp(z,"limit")==0 ){
          if( i+1>=nArg ){
            eputz("Error: missing argument on --limit\n");
            rc = 1;
            goto meta_command_exit;
          }else{
            p->mxProgress = (int)integerValue(azArg[++i]);
          }
          continue;
        }
        sqlite3_fprintf(stderr,"Error: unknown option: \"%s\"\n", azArg[i]);
        rc = 1;
        goto meta_command_exit;
      }else{
        nn = (int)integerValue(z);
      }
    }
    open_db(p, 0);
    sqlite3_progress_handler(p->db, nn, progress_handler, p);
  }else
#endif /* SQLITE_OMIT_PROGRESS_CALLBACK */

  if( c=='p' && cli_strncmp(azArg[0], "prompt", n)==0 ){
    if( nArg >= 2) {
      shell_strncpy(mainPrompt,azArg[1],(int)ArraySize(mainPrompt)-1);
    }
    if( nArg >= 3) {
      shell_strncpy(continuePrompt,azArg[2],(int)ArraySize(continuePrompt)-1);
    }
  }else

#ifndef SQLITE_SHELL_FIDDLE
  if( c=='q' && cli_strncmp(azArg[0], "quit", n)==0 ){
    rc = 2;
  }else
#endif

#ifndef SQLITE_SHELL_FIDDLE
  if( c=='r' && n>=3 && cli_strncmp(azArg[0], "read", n)==0 ){
    FILE *inSaved = p->in;
    int savedLineno = p->lineno;
    failIfSafeMode(p, "cannot run .read in safe mode");
    if( nArg!=2 ){
      eputz("Usage: .read FILE\n");
      rc = 1;
      goto meta_command_exit;
    }
    if( azArg[1][0]=='|' ){
#ifdef SQLITE_OMIT_POPEN
      eputz("Error: pipes are not supported in this OS\n");
      rc = 1;
#else
      p->in = sqlite3_popen(azArg[1]+1, "r");
      if( p->in==0 ){
        sqlite3_fprintf(stderr,"Error: cannot open \"%s\"\n", azArg[1]);
        rc = 1;
      }else{
        rc = process_input(p);
        pclose(p->in);
      }
#endif
    }else if( (p->in = openChrSource(azArg[1]))==0 ){
      sqlite3_fprintf(stderr,"Error: cannot open \"%s\"\n", azArg[1]);
      rc = 1;
    }else{
      rc = process_input(p);
      fclose(p->in);
    }
    p->in = inSaved;
    p->lineno = savedLineno;
  }else
#endif /* !defined(SQLITE_SHELL_FIDDLE) */

#ifndef SQLITE_SHELL_FIDDLE
  if( c=='r' && n>=3 && cli_strncmp(azArg[0], "restore", n)==0 ){
    const char *zSrcFile;
    const char *zDb;
    sqlite3 *pSrc;
    sqlite3_backup *pBackup;
    int nTimeout = 0;

    failIfSafeMode(p, "cannot run .restore in safe mode");
    if( nArg==2 ){
      zSrcFile = azArg[1];
      zDb = "main";
    }else if( nArg==3 ){
      zSrcFile = azArg[2];
      zDb = azArg[1];
    }else{
      eputz("Usage: .restore ?DB? FILE\n");
      rc = 1;
      goto meta_command_exit;
    }
    rc = sqlite3_open(zSrcFile, &pSrc);
    if( rc!=SQLITE_OK ){
      sqlite3_fprintf(stderr,"Error: cannot open \"%s\"\n", zSrcFile);
      close_db(pSrc);
      return 1;
    }
    open_db(p, 0);
    pBackup = sqlite3_backup_init(p->db, zDb, pSrc, "main");
    if( pBackup==0 ){
      shellDatabaseError(p->db);
      close_db(pSrc);
      return 1;
    }
    while( (rc = sqlite3_backup_step(pBackup,100))==SQLITE_OK
          || rc==SQLITE_BUSY  ){
      if( rc==SQLITE_BUSY ){
        if( nTimeout++ >= 3 ) break;
        sqlite3_sleep(100);
      }
    }
    sqlite3_backup_finish(pBackup);
    if( rc==SQLITE_DONE ){
      rc = 0;
    }else if( rc==SQLITE_BUSY || rc==SQLITE_LOCKED ){
      eputz("Error: source database is busy\n");
      rc = 1;
    }else{
      shellDatabaseError(p->db);
      rc = 1;
    }
    close_db(pSrc);
  }else
#endif /* !defined(SQLITE_SHELL_FIDDLE) */

  if( c=='s' &&
     (cli_strncmp(azArg[0], "scanstats",  n)==0 ||
      cli_strncmp(azArg[0], "scanstatus", n)==0)
  ){
    if( nArg==2 ){
      if( cli_strcmp(azArg[1], "vm")==0 ){
        p->scanstatsOn = 3;
      }else
      if( cli_strcmp(azArg[1], "est")==0 ){
        p->scanstatsOn = 2;
      }else{
        p->scanstatsOn = (u8)booleanValue(azArg[1]);
      }
      open_db(p, 0);
      sqlite3_db_config(
          p->db, SQLITE_DBCONFIG_STMT_SCANSTATUS, p->scanstatsOn, (int*)0
      );
#if !defined(SQLITE_ENABLE_STMT_SCANSTATUS)
      eputz("Warning: .scanstats not available in this build.\n");
#elif !defined(SQLITE_ENABLE_BYTECODE_VTAB)
      if( p->scanstatsOn==3 ){
        eputz("Warning: \".scanstats vm\" not available in this build.\n");
      }
#endif
    }else{
      eputz("Usage: .scanstats on|off|est\n");
      rc = 1;
    }
  }else

  if( c=='s' && cli_strncmp(azArg[0], "schema", n)==0 ){
    ShellText sSelect;
    ShellState data;
    char *zErrMsg = 0;
    const char *zDiv = "(";
    const char *zName = 0;
    int iSchema = 0;
    int bDebug = 0;
    int bNoSystemTabs = 0;
    int ii;

    open_db(p, 0);
    memcpy(&data, p, sizeof(data));
    data.showHeader = 0;
    data.cMode = data.mode = MODE_Semi;
    initText(&sSelect);
    for(ii=1; ii<nArg; ii++){
      if( optionMatch(azArg[ii],"indent") ){
        data.cMode = data.mode = MODE_Pretty;
      }else if( optionMatch(azArg[ii],"debug") ){
        bDebug = 1;
      }else if( optionMatch(azArg[ii],"nosys") ){
        bNoSystemTabs = 1;
      }else if( azArg[ii][0]=='-' ){
        sqlite3_fprintf(stderr,"Unknown option: \"%s\"\n", azArg[ii]);
        rc = 1;
        goto meta_command_exit;
      }else if( zName==0 ){
        zName = azArg[ii];
      }else{
        eputz("Usage: .schema ?--indent? ?--nosys? ?LIKE-PATTERN?\n");
        rc = 1;
        goto meta_command_exit;
      }
    }
    if( zName!=0 ){
      int isSchema = sqlite3_strlike(zName, "sqlite_master", '\\')==0
                  || sqlite3_strlike(zName, "sqlite_schema", '\\')==0
                  || sqlite3_strlike(zName,"sqlite_temp_master", '\\')==0
                  || sqlite3_strlike(zName,"sqlite_temp_schema", '\\')==0;
      if( isSchema ){
        char *new_argv[2], *new_colv[2];
        new_argv[0] = sqlite3_mprintf(
                      "CREATE TABLE %s (\n"
                      "  type text,\n"
                      "  name text,\n"
                      "  tbl_name text,\n"
                      "  rootpage integer,\n"
                      "  sql text\n"
                      ")", zName);
        shell_check_oom(new_argv[0]);
        new_argv[1] = 0;
        new_colv[0] = "sql";
        new_colv[1] = 0;
        callback(&data, 1, new_argv, new_colv);
        sqlite3_free(new_argv[0]);
      }
    }
    if( zDiv ){
      sqlite3_stmt *pStmt = 0;
      rc = sqlite3_prepare_v2(p->db, "SELECT name FROM pragma_database_list",
                              -1, &pStmt, 0);
      if( rc ){
        shellDatabaseError(p->db);
        sqlite3_finalize(pStmt);
        rc = 1;
        goto meta_command_exit;
      }
      appendText(&sSelect, "SELECT sql FROM", 0);
      iSchema = 0;
      while( sqlite3_step(pStmt)==SQLITE_ROW ){
        const char *zDb = (const char*)sqlite3_column_text(pStmt, 0);
        char zScNum[30];
        sqlite3_snprintf(sizeof(zScNum), zScNum, "%d", ++iSchema);
        appendText(&sSelect, zDiv, 0);
        zDiv = " UNION ALL ";
        appendText(&sSelect, "SELECT shell_add_schema(sql,", 0);
        if( sqlite3_stricmp(zDb, "main")!=0 ){
          appendText(&sSelect, zDb, '\'');
        }else{
          appendText(&sSelect, "NULL", 0);
        }
        appendText(&sSelect, ",name) AS sql, type, tbl_name, name, rowid,", 0);
        appendText(&sSelect, zScNum, 0);
        appendText(&sSelect, " AS snum, ", 0);
        appendText(&sSelect, zDb, '\'');
        appendText(&sSelect, " AS sname FROM ", 0);
        appendText(&sSelect, zDb, quoteChar(zDb));
        appendText(&sSelect, ".sqlite_schema", 0);
      }
      sqlite3_finalize(pStmt);
#ifndef SQLITE_OMIT_INTROSPECTION_PRAGMAS
      if( zName ){
        appendText(&sSelect,
           " UNION ALL SELECT shell_module_schema(name),"
           " 'table', name, name, name, 9e+99, 'main' FROM pragma_module_list",
        0);
      }
#endif
      appendText(&sSelect, ") WHERE ", 0);
      if( zName ){
        char *zQarg = sqlite3_mprintf("%Q", zName);
        int bGlob;
        shell_check_oom(zQarg);
        bGlob = strchr(zName, '*') != 0 || strchr(zName, '?') != 0 ||
                strchr(zName, '[') != 0;
        if( strchr(zName, '.') ){
          appendText(&sSelect, "lower(printf('%s.%s',sname,tbl_name))", 0);
        }else{
          appendText(&sSelect, "lower(tbl_name)", 0);
        }
        appendText(&sSelect, bGlob ? " GLOB " : " LIKE ", 0);
        appendText(&sSelect, zQarg, 0);
        if( !bGlob ){
          appendText(&sSelect, " ESCAPE '\\' ", 0);
        }
        appendText(&sSelect, " AND ", 0);
        sqlite3_free(zQarg);
      }
      if( bNoSystemTabs ){
        appendText(&sSelect, "name NOT LIKE 'sqlite_%%' AND ", 0);
      }
      appendText(&sSelect, "sql IS NOT NULL"
                           " ORDER BY snum, rowid", 0);
      if( bDebug ){
        sqlite3_fprintf(p->out, "SQL: %s;\n", sSelect.z);
      }else{
        rc = sqlite3_exec(p->db, sSelect.z, callback, &data, &zErrMsg);
      }
      freeText(&sSelect);
    }
    if( zErrMsg ){
      shellEmitError(zErrMsg);
      sqlite3_free(zErrMsg);
      rc = 1;
    }else if( rc != SQLITE_OK ){
      eputz("Error: querying schema information\n");
      rc = 1;
    }else{
      rc = 0;
    }
  }else

  if( (c=='s' && n==11 && cli_strncmp(azArg[0], "selecttrace", n)==0)
   || (c=='t' && n==9  && cli_strncmp(azArg[0], "treetrace", n)==0)
  ){
    unsigned int x = nArg>=2? (unsigned int)integerValue(azArg[1]) : 0xffffffff;
    sqlite3_test_control(SQLITE_TESTCTRL_TRACEFLAGS, 1, &x);
  }else

#if defined(SQLITE_ENABLE_SESSION)
  if( c=='s' && cli_strncmp(azArg[0],"session",n)==0 && n>=3 ){
    struct AuxDb *pAuxDb = p->pAuxDb;
    OpenSession *pSession = &pAuxDb->aSession[0];
    char **azCmd = &azArg[1];
    int iSes = 0;
    int nCmd = nArg - 1;
    int i;
    if( nArg<=1 ) goto session_syntax_error;
    open_db(p, 0);
    if( nArg>=3 ){
      for(iSes=0; iSes<pAuxDb->nSession; iSes++){
        if( cli_strcmp(pAuxDb->aSession[iSes].zName, azArg[1])==0 ) break;
      }
      if( iSes<pAuxDb->nSession ){
        pSession = &pAuxDb->aSession[iSes];
        azCmd++;
        nCmd--;
      }else{
        pSession = &pAuxDb->aSession[0];
        iSes = 0;
      }
    }

    /* .session attach TABLE
    ** Invoke the sqlite3session_attach() interface to attach a particular
    ** table so that it is never filtered.
    */
    if( cli_strcmp(azCmd[0],"attach")==0 ){
      if( nCmd!=2 ) goto session_syntax_error;
      if( pSession->p==0 ){
        session_not_open:
        eputz("ERROR: No sessions are open\n");
      }else{
        rc = sqlite3session_attach(pSession->p, azCmd[1]);
        if( rc ){
          sqlite3_fprintf(stderr,
               "ERROR: sqlite3session_attach() returns %d\n",rc);
          rc = 0;
        }
      }
    }else

    /* .session changeset FILE
    ** .session patchset FILE
    ** Write a changeset or patchset into a file.  The file is overwritten.
    */
    if( cli_strcmp(azCmd[0],"changeset")==0
     || cli_strcmp(azCmd[0],"patchset")==0
    ){
      FILE *out = 0;
      failIfSafeMode(p, "cannot run \".session %s\" in safe mode", azCmd[0]);
      if( nCmd!=2 ) goto session_syntax_error;
      if( pSession->p==0 ) goto session_not_open;
      out = sqlite3_fopen(azCmd[1], "wb");
      if( out==0 ){
        sqlite3_fprintf(stderr,"ERROR: cannot open \"%s\" for writing\n",
              azCmd[1]);
      }else{
        int szChng;
        void *pChng;
        if( azCmd[0][0]=='c' ){
          rc = sqlite3session_changeset(pSession->p, &szChng, &pChng);
        }else{
          rc = sqlite3session_patchset(pSession->p, &szChng, &pChng);
        }
        if( rc ){
          sqlite3_fprintf(stdout, "Error: error code %d\n", rc);
          rc = 0;
        }
        if( pChng
          && fwrite(pChng, szChng, 1, out)!=1 ){
          sqlite3_fprintf(stderr,
              "ERROR: Failed to write entire %d-byte output\n", szChng);
        }
        sqlite3_free(pChng);
        fclose(out);
      }
    }else

    /* .session close
    ** Close the identified session
    */
    if( cli_strcmp(azCmd[0], "close")==0 ){
      if( nCmd!=1 ) goto session_syntax_error;
      if( pAuxDb->nSession ){
        session_close(pSession);
        pAuxDb->aSession[iSes] = pAuxDb->aSession[--pAuxDb->nSession];
      }
    }else

    /* .session enable ?BOOLEAN?
    ** Query or set the enable flag
    */
    if( cli_strcmp(azCmd[0], "enable")==0 ){
      int ii;
      if( nCmd>2 ) goto session_syntax_error;
      ii = nCmd==1 ? -1 : booleanValue(azCmd[1]);
      if( pAuxDb->nSession ){
        ii = sqlite3session_enable(pSession->p, ii);
        sqlite3_fprintf(p->out,
            "session %s enable flag = %d\n", pSession->zName, ii);
      }
    }else

    /* .session filter GLOB ....
    ** Set a list of GLOB patterns of table names to be excluded.
    */
    if( cli_strcmp(azCmd[0], "filter")==0 ){
      int ii, nByte;
      if( nCmd<2 ) goto session_syntax_error;
      if( pAuxDb->nSession ){
        for(ii=0; ii<pSession->nFilter; ii++){
          sqlite3_free(pSession->azFilter[ii]);
        }
        sqlite3_free(pSession->azFilter);
        nByte = sizeof(pSession->azFilter[0])*(nCmd-1);
        pSession->azFilter = sqlite3_malloc( nByte );
        shell_check_oom( pSession->azFilter );
        for(ii=1; ii<nCmd; ii++){
          char *x = pSession->azFilter[ii-1] = sqlite3_mprintf("%s", azCmd[ii]);
          shell_check_oom(x);
        }
        pSession->nFilter = ii-1;
      }
    }else

    /* .session indirect ?BOOLEAN?
    ** Query or set the indirect flag
    */
    if( cli_strcmp(azCmd[0], "indirect")==0 ){
      int ii;
      if( nCmd>2 ) goto session_syntax_error;
      ii = nCmd==1 ? -1 : booleanValue(azCmd[1]);
      if( pAuxDb->nSession ){
        ii = sqlite3session_indirect(pSession->p, ii);
        sqlite3_fprintf(p->out,
            "session %s indirect flag = %d\n", pSession->zName, ii);
      }
    }else

    /* .session isempty
    ** Determine if the session is empty
    */
    if( cli_strcmp(azCmd[0], "isempty")==0 ){
      int ii;
      if( nCmd!=1 ) goto session_syntax_error;
      if( pAuxDb->nSession ){
        ii = sqlite3session_isempty(pSession->p);
        sqlite3_fprintf(p->out,
             "session %s isempty flag = %d\n", pSession->zName, ii);
      }
    }else

    /* .session list
    ** List all currently open sessions
    */
    if( cli_strcmp(azCmd[0],"list")==0 ){
      for(i=0; i<pAuxDb->nSession; i++){
        sqlite3_fprintf(p->out, "%d %s\n", i, pAuxDb->aSession[i].zName);
      }
    }else

    /* .session open DB NAME
    ** Open a new session called NAME on the attached database DB.
    ** DB is normally "main".
    */
    if( cli_strcmp(azCmd[0],"open")==0 ){
      char *zName;
      if( nCmd!=3 ) goto session_syntax_error;
      zName = azCmd[2];
      if( zName[0]==0 ) goto session_syntax_error;
      for(i=0; i<pAuxDb->nSession; i++){
        if( cli_strcmp(pAuxDb->aSession[i].zName,zName)==0 ){
          sqlite3_fprintf(stderr,"Session \"%s\" already exists\n", zName);
          goto meta_command_exit;
        }
      }
      if( pAuxDb->nSession>=ArraySize(pAuxDb->aSession) ){
        sqlite3_fprintf(stderr,
           "Maximum of %d sessions\n", ArraySize(pAuxDb->aSession));
        goto meta_command_exit;
      }
      pSession = &pAuxDb->aSession[pAuxDb->nSession];
      rc = sqlite3session_create(p->db, azCmd[1], &pSession->p);
      if( rc ){
        sqlite3_fprintf(stderr,"Cannot open session: error code=%d\n", rc);
        rc = 0;
        goto meta_command_exit;
      }
      pSession->nFilter = 0;
      sqlite3session_table_filter(pSession->p, session_filter, pSession);
      pAuxDb->nSession++;
      pSession->zName = sqlite3_mprintf("%s", zName);
      shell_check_oom(pSession->zName);
    }else
    /* If no command name matches, show a syntax error */
    session_syntax_error:
    showHelp(p->out, "session");
  }else
#endif

#ifdef SQLITE_DEBUG
  /* Undocumented commands for internal testing.  Subject to change
  ** without notice. */
  if( c=='s' && n>=10 && cli_strncmp(azArg[0], "selftest-", 9)==0 ){
    if( cli_strncmp(azArg[0]+9, "boolean", n-9)==0 ){
      int i, v;
      for(i=1; i<nArg; i++){
        v = booleanValue(azArg[i]);
        sqlite3_fprintf(p->out, "%s: %d 0x%x\n", azArg[i], v, v);
      }
    }
    if( cli_strncmp(azArg[0]+9, "integer", n-9)==0 ){
      int i; sqlite3_int64 v;
      for(i=1; i<nArg; i++){
        char zBuf[200];
        v = integerValue(azArg[i]);
        sqlite3_snprintf(sizeof(zBuf),zBuf,"%s: %lld 0x%llx\n", azArg[i],v,v);
        sqlite3_fputs(zBuf, p->out);
      }
    }
  }else
#endif

  if( c=='s' && n>=4 && cli_strncmp(azArg[0],"selftest",n)==0 ){
    int bIsInit = 0;         /* True to initialize the SELFTEST table */
    int bVerbose = 0;        /* Verbose output */
    int bSelftestExists;     /* True if SELFTEST already exists */
    int i, k;                /* Loop counters */
    int nTest = 0;           /* Number of tests runs */
    int nErr = 0;            /* Number of errors seen */
    ShellText str;           /* Answer for a query */
    sqlite3_stmt *pStmt = 0; /* Query against the SELFTEST table */

    open_db(p,0);
    for(i=1; i<nArg; i++){
      const char *z = azArg[i];
      if( z[0]=='-' && z[1]=='-' ) z++;
      if( cli_strcmp(z,"-init")==0 ){
        bIsInit = 1;
      }else
      if( cli_strcmp(z,"-v")==0 ){
        bVerbose++;
      }else
      {
        sqlite3_fprintf(stderr,
              "Unknown option \"%s\" on \"%s\"\n", azArg[i], azArg[0]);
        sqlite3_fputs("Should be one of: --init -v\n", stderr);
        rc = 1;
        goto meta_command_exit;
      }
    }
    if( sqlite3_table_column_metadata(p->db,"main","selftest",0,0,0,0,0,0)
           != SQLITE_OK ){
      bSelftestExists = 0;
    }else{
      bSelftestExists = 1;
    }
    if( bIsInit ){
      createSelftestTable(p);
      bSelftestExists = 1;
    }
    initText(&str);
    appendText(&str, "x", 0);
    for(k=bSelftestExists; k>=0; k--){
      if( k==1 ){
        rc = sqlite3_prepare_v2(p->db,
            "SELECT tno,op,cmd,ans FROM selftest ORDER BY tno",
            -1, &pStmt, 0);
      }else{
        rc = sqlite3_prepare_v2(p->db,
          "VALUES(0,'memo','Missing SELFTEST table - default checks only',''),"
          "      (1,'run','PRAGMA integrity_check','ok')",
          -1, &pStmt, 0);
      }
      if( rc ){
        eputz("Error querying the selftest table\n");
        rc = 1;
        sqlite3_finalize(pStmt);
        goto meta_command_exit;
      }
      for(i=1; sqlite3_step(pStmt)==SQLITE_ROW; i++){
        int tno = sqlite3_column_int(pStmt, 0);
        const char *zOp = (const char*)sqlite3_column_text(pStmt, 1);
        const char *zSql = (const char*)sqlite3_column_text(pStmt, 2);
        const char *zAns = (const char*)sqlite3_column_text(pStmt, 3);

        if( zOp==0 ) continue;
        if( zSql==0 ) continue;
        if( zAns==0 ) continue;
        k = 0;
        if( bVerbose>0 ){
          sqlite3_fprintf(stdout, "%d: %s %s\n", tno, zOp, zSql);
        }
        if( cli_strcmp(zOp,"memo")==0 ){
          sqlite3_fprintf(p->out, "%s\n", zSql);
        }else
        if( cli_strcmp(zOp,"run")==0 ){
          char *zErrMsg = 0;
          str.n = 0;
          str.z[0] = 0;
          rc = sqlite3_exec(p->db, zSql, captureOutputCallback, &str, &zErrMsg);
          nTest++;
          if( bVerbose ){
            sqlite3_fprintf(p->out, "Result: %s\n", str.z);
          }
          if( rc || zErrMsg ){
            nErr++;
            rc = 1;
            sqlite3_fprintf(p->out, "%d: error-code-%d: %s\n", tno, rc,zErrMsg);
            sqlite3_free(zErrMsg);
          }else if( cli_strcmp(zAns,str.z)!=0 ){
            nErr++;
            rc = 1;
            sqlite3_fprintf(p->out, "%d: Expected: [%s]\n", tno, zAns);
            sqlite3_fprintf(p->out, "%d:      Got: [%s]\n", tno, str.z);
          }
        }
        else{
          sqlite3_fprintf(stderr,
                "Unknown operation \"%s\" on selftest line %d\n", zOp, tno);
          rc = 1;
          break;
        }
      } /* End loop over rows of content from SELFTEST */
      sqlite3_finalize(pStmt);
    } /* End loop over k */
    freeText(&str);
    sqlite3_fprintf(p->out, "%d errors out of %d tests\n", nErr, nTest);
  }else

  if( c=='s' && cli_strncmp(azArg[0], "separator", n)==0 ){
    if( nArg<2 || nArg>3 ){
      eputz("Usage: .separator COL ?ROW?\n");
      rc = 1;
    }
    if( nArg>=2 ){
      sqlite3_snprintf(sizeof(p->colSeparator), p->colSeparator,
                       "%.*s", (int)ArraySize(p->colSeparator)-1, azArg[1]);
    }
    if( nArg>=3 ){
      sqlite3_snprintf(sizeof(p->rowSeparator), p->rowSeparator,
                       "%.*s", (int)ArraySize(p->rowSeparator)-1, azArg[2]);
    }
  }else

  if( c=='s' && n>=4 && cli_strncmp(azArg[0],"sha3sum",n)==0 ){
    const char *zLike = 0;   /* Which table to checksum. 0 means everything */
    int i;                   /* Loop counter */
    int bSchema = 0;         /* Also hash the schema */
    int bSeparate = 0;       /* Hash each table separately */
    int iSize = 224;         /* Hash algorithm to use */
    int bDebug = 0;          /* Only show the query that would have run */
    sqlite3_stmt *pStmt;     /* For querying tables names */
    char *zSql;              /* SQL to be run */
    char *zSep;              /* Separator */
    ShellText sSql;          /* Complete SQL for the query to run the hash */
    ShellText sQuery;        /* Set of queries used to read all content */
    open_db(p, 0);
    for(i=1; i<nArg; i++){
      const char *z = azArg[i];
      if( z[0]=='-' ){
        z++;
        if( z[0]=='-' ) z++;
        if( cli_strcmp(z,"schema")==0 ){
          bSchema = 1;
        }else
        if( cli_strcmp(z,"sha3-224")==0 || cli_strcmp(z,"sha3-256")==0
         || cli_strcmp(z,"sha3-384")==0 || cli_strcmp(z,"sha3-512")==0
        ){
          iSize = atoi(&z[5]);
        }else
        if( cli_strcmp(z,"debug")==0 ){
          bDebug = 1;
        }else
        {
          sqlite3_fprintf(stderr,
                  "Unknown option \"%s\" on \"%s\"\n", azArg[i], azArg[0]);
          showHelp(p->out, azArg[0]);
          rc = 1;
          goto meta_command_exit;
        }
      }else if( zLike ){
        eputz("Usage: .sha3sum ?OPTIONS? ?LIKE-PATTERN?\n");
        rc = 1;
        goto meta_command_exit;
      }else{
        zLike = z;
        bSeparate = 1;
        if( sqlite3_strlike("sqlite\\_%", zLike, '\\')==0 ) bSchema = 1;
      }
    }
    if( bSchema ){
      zSql = "SELECT lower(name) as tname FROM sqlite_schema"
             " WHERE type='table' AND coalesce(rootpage,0)>1"
             " UNION ALL SELECT 'sqlite_schema'"
             " ORDER BY 1 collate nocase";
    }else{
      zSql = "SELECT lower(name) as tname FROM sqlite_schema"
             " WHERE type='table' AND coalesce(rootpage,0)>1"
             " AND name NOT LIKE 'sqlite_%'"
             " ORDER BY 1 collate nocase";
    }
    sqlite3_prepare_v2(p->db, zSql, -1, &pStmt, 0);
    initText(&sQuery);
    initText(&sSql);
    appendText(&sSql, "WITH [sha3sum$query](a,b) AS(",0);
    zSep = "VALUES(";
    while( SQLITE_ROW==sqlite3_step(pStmt) ){
      const char *zTab = (const char*)sqlite3_column_text(pStmt,0);
      if( zTab==0 ) continue;
      if( zLike && sqlite3_strlike(zLike, zTab, 0)!=0 ) continue;
      if( cli_strncmp(zTab, "sqlite_",7)!=0 ){
        appendText(&sQuery,"SELECT * FROM ", 0);
        appendText(&sQuery,zTab,'"');
        appendText(&sQuery," NOT INDEXED;", 0);
      }else if( cli_strcmp(zTab, "sqlite_schema")==0 ){
        appendText(&sQuery,"SELECT type,name,tbl_name,sql FROM sqlite_schema"
                           " ORDER BY name;", 0);
      }else if( cli_strcmp(zTab, "sqlite_sequence")==0 ){
        appendText(&sQuery,"SELECT name,seq FROM sqlite_sequence"
                           " ORDER BY name;", 0);
      }else if( cli_strcmp(zTab, "sqlite_stat1")==0 ){
        appendText(&sQuery,"SELECT tbl,idx,stat FROM sqlite_stat1"
                           " ORDER BY tbl,idx;", 0);
      }else if( cli_strcmp(zTab, "sqlite_stat4")==0 ){
        appendText(&sQuery, "SELECT * FROM ", 0);
        appendText(&sQuery, zTab, 0);
        appendText(&sQuery, " ORDER BY tbl, idx, rowid;\n", 0);
      }
      appendText(&sSql, zSep, 0);
      appendText(&sSql, sQuery.z, '\'');
      sQuery.n = 0;
      appendText(&sSql, ",", 0);
      appendText(&sSql, zTab, '\'');
      zSep = "),(";
    }
    sqlite3_finalize(pStmt);
    if( bSeparate ){
      zSql = sqlite3_mprintf(
          "%s))"
          " SELECT lower(hex(sha3_query(a,%d))) AS hash, b AS label"
          "   FROM [sha3sum$query]",
          sSql.z, iSize);
    }else{
      zSql = sqlite3_mprintf(
          "%s))"
          " SELECT lower(hex(sha3_query(group_concat(a,''),%d))) AS hash"
          "   FROM [sha3sum$query]",
          sSql.z, iSize);
    }
    shell_check_oom(zSql);
    freeText(&sQuery);
    freeText(&sSql);
    if( bDebug ){
      sqlite3_fprintf(p->out, "%s\n", zSql);
    }else{
      shell_exec(p, zSql, 0);
    }
#if !defined(SQLITE_OMIT_SCHEMA_PRAGMAS) && !defined(SQLITE_OMIT_VIRTUALTABLE)
    {
      int lrc;
      char *zRevText = /* Query for reversible to-blob-to-text check */
        "SELECT lower(name) as tname FROM sqlite_schema\n"
        "WHERE type='table' AND coalesce(rootpage,0)>1\n"
        "AND name NOT LIKE 'sqlite_%%'%s\n"
        "ORDER BY 1 collate nocase";
      zRevText = sqlite3_mprintf(zRevText, zLike? " AND name LIKE $tspec" : "");
      zRevText = sqlite3_mprintf(
          /* lower-case query is first run, producing upper-case query. */
          "with tabcols as materialized(\n"
          "select tname, cname\n"
          "from ("
          " select printf('\"%%w\"',ss.tname) as tname,"
          " printf('\"%%w\"',ti.name) as cname\n"
          " from (%z) ss\n inner join pragma_table_info(tname) ti))\n"
          "select 'SELECT total(bad_text_count) AS bad_text_count\n"
          "FROM ('||group_concat(query, ' UNION ALL ')||')' as btc_query\n"
          " from (select 'SELECT COUNT(*) AS bad_text_count\n"
          "FROM '||tname||' WHERE '\n"
          "||group_concat('CAST(CAST('||cname||' AS BLOB) AS TEXT)<>'||cname\n"
          "|| ' AND typeof('||cname||')=''text'' ',\n"
          "' OR ') as query, tname from tabcols group by tname)"
          , zRevText);
      shell_check_oom(zRevText);
      if( bDebug ) sqlite3_fprintf(p->out, "%s\n", zRevText);
      lrc = sqlite3_prepare_v2(p->db, zRevText, -1, &pStmt, 0);
      if( lrc!=SQLITE_OK ){
        /* assert(lrc==SQLITE_NOMEM); // might also be SQLITE_ERROR if the
        ** user does cruel and unnatural things like ".limit expr_depth 0". */
        rc = 1;
      }else{
        if( zLike ) sqlite3_bind_text(pStmt,1,zLike,-1,SQLITE_STATIC);
        lrc = SQLITE_ROW==sqlite3_step(pStmt);
        if( lrc ){
          const char *zGenQuery = (char*)sqlite3_column_text(pStmt,0);
          sqlite3_stmt *pCheckStmt;
          lrc = sqlite3_prepare_v2(p->db, zGenQuery, -1, &pCheckStmt, 0);
          if( bDebug ) sqlite3_fprintf(p->out, "%s\n", zGenQuery);
          if( lrc!=SQLITE_OK ){
            rc = 1;
          }else{
            if( SQLITE_ROW==sqlite3_step(pCheckStmt) ){
              double countIrreversible = sqlite3_column_double(pCheckStmt, 0);
              if( countIrreversible>0 ){
                int sz = (int)(countIrreversible + 0.5);
                sqlite3_fprintf(stderr,
                      "Digest includes %d invalidly encoded text field%s.\n",
                      sz, (sz>1)? "s": "");
              }
            }
            sqlite3_finalize(pCheckStmt);
          }
          sqlite3_finalize(pStmt);
        }
      }
      if( rc ) eputz(".sha3sum failed.\n");
      sqlite3_free(zRevText);
    }
#endif /* !defined(*_OMIT_SCHEMA_PRAGMAS) && !defined(*_OMIT_VIRTUALTABLE) */
    sqlite3_free(zSql);
  }else

#if !defined(SQLITE_NOHAVE_SYSTEM) && !defined(SQLITE_SHELL_FIDDLE)
  if( c=='s'
   && (cli_strncmp(azArg[0], "shell", n)==0
       || cli_strncmp(azArg[0],"system",n)==0)
  ){
    char *zCmd;
    int i, x;
    failIfSafeMode(p, "cannot run .%s in safe mode", azArg[0]);
    if( nArg<2 ){
      eputz("Usage: .system COMMAND\n");
      rc = 1;
      goto meta_command_exit;
    }
    zCmd = sqlite3_mprintf(strchr(azArg[1],' ')==0?"%s":"\"%s\"", azArg[1]);
    for(i=2; i<nArg && zCmd!=0; i++){
      zCmd = sqlite3_mprintf(strchr(azArg[i],' ')==0?"%z %s":"%z \"%s\"",
                             zCmd, azArg[i]);
    }
    /*consoleRestore();*/
    x = zCmd!=0 ? system(zCmd) : 1;
    /*consoleRenewSetup();*/
    sqlite3_free(zCmd);
    if( x ) sqlite3_fprintf(stderr,"System command returns %d\n", x);
  }else
#endif /* !defined(SQLITE_NOHAVE_SYSTEM) && !defined(SQLITE_SHELL_FIDDLE) */

  if( c=='s' && cli_strncmp(azArg[0], "show", n)==0 ){
    static const char *azBool[] = { "off", "on", "trigger", "full"};
    const char *zOut;
    int i;
    if( nArg!=1 ){
      eputz("Usage: .show\n");
      rc = 1;
      goto meta_command_exit;
    }
    sqlite3_fprintf(p->out, "%12.12s: %s\n","echo",
          azBool[ShellHasFlag(p, SHFLG_Echo)]);
    sqlite3_fprintf(p->out, "%12.12s: %s\n","eqp", azBool[p->autoEQP&3]);
    sqlite3_fprintf(p->out, "%12.12s: %s\n","explain",
          p->mode==MODE_Explain ? "on" : p->autoExplain ? "auto" : "off");
    sqlite3_fprintf(p->out, "%12.12s: %s\n","headers",
          azBool[p->showHeader!=0]);
    if( p->mode==MODE_Column
     || (p->mode>=MODE_Markdown && p->mode<=MODE_Box)
    ){
      sqlite3_fprintf(p->out,
            "%12.12s: %s --wrap %d --wordwrap %s --%squote\n", "mode",
            modeDescr[p->mode], p->cmOpts.iWrap,
            p->cmOpts.bWordWrap ? "on" : "off",
            p->cmOpts.bQuote ? "" : "no");
    }else{
      sqlite3_fprintf(p->out, "%12.12s: %s\n","mode", modeDescr[p->mode]);
    }
    sqlite3_fprintf(p->out, "%12.12s: ", "nullvalue");
    output_c_string(p->out, p->nullValue);
    sqlite3_fputs("\n", p->out);
    sqlite3_fprintf(p->out, "%12.12s: %s\n","output",
          strlen30(p->outfile) ? p->outfile : "stdout");
    sqlite3_fprintf(p->out, "%12.12s: ", "colseparator");
    output_c_string(p->out, p->colSeparator);
    sqlite3_fputs("\n", p->out);
    sqlite3_fprintf(p->out, "%12.12s: ", "rowseparator");
    output_c_string(p->out, p->rowSeparator);
    sqlite3_fputs("\n", p->out);
    switch( p->statsOn ){
      case 0:  zOut = "off";     break;
      default: zOut = "on";      break;
      case 2:  zOut = "stmt";    break;
      case 3:  zOut = "vmstep";  break;
    }
    sqlite3_fprintf(p->out, "%12.12s: %s\n","stats", zOut);
    sqlite3_fprintf(p->out, "%12.12s: ", "width");
    for (i=0;i<p->nWidth;i++) {
      sqlite3_fprintf(p->out, "%d ", p->colWidth[i]);
    }
    sqlite3_fputs("\n", p->out);
    sqlite3_fprintf(p->out, "%12.12s: %s\n", "filename",
          p->pAuxDb->zDbFilename ? p->pAuxDb->zDbFilename : "");
  }else

  if( c=='s' && cli_strncmp(azArg[0], "stats", n)==0 ){
    if( nArg==2 ){
      if( cli_strcmp(azArg[1],"stmt")==0 ){
        p->statsOn = 2;
      }else if( cli_strcmp(azArg[1],"vmstep")==0 ){
        p->statsOn = 3;
      }else{
        p->statsOn = (u8)booleanValue(azArg[1]);
      }
    }else if( nArg==1 ){
      display_stats(p->db, p, 0);
    }else{
      eputz("Usage: .stats ?on|off|stmt|vmstep?\n");
      rc = 1;
    }
  }else

  if( (c=='t' && n>1 && cli_strncmp(azArg[0], "tables", n)==0)
   || (c=='i' && (cli_strncmp(azArg[0], "indices", n)==0
                 || cli_strncmp(azArg[0], "indexes", n)==0) )
  ){
    sqlite3_stmt *pStmt;
    char **azResult;
    int nRow, nAlloc;
    int ii;
    ShellText s;
    initText(&s);
    open_db(p, 0);
    rc = sqlite3_prepare_v2(p->db, "PRAGMA database_list", -1, &pStmt, 0);
    if( rc ){
      sqlite3_finalize(pStmt);
      return shellDatabaseError(p->db);
    }

    if( nArg>2 && c=='i' ){
      /* It is an historical accident that the .indexes command shows an error
      ** when called with the wrong number of arguments whereas the .tables
      ** command does not. */
      eputz("Usage: .indexes ?LIKE-PATTERN?\n");
      rc = 1;
      sqlite3_finalize(pStmt);
      goto meta_command_exit;
    }
    for(ii=0; sqlite3_step(pStmt)==SQLITE_ROW; ii++){
      const char *zDbName = (const char*)sqlite3_column_text(pStmt, 1);
      if( zDbName==0 ) continue;
      if( s.z && s.z[0] ) appendText(&s, " UNION ALL ", 0);
      if( sqlite3_stricmp(zDbName, "main")==0 ){
        appendText(&s, "SELECT name FROM ", 0);
      }else{
        appendText(&s, "SELECT ", 0);
        appendText(&s, zDbName, '\'');
        appendText(&s, "||'.'||name FROM ", 0);
      }
      appendText(&s, zDbName, '"');
      appendText(&s, ".sqlite_schema ", 0);
      if( c=='t' ){
        appendText(&s," WHERE type IN ('table','view')"
                      "   AND name NOT LIKE 'sqlite_%'"
                      "   AND name LIKE ?1", 0);
      }else{
        appendText(&s," WHERE type='index'"
                      "   AND tbl_name LIKE ?1", 0);
      }
    }
    rc = sqlite3_finalize(pStmt);
    if( rc==SQLITE_OK ){
      appendText(&s, " ORDER BY 1", 0);
      rc = sqlite3_prepare_v2(p->db, s.z, -1, &pStmt, 0);
    }
    freeText(&s);
    if( rc ) return shellDatabaseError(p->db);

    /* Run the SQL statement prepared by the above block. Store the results
    ** as an array of nul-terminated strings in azResult[].  */
    nRow = nAlloc = 0;
    azResult = 0;
    if( nArg>1 ){
      sqlite3_bind_text(pStmt, 1, azArg[1], -1, SQLITE_TRANSIENT);
    }else{
      sqlite3_bind_text(pStmt, 1, "%", -1, SQLITE_STATIC);
    }
    while( sqlite3_step(pStmt)==SQLITE_ROW ){
      if( nRow>=nAlloc ){
        char **azNew;
        int n2 = nAlloc*2 + 10;
        azNew = sqlite3_realloc64(azResult, sizeof(azResult[0])*n2);
        shell_check_oom(azNew);
        nAlloc = n2;
        azResult = azNew;
      }
      azResult[nRow] = sqlite3_mprintf("%s", sqlite3_column_text(pStmt, 0));
      shell_check_oom(azResult[nRow]);
      nRow++;
    }
    if( sqlite3_finalize(pStmt)!=SQLITE_OK ){
      rc = shellDatabaseError(p->db);
    }

    /* Pretty-print the contents of array azResult[] to the output */
    if( rc==0 && nRow>0 ){
      int len, maxlen = 0;
      int i, j;
      int nPrintCol, nPrintRow;
      for(i=0; i<nRow; i++){
        len = strlen30(azResult[i]);
        if( len>maxlen ) maxlen = len;
      }
      nPrintCol = 80/(maxlen+2);
      if( nPrintCol<1 ) nPrintCol = 1;
      nPrintRow = (nRow + nPrintCol - 1)/nPrintCol;
      for(i=0; i<nPrintRow; i++){
        for(j=i; j<nRow; j+=nPrintRow){
          char *zSp = j<nPrintRow ? "" : "  ";
          sqlite3_fprintf(p->out,
               "%s%-*s", zSp, maxlen, azResult[j] ? azResult[j]:"");
        }
        sqlite3_fputs("\n", p->out);
      }
    }

    for(ii=0; ii<nRow; ii++) sqlite3_free(azResult[ii]);
    sqlite3_free(azResult);
  }else

#ifndef SQLITE_SHELL_FIDDLE
  /* Begin redirecting output to the file "testcase-out.txt" */
  if( c=='t' && cli_strcmp(azArg[0],"testcase")==0 ){
    output_reset(p);
    p->out = output_file_open("testcase-out.txt");
    if( p->out==0 ){
      eputz("Error: cannot open 'testcase-out.txt'\n");
    }
    if( nArg>=2 ){
      sqlite3_snprintf(sizeof(p->zTestcase), p->zTestcase, "%s", azArg[1]);
    }else{
      sqlite3_snprintf(sizeof(p->zTestcase), p->zTestcase, "?");
    }
  }else
#endif /* !defined(SQLITE_SHELL_FIDDLE) */

#ifndef SQLITE_UNTESTABLE
  if( c=='t' && n>=8 && cli_strncmp(azArg[0], "testctrl", n)==0 ){
    static const struct {
       const char *zCtrlName;   /* Name of a test-control option */
       int ctrlCode;            /* Integer code for that option */
       int unSafe;              /* Not valid unless --unsafe-testing */
       const char *zUsage;      /* Usage notes */
    } aCtrl[] = {
    {"always",             SQLITE_TESTCTRL_ALWAYS, 1,     "BOOLEAN"         },
    {"assert",             SQLITE_TESTCTRL_ASSERT, 1,     "BOOLEAN"         },
  /*{"benign_malloc_hooks",SQLITE_TESTCTRL_BENIGN_MALLOC_HOOKS,1, ""        },*/
  /*{"bitvec_test",        SQLITE_TESTCTRL_BITVEC_TEST, 1,  ""              },*/
    {"byteorder",          SQLITE_TESTCTRL_BYTEORDER, 0,  ""                },
    {"extra_schema_checks",SQLITE_TESTCTRL_EXTRA_SCHEMA_CHECKS,0,"BOOLEAN"  },
    {"fault_install",      SQLITE_TESTCTRL_FAULT_INSTALL, 1,"args..."       },
    {"fk_no_action",       SQLITE_TESTCTRL_FK_NO_ACTION, 0, "BOOLEAN"       },
    {"imposter",         SQLITE_TESTCTRL_IMPOSTER,1,"SCHEMA ON/OFF ROOTPAGE"},
    {"internal_functions", SQLITE_TESTCTRL_INTERNAL_FUNCTIONS,0,""          },
    {"json_selfcheck",     SQLITE_TESTCTRL_JSON_SELFCHECK ,0,"BOOLEAN"      },
    {"localtime_fault",    SQLITE_TESTCTRL_LOCALTIME_FAULT,0,"BOOLEAN"      },
    {"never_corrupt",      SQLITE_TESTCTRL_NEVER_CORRUPT,1, "BOOLEAN"       },
    {"optimizations",      SQLITE_TESTCTRL_OPTIMIZATIONS,0,"DISABLE-MASK ..."},
#ifdef YYCOVERAGE
    {"parser_coverage",    SQLITE_TESTCTRL_PARSER_COVERAGE,0,""             },
#endif
    {"pending_byte",       SQLITE_TESTCTRL_PENDING_BYTE,1, "OFFSET  "       },
    {"prng_restore",       SQLITE_TESTCTRL_PRNG_RESTORE,0, ""               },
    {"prng_save",          SQLITE_TESTCTRL_PRNG_SAVE,   0, ""               },
    {"prng_seed",          SQLITE_TESTCTRL_PRNG_SEED,   0, "SEED ?db?"      },
    {"seek_count",         SQLITE_TESTCTRL_SEEK_COUNT,  0, ""               },
    {"sorter_mmap",        SQLITE_TESTCTRL_SORTER_MMAP, 0, "NMAX"           },
    {"tune",               SQLITE_TESTCTRL_TUNE,        1, "ID VALUE"       },
    };
    int testctrl = -1;
    int iCtrl = -1;
    int rc2 = 0;    /* 0: usage.  1: %d  2: %x  3: no-output */
    int isOk = 0;
    int i, n2;
    const char *zCmd = 0;

    open_db(p, 0);
    zCmd = nArg>=2 ? azArg[1] : "help";

    /* The argument can optionally begin with "-" or "--" */
    if( zCmd[0]=='-' && zCmd[1] ){
      zCmd++;
      if( zCmd[0]=='-' && zCmd[1] ) zCmd++;
    }

    /* --help lists all test-controls */
    if( cli_strcmp(zCmd,"help")==0 ){
      sqlite3_fputs("Available test-controls:\n", p->out);
      for(i=0; i<ArraySize(aCtrl); i++){
        if( aCtrl[i].unSafe && !ShellHasFlag(p,SHFLG_TestingMode) ) continue;
        sqlite3_fprintf(p->out, "  .testctrl %s %s\n",
              aCtrl[i].zCtrlName, aCtrl[i].zUsage);
      }
      rc = 1;
      goto meta_command_exit;
    }

    /* convert testctrl text option to value. allow any unique prefix
    ** of the option name, or a numerical value. */
    n2 = strlen30(zCmd);
    for(i=0; i<ArraySize(aCtrl); i++){
      if( aCtrl[i].unSafe && !ShellHasFlag(p,SHFLG_TestingMode) ) continue;
      if( cli_strncmp(zCmd, aCtrl[i].zCtrlName, n2)==0 ){
        if( testctrl<0 ){
          testctrl = aCtrl[i].ctrlCode;
          iCtrl = i;
        }else{
          sqlite3_fprintf(stderr,"Error: ambiguous test-control: \"%s\"\n"
                "Use \".testctrl --help\" for help\n", zCmd);
          rc = 1;
          goto meta_command_exit;
        }
      }
    }
    if( testctrl<0 ){
      sqlite3_fprintf(stderr,"Error: unknown test-control: %s\n"
            "Use \".testctrl --help\" for help\n", zCmd);
    }else{
      switch(testctrl){

        /* Special processing for .testctrl opt MASK ...
        ** Each MASK argument can be one of:
        **
        **      +LABEL       Enable the named optimization 
        **
        **      -LABEL       Disable the named optimization
        **
        **      INTEGER      Mask of optimizations to disable
        */
        case SQLITE_TESTCTRL_OPTIMIZATIONS: {
          static const struct {
             unsigned int mask;    /* Mask for this optimization */
             unsigned int bDsply;  /* Display this on output */
             const char *zLabel;   /* Name of optimization */
          } aLabel[] = {
            { 0x00000001, 1, "QueryFlattener" },
            { 0x00000001, 0, "Flatten" },
            { 0x00000002, 1, "WindowFunc" },
            { 0x00000004, 1, "GroupByOrder" },
            { 0x00000008, 1, "FactorOutConst" },
            { 0x00000010, 1, "DistinctOpt" },
            { 0x00000020, 1, "CoverIdxScan" },
            { 0x00000040, 1, "OrderByIdxJoin" },
            { 0x00000080, 1, "Transitive" },
            { 0x00000100, 1, "OmitNoopJoin" },
            { 0x00000200, 1, "CountOfView" },
            { 0x00000400, 1, "CurosrHints" },
            { 0x00000800, 1, "Stat4" },
            { 0x00001000, 1, "PushDown" },
            { 0x00002000, 1, "SimplifyJoin" },
            { 0x00004000, 1, "SkipScan" },
            { 0x00008000, 1, "PropagateConst" },
            { 0x00010000, 1, "MinMaxOpt" },
            { 0x00020000, 1, "SeekScan" },
            { 0x00040000, 1, "OmitOrderBy" },
            { 0x00080000, 1, "BloomFilter" },
            { 0x00100000, 1, "BloomPulldown" },
            { 0x00200000, 1, "BalancedMerge" },
            { 0x00400000, 1, "ReleaseReg" },
            { 0x00800000, 1, "FlttnUnionAll" },
            { 0x01000000, 1, "IndexedEXpr" },
            { 0x02000000, 1, "Coroutines" },
            { 0x04000000, 1, "NullUnusedCols" },
            { 0x08000000, 1, "OnePass" },
            { 0x10000000, 1, "OrderBySubq" },
            { 0x20000000, 1, "StarQuery" },
            { 0xffffffff, 0, "All" },
          };
          unsigned int curOpt;
          unsigned int newOpt;
          unsigned int m;
          int ii;
          int nOff;
          sqlite3_test_control(SQLITE_TESTCTRL_GETOPT, p->db, &curOpt);
          newOpt = curOpt;
          for(ii=2; ii<nArg; ii++){
            const char *z = azArg[ii];
            int useLabel = 0;
            const char *zLabel = 0;
            if( (z[0]=='+'|| z[0]=='-') && !IsDigit(z[1]) ){
              useLabel = z[0];
              zLabel = &z[1];
            }else if( !IsDigit(z[0]) && z[0]!=0 && !IsDigit(z[1]) ){
              useLabel = '+';
              zLabel = z;
            }else{
              newOpt = (unsigned int)strtol(z,0,0);
            }
            if( useLabel ){
              int jj;
              for(jj=0; jj<ArraySize(aLabel); jj++){
                if( sqlite3_stricmp(zLabel, aLabel[jj].zLabel)==0 ) break;
              }
              if( jj>=ArraySize(aLabel) ){
                sqlite3_fprintf(stderr,
                    "Error: no such optimization: \"%s\"\n", zLabel);
                sqlite3_fputs("Should be one of:", stderr);
                for(jj=0; jj<ArraySize(aLabel); jj++){
                  sqlite3_fprintf(stderr," %s", aLabel[jj].zLabel);
                }
                sqlite3_fputs("\n", stderr);
                rc = 1;
                goto meta_command_exit;
              }
              if( useLabel=='+' ){
                newOpt &= ~aLabel[jj].mask;
              }else{
                newOpt |= aLabel[jj].mask;
              }
            }
          }
          if( curOpt!=newOpt ){
            sqlite3_test_control(SQLITE_TESTCTRL_OPTIMIZATIONS,p->db,newOpt);
          }
          for(ii=nOff=0, m=1; ii<32; ii++, m <<= 1){
            if( m & newOpt ) nOff++;
          }
          if( nOff<12 ){
            sqlite3_fputs("+All", p->out);
            for(ii=0; ii<ArraySize(aLabel); ii++){
              if( !aLabel[ii].bDsply  ) continue;
              if( (newOpt & aLabel[ii].mask)!=0 ){
                sqlite3_fprintf(p->out, " -%s", aLabel[ii].zLabel);
              }
            }
          }else{
            sqlite3_fputs("-All", p->out);
            for(ii=0; ii<ArraySize(aLabel); ii++){
              if( !aLabel[ii].bDsply  ) continue;
              if( (newOpt & aLabel[ii].mask)==0 ){
                sqlite3_fprintf(p->out, " +%s", aLabel[ii].zLabel);
              }
            }
          }
          sqlite3_fputs("\n", p->out);
          rc2 = isOk = 3;
          break;
        }

        /* sqlite3_test_control(int, db, int) */
        case SQLITE_TESTCTRL_FK_NO_ACTION:
          if( nArg==3 ){
            unsigned int opt = (unsigned int)strtol(azArg[2], 0, 0);
            rc2 = sqlite3_test_control(testctrl, p->db, opt);
            isOk = 3;
          }
          break;

        /* sqlite3_test_control(int) */
        case SQLITE_TESTCTRL_PRNG_SAVE:
        case SQLITE_TESTCTRL_PRNG_RESTORE:
        case SQLITE_TESTCTRL_BYTEORDER:
          if( nArg==2 ){
            rc2 = sqlite3_test_control(testctrl);
            isOk = testctrl==SQLITE_TESTCTRL_BYTEORDER ? 1 : 3;
          }
          break;

        /* sqlite3_test_control(int, uint) */
        case SQLITE_TESTCTRL_PENDING_BYTE:
          if( nArg==3 ){
            unsigned int opt = (unsigned int)integerValue(azArg[2]);
            rc2 = sqlite3_test_control(testctrl, opt);
            isOk = 3;
          }
          break;

        /* sqlite3_test_control(int, int, sqlite3*) */
        case SQLITE_TESTCTRL_PRNG_SEED:
          if( nArg==3 || nArg==4 ){
            int ii = (int)integerValue(azArg[2]);
            sqlite3 *db;
            if( ii==0 && cli_strcmp(azArg[2],"random")==0 ){
              sqlite3_randomness(sizeof(ii),&ii);
              sqlite3_fprintf(stdout, "-- random seed: %d\n", ii);
            }
            if( nArg==3 ){
              db = 0;
            }else{
              db = p->db;
              /* Make sure the schema has been loaded */
              sqlite3_table_column_metadata(db, 0, "x", 0, 0, 0, 0, 0, 0);
            }
            rc2 = sqlite3_test_control(testctrl, ii, db);
            isOk = 3;
          }
          break;

        /* sqlite3_test_control(int, int) */
        case SQLITE_TESTCTRL_ASSERT:
        case SQLITE_TESTCTRL_ALWAYS:
          if( nArg==3 ){
            int opt = booleanValue(azArg[2]);
            rc2 = sqlite3_test_control(testctrl, opt);
            isOk = 1;
          }
          break;

        /* sqlite3_test_control(int, int) */
        case SQLITE_TESTCTRL_LOCALTIME_FAULT:
        case SQLITE_TESTCTRL_NEVER_CORRUPT:
          if( nArg==3 ){
            int opt = booleanValue(azArg[2]);
            rc2 = sqlite3_test_control(testctrl, opt);
            isOk = 3;
          }
          break;

        /* sqlite3_test_control(sqlite3*) */
        case SQLITE_TESTCTRL_INTERNAL_FUNCTIONS:
          rc2 = sqlite3_test_control(testctrl, p->db);
          isOk = 3;
          break;

        case SQLITE_TESTCTRL_IMPOSTER:
          if( nArg==5 ){
            rc2 = sqlite3_test_control(testctrl, p->db,
                          azArg[2],
                          integerValue(azArg[3]),
                          integerValue(azArg[4]));
            isOk = 3;
          }
          break;

        case SQLITE_TESTCTRL_SEEK_COUNT: {
          u64 x = 0;
          rc2 = sqlite3_test_control(testctrl, p->db, &x);
          sqlite3_fprintf(p->out, "%llu\n", x);
          isOk = 3;
          break;
        }

#ifdef YYCOVERAGE
        case SQLITE_TESTCTRL_PARSER_COVERAGE: {
          if( nArg==2 ){
            sqlite3_test_control(testctrl, p->out);
            isOk = 3;
          }
          break;
        }
#endif
#ifdef SQLITE_DEBUG
        case SQLITE_TESTCTRL_TUNE: {
          if( nArg==4 ){
            int id = (int)integerValue(azArg[2]);
            int val = (int)integerValue(azArg[3]);
            sqlite3_test_control(testctrl, id, &val);
            isOk = 3;
          }else if( nArg==3 ){
            int id = (int)integerValue(azArg[2]);
            sqlite3_test_control(testctrl, -id, &rc2);
            isOk = 1;
          }else if( nArg==2 ){
            int id = 1;
            while(1){
              int val = 0;
              rc2 = sqlite3_test_control(testctrl, -id, &val);
              if( rc2!=SQLITE_OK ) break;
              if( id>1 ) sqlite3_fputs("  ", p->out);
              sqlite3_fprintf(p->out, "%d: %d", id, val);
              id++;
            }
            if( id>1 ) sqlite3_fputs("\n", p->out);
            isOk = 3;
          }
          break;
        }
#endif
        case SQLITE_TESTCTRL_SORTER_MMAP:
          if( nArg==3 ){
            int opt = (unsigned int)integerValue(azArg[2]);
            rc2 = sqlite3_test_control(testctrl, p->db, opt);
            isOk = 3;
          }
          break;
        case SQLITE_TESTCTRL_JSON_SELFCHECK:
          if( nArg==2 ){
            rc2 = -1;
            isOk = 1;
          }else{
            rc2 = booleanValue(azArg[2]);
            isOk = 3;
          }
          sqlite3_test_control(testctrl, &rc2);
          break;
        case SQLITE_TESTCTRL_FAULT_INSTALL: {
          int kk;
          int bShowHelp = nArg<=2;
          isOk = 3;
          for(kk=2; kk<nArg; kk++){
            const char *z = azArg[kk];
            if( z[0]=='-' && z[1]=='-' ) z++;
            if( cli_strcmp(z,"off")==0 ){
              sqlite3_test_control(testctrl, 0);
            }else if( cli_strcmp(z,"on")==0 ){
              faultsim_state.iCnt = faultsim_state.nSkip;
              if( faultsim_state.iErr==0 ) faultsim_state.iErr = 1;
              faultsim_state.nHit = 0;
              sqlite3_test_control(testctrl, faultsim_callback);
            }else if( cli_strcmp(z,"reset")==0 ){
              faultsim_state.iCnt = faultsim_state.nSkip;
              faultsim_state.nHit = 0;
              sqlite3_test_control(testctrl, faultsim_callback);
            }else if( cli_strcmp(z,"status")==0 ){
              sqlite3_fprintf(p->out, "faultsim.iId:       %d\n",
                              faultsim_state.iId);
              sqlite3_fprintf(p->out, "faultsim.iErr:      %d\n",
                              faultsim_state.iErr);
              sqlite3_fprintf(p->out, "faultsim.iCnt:      %d\n",
                              faultsim_state.iCnt);
              sqlite3_fprintf(p->out, "faultsim.nHit:      %d\n",
                              faultsim_state.nHit);
              sqlite3_fprintf(p->out, "faultsim.iInterval: %d\n",
                              faultsim_state.iInterval);
              sqlite3_fprintf(p->out, "faultsim.eVerbose:  %d\n",
                              faultsim_state.eVerbose);
              sqlite3_fprintf(p->out, "faultsim.nRepeat:   %d\n",
                              faultsim_state.nRepeat);
              sqlite3_fprintf(p->out, "faultsim.nSkip:     %d\n",
                              faultsim_state.nSkip);
            }else if( cli_strcmp(z,"-v")==0 ){
              if( faultsim_state.eVerbose<2 ) faultsim_state.eVerbose++;
            }else if( cli_strcmp(z,"-q")==0 ){
              if( faultsim_state.eVerbose>0 ) faultsim_state.eVerbose--;
            }else if( cli_strcmp(z,"-id")==0 && kk+1<nArg ){
              faultsim_state.iId = atoi(azArg[++kk]);
            }else if( cli_strcmp(z,"-errcode")==0 && kk+1<nArg ){
              faultsim_state.iErr = atoi(azArg[++kk]);
            }else if( cli_strcmp(z,"-interval")==0 && kk+1<nArg ){
              faultsim_state.iInterval = atoi(azArg[++kk]);
            }else if( cli_strcmp(z,"-repeat")==0 && kk+1<nArg ){
              faultsim_state.nRepeat = atoi(azArg[++kk]);
           }else if( cli_strcmp(z,"-skip")==0 && kk+1<nArg ){
              faultsim_state.nSkip = atoi(azArg[++kk]);
            }else if( cli_strcmp(z,"-?")==0 || sqlite3_strglob("*help*",z)==0){
              bShowHelp = 1;
            }else{
              sqlite3_fprintf(stderr,
                  "Unrecognized fault_install argument: \"%s\"\n",
                  azArg[kk]);
              rc = 1;
              bShowHelp = 1;
              break;
            }
          }
          if( bShowHelp ){
            sqlite3_fputs(
               "Usage: .testctrl fault_install ARGS\n"
               "Possible arguments:\n"
               "   off               Disable faultsim\n"
               "   on                Activate faultsim\n"
               "   reset             Reset the trigger counter\n"
               "   status            Show current status\n"
               "   -v                Increase verbosity\n"
               "   -q                Decrease verbosity\n"
               "   --errcode N       When triggered, return N as error code\n"
               "   --id ID           Trigger only for the ID specified\n"
               "   --interval N      Trigger only after every N-th call\n"
               "   --repeat N        Turn off after N hits.  0 means never\n"
               "   --skip N          Skip the first N encounters\n"
               ,p->out
            );
          }
          break;
        }
      }
    }
    if( isOk==0 && iCtrl>=0 ){
      sqlite3_fprintf(p->out,
          "Usage: .testctrl %s %s\n", zCmd,aCtrl[iCtrl].zUsage);
      rc = 1;
    }else if( isOk==1 ){
      sqlite3_fprintf(p->out, "%d\n", rc2);
    }else if( isOk==2 ){
      sqlite3_fprintf(p->out, "0x%08x\n", rc2);
    }
  }else
#endif /* !defined(SQLITE_UNTESTABLE) */

  if( c=='t' && n>4 && cli_strncmp(azArg[0], "timeout", n)==0 ){
    open_db(p, 0);
    sqlite3_busy_timeout(p->db, nArg>=2 ? (int)integerValue(azArg[1]) : 0);
  }else

  if( c=='t' && n>=5 && cli_strncmp(azArg[0], "timer", n)==0 ){
    if( nArg==2 ){
      enableTimer = booleanValue(azArg[1]);
      if( enableTimer && !HAS_TIMER ){
        eputz("Error: timer not available on this system.\n");
        enableTimer = 0;
      }
    }else{
      eputz("Usage: .timer on|off\n");
      rc = 1;
    }
  }else

#ifndef SQLITE_OMIT_TRACE
  if( c=='t' && cli_strncmp(azArg[0], "trace", n)==0 ){
    int mType = 0;
    int jj;
    open_db(p, 0);
    for(jj=1; jj<nArg; jj++){
      const char *z = azArg[jj];
      if( z[0]=='-' ){
        if( optionMatch(z, "expanded") ){
          p->eTraceType = SHELL_TRACE_EXPANDED;
        }
#ifdef SQLITE_ENABLE_NORMALIZE
        else if( optionMatch(z, "normalized") ){
          p->eTraceType = SHELL_TRACE_NORMALIZED;
        }
#endif
        else if( optionMatch(z, "plain") ){
          p->eTraceType = SHELL_TRACE_PLAIN;
        }
        else if( optionMatch(z, "profile") ){
          mType |= SQLITE_TRACE_PROFILE;
        }
        else if( optionMatch(z, "row") ){
          mType |= SQLITE_TRACE_ROW;
        }
        else if( optionMatch(z, "stmt") ){
          mType |= SQLITE_TRACE_STMT;
        }
        else if( optionMatch(z, "close") ){
          mType |= SQLITE_TRACE_CLOSE;
        }
        else {
          sqlite3_fprintf(stderr,"Unknown option \"%s\" on \".trace\"\n", z);
          rc = 1;
          goto meta_command_exit;
        }
      }else{
        output_file_close(p->traceOut);
        p->traceOut = output_file_open(z);
      }
    }
    if( p->traceOut==0 ){
      sqlite3_trace_v2(p->db, 0, 0, 0);
    }else{
      if( mType==0 ) mType = SQLITE_TRACE_STMT;
      sqlite3_trace_v2(p->db, mType, sql_trace_callback, p);
    }
  }else
#endif /* !defined(SQLITE_OMIT_TRACE) */

#if defined(SQLITE_DEBUG) && !defined(SQLITE_OMIT_VIRTUALTABLE)
  if( c=='u' && cli_strncmp(azArg[0], "unmodule", n)==0 ){
    int ii;
    int lenOpt;
    char *zOpt;
    if( nArg<2 ){
      eputz("Usage: .unmodule [--allexcept] NAME ...\n");
      rc = 1;
      goto meta_command_exit;
    }
    open_db(p, 0);
    zOpt = azArg[1];
    if( zOpt[0]=='-' && zOpt[1]=='-' && zOpt[2]!=0 ) zOpt++;
    lenOpt = (int)strlen(zOpt);
    if( lenOpt>=3 && cli_strncmp(zOpt, "-allexcept",lenOpt)==0 ){
      assert( azArg[nArg]==0 );
      sqlite3_drop_modules(p->db, nArg>2 ? (const char**)(azArg+2) : 0);
    }else{
      for(ii=1; ii<nArg; ii++){
        sqlite3_create_module(p->db, azArg[ii], 0, 0);
      }
    }
  }else
#endif

  if( c=='v' && cli_strncmp(azArg[0], "version", n)==0 ){
    char *zPtrSz = sizeof(void*)==8 ? "64-bit" : "32-bit";
    sqlite3_fprintf(p->out, "SQLite %s %s\n" /*extra-version-info*/,
          sqlite3_libversion(), sqlite3_sourceid());
#if SQLITE_HAVE_ZLIB
    sqlite3_fprintf(p->out, "zlib version %s\n", zlibVersion());
#endif
#define CTIMEOPT_VAL_(opt) #opt
#define CTIMEOPT_VAL(opt) CTIMEOPT_VAL_(opt)
#if defined(__clang__) && defined(__clang_major__)
    sqlite3_fprintf(p->out, "clang-" CTIMEOPT_VAL(__clang_major__) "."
          CTIMEOPT_VAL(__clang_minor__) "."
          CTIMEOPT_VAL(__clang_patchlevel__) " (%s)\n", zPtrSz);
#elif defined(_MSC_VER)
    sqlite3_fprintf(p->out, "msvc-" CTIMEOPT_VAL(_MSC_VER) " (%s)\n", zPtrSz);
#elif defined(__GNUC__) && defined(__VERSION__)
    sqlite3_fprintf(p->out, "gcc-" __VERSION__ " (%s)\n", zPtrSz);
#endif
  }else

  if( c=='v' && cli_strncmp(azArg[0], "vfsinfo", n)==0 ){
    const char *zDbName = nArg==2 ? azArg[1] : "main";
    sqlite3_vfs *pVfs = 0;
    if( p->db ){
      sqlite3_file_control(p->db, zDbName, SQLITE_FCNTL_VFS_POINTER, &pVfs);
      if( pVfs ){
        sqlite3_fprintf(p->out, "vfs.zName      = \"%s\"\n", pVfs->zName);
        sqlite3_fprintf(p->out, "vfs.iVersion   = %d\n", pVfs->iVersion);
        sqlite3_fprintf(p->out, "vfs.szOsFile   = %d\n", pVfs->szOsFile);
        sqlite3_fprintf(p->out, "vfs.mxPathname = %d\n", pVfs->mxPathname);
      }
    }
  }else

  if( c=='v' && cli_strncmp(azArg[0], "vfslist", n)==0 ){
    sqlite3_vfs *pVfs;
    sqlite3_vfs *pCurrent = 0;
    if( p->db ){
      sqlite3_file_control(p->db, "main", SQLITE_FCNTL_VFS_POINTER, &pCurrent);
    }
    for(pVfs=sqlite3_vfs_find(0); pVfs; pVfs=pVfs->pNext){
      sqlite3_fprintf(p->out, "vfs.zName      = \"%s\"%s\n", pVfs->zName,
            pVfs==pCurrent ? "  <--- CURRENT" : "");
      sqlite3_fprintf(p->out, "vfs.iVersion   = %d\n", pVfs->iVersion);
      sqlite3_fprintf(p->out, "vfs.szOsFile   = %d\n", pVfs->szOsFile);
      sqlite3_fprintf(p->out, "vfs.mxPathname = %d\n", pVfs->mxPathname);
      if( pVfs->pNext ){
        sqlite3_fputs("-----------------------------------\n", p->out);
      }
    }
  }else

  if( c=='v' && cli_strncmp(azArg[0], "vfsname", n)==0 ){
    const char *zDbName = nArg==2 ? azArg[1] : "main";
    char *zVfsName = 0;
    if( p->db ){
      sqlite3_file_control(p->db, zDbName, SQLITE_FCNTL_VFSNAME, &zVfsName);
      if( zVfsName ){
        sqlite3_fprintf(p->out, "%s\n", zVfsName);
        sqlite3_free(zVfsName);
      }
    }
  }else

  if( c=='w' && cli_strncmp(azArg[0], "wheretrace", n)==0 ){
    unsigned int x = nArg>=2? (unsigned int)integerValue(azArg[1]) : 0xffffffff;
    sqlite3_test_control(SQLITE_TESTCTRL_TRACEFLAGS, 3, &x);
  }else

  if( c=='w' && cli_strncmp(azArg[0], "width", n)==0 ){
    int j;
    assert( nArg<=ArraySize(azArg) );
    p->nWidth = nArg-1;
    p->colWidth = realloc(p->colWidth, (p->nWidth+1)*sizeof(int)*2);
    if( p->colWidth==0 && p->nWidth>0 ) shell_out_of_memory();
    if( p->nWidth ) p->actualWidth = &p->colWidth[p->nWidth];
    for(j=1; j<nArg; j++){
      p->colWidth[j-1] = (int)integerValue(azArg[j]);
    }
  }else

  {
    sqlite3_fprintf(stderr,"Error: unknown command or invalid arguments: "
          " \"%s\". Enter \".help\" for help\n", azArg[0]);
    rc = 1;
  }

meta_command_exit:
  if( p->outCount ){
    p->outCount--;
    if( p->outCount==0 ) output_reset(p);
  }
  p->bSafeMode = p->bSafeModePersist;
  return rc;
}
