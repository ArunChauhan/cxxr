/*CXXR $Id$
 *CXXR
 *CXXR This file is part of CXXR, a project to refactor the R interpreter
 *CXXR into C++.  It may consist in whole or in part of program code and
 *CXXR documentation taken from the R project itself, incorporated into
 *CXXR CXXR (and possibly MODIFIED) under the terms of the GNU General Public
 *CXXR Licence.
 *CXXR 
 *CXXR CXXR is Copyright (C) 2008-9 Andrew R. Runnalls, subject to such other
 *CXXR copyrights and copyright restrictions as may be stated below.
 *CXXR 
 *CXXR CXXR is not part of the R project, and bugs and other issues should
 *CXXR not be reported via r-bugs or other R project channels; instead refer
 *CXXR to the CXXR website.
 *CXXR */

/*
 *  R : A Computer Language for Statistical Data Analysis
 *  Copyright (C)  2001-5   The R Development Core Team.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, a copy is available at
 *  http://www.r-project.org/Licenses/
 */

/* <UTF8> chars are only handled as a whole */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_SOCKETS


/* ------------------- socket connections  --------------------- */

#include <Defn.h>
#include <Rconnections.h>
#include <R_ext/R-ftp-http.h>
#include "sock.h"
#include <errno.h>

static void listencleanup(void *data)
{
    int *psock = static_cast<int*>(data);
    R_SockClose(*psock);
}

static Rboolean sock_open(Rconnection con)
{
    Rsockconn thisconn = (Rsockconn)con->connprivate;
    int sock, sock1, mlen;
    int timeout = asInteger(GetOption(install("timeout"), R_BaseEnv));
    char buf[256];

    if(timeout == NA_INTEGER || timeout <= 0) timeout = 60;
    R_SockTimeout(timeout);
    thisconn->pend = thisconn->pstart = thisconn->inbuf;

    if(thisconn->server) {
	sock1 = R_SockOpen(thisconn->port);
	if(sock1 < 0) {
	    warning("port %d cannot be opened", thisconn->port);
	    return FALSE;
	}
	{
	    RCNTXT cntxt;

	    /* set up a context which will close socket on jump. */
	    begincontext(&cntxt, CTXT_CCODE, R_NilValue, R_BaseEnv,
			 R_BaseEnv, R_NilValue, R_NilValue);
	    cntxt.cend = &listencleanup;
	    cntxt.cenddata = &sock1;
	    sock = R_SockListen(sock1, buf, 256);
	    endcontext(&cntxt);
	}
	if(sock < 0) {
	    warning("problem in listening on this socket");
	    R_SockClose(sock1);
	    return FALSE;
	}
	free(con->description);
	con->description = (char *) malloc(strlen(buf) + 10);
	sprintf(con->description, "<-%s:%d", buf, thisconn->port);
	R_SockClose(sock1);
    } else {
	sock = R_SockConnect(thisconn->port, con->description);
	if(sock < 0) {
	    warning("%s:%d cannot be opened", con->description, thisconn->port);
	    return FALSE;
	}
	sprintf(buf, "->%s:%d", con->description, thisconn->port);
	strcpy(con->description, buf);
    }
    thisconn->fd = sock;

    mlen = strlen(con->mode);
    con->isopen = TRUE;
    if(mlen >= 2 && con->mode[mlen - 1] == 'b') con->text = FALSE;
    else con->text = TRUE;
    set_iconv(con); /* OK for output, at least */
    con->save = -1000;
    return TRUE;
}

static void sock_close(Rconnection con)
{
    Rsockconn thisconn = (Rsockconn)con->connprivate;
    R_SockClose(thisconn->fd);
    con->isopen = FALSE;
}

static int sock_read_helper(Rconnection con, void *ptr, size_t size)
{
    Rsockconn thisconn = (Rsockconn)con->connprivate;
    int res;
    int nread = 0, n;

    do {
	/* read data into the buffer if it's empty and size > 0 */
	if (size > 0 && thisconn->pstart == thisconn->pend) {
	    thisconn->pstart = thisconn->pend = thisconn->inbuf;
	    do
		res = R_SockRead(thisconn->fd, thisconn->inbuf, 4096, con->blocking);
	    while (-res == EINTR);
	    if (! con->blocking && -res == EAGAIN) {
		con->incomplete = TRUE;
		return nread;
	    }
	    else if (con->blocking && res == 0) /* should mean EOF */
		return nread;
	    else if (res < 0) return res;
	    else thisconn->pend = thisconn->inbuf + res;
	}

	/* copy data from buffer to ptr */
	if (thisconn->pstart + size <= thisconn->pend)
	    n = size;
	else
	    n = thisconn->pend - thisconn->pstart;
	memcpy(ptr, thisconn->pstart, n);
	ptr = ((char *) ptr) + n;
	thisconn->pstart += n;
	size -= n;
	nread += n;
    } while (size > 0);

    con->incomplete = FALSE;
    return nread;
}


static int sock_fgetc_internal(Rconnection con)
{
    unsigned char c;
    int n;

    n = sock_read_helper(con, (char *)&c, 1);
    return (n == 1) ? c : R_EOF;
}

static size_t sock_read(void *ptr, size_t size, size_t nitems,
			Rconnection con)
{
    return sock_read_helper(con, ptr, size * nitems)/size;
}

static size_t sock_write(const void *ptr, size_t size, size_t nitems,
			 Rconnection con)
{
    Rsockconn thisconn = (Rsockconn)con->connprivate;

    return R_SockWrite(thisconn->fd, ptr, size * nitems)/size;
}

Rconnection in_R_newsock(const char *host, int port, int server,
			 const char * const mode)
{
    Rconnection newconn;

    newconn = (Rconnection) malloc(sizeof(struct Rconn));
    if(!newconn) error(_("allocation of socket connection failed"));
    newconn->connclass = (char *) malloc(strlen("sockconn") + 1);
    if(!newconn->connclass) {
	free(newconn);
	error(_("allocation of socket connection failed"));
    }
    strcpy(newconn->connclass, "sockconn");
    newconn->description = (char *) malloc(strlen(host) + 10);
    if(!newconn->description) {
	free(newconn->connclass); free(newconn);
	error(_("allocation of socket connection failed"));
    }
    init_con(newconn, host, CE_NATIVE, mode);
    newconn->open = &sock_open;
    newconn->close = &sock_close;
    newconn->vfprintf = &dummy_vfprintf;
    newconn->fgetc_internal = &sock_fgetc_internal;
    newconn->fgetc = &dummy_fgetc;
    newconn->read = &sock_read;
    newconn->write = &sock_write;
    newconn->connprivate = (void *) malloc(sizeof(struct sockconn));
    if(!newconn->connprivate) {
	free(newconn->description); free(newconn->connclass); free(newconn);
	error(_("allocation of socket connection failed"));
    }
    ((Rsockconn)newconn->connprivate)-> port = port;
    ((Rsockconn)newconn->connprivate)-> server = server;
    return newconn;
}

#endif /* HAVE_SOCKETS */
