/* 
 * Copyright (C) 2004 Georgy Yunaev tim@krasnogorsk.ru
 *
 * This library is free software; you can redistribute it and/or modify it 
 * under the terms of the GNU Lesser General Public License as published by 
 * the Free Software Foundation; either version 2 of the License, or (at your 
 * option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT 
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or 
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public 
 * License for more details.
 *
 * $Id$
 */


#if !defined (WIN32)
	#include "config.h"
	#include <stdio.h>
	#include <stdarg.h>
	#include <unistd.h>
	#include <string.h>
	#include <stdlib.h>
	#include <sys/stat.h>
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netdb.h>
	#include <arpa/inet.h>	
	#include <netinet/in.h>
	#include <fcntl.h>
	#include <errno.h>
	#include <ctype.h>
	#include <time.h>

	typedef int					socket_t;
	#define closesocket 		close
	#define	GetSocketError()	errno

	#if defined (ENABLE_THREADS)
		#include <pthread.h>
		typedef pthread_mutex_t		port_mutex_t;

		#if !defined (PTHREAD_MUTEX_RECURSIVE)
			#define PTHREAD_MUTEX_RECURSIVE	PTHREAD_MUTEX_RECURSIVE_NP
		#endif
	#endif 
#else
	#include <windows.h>
	#include <winsock.h>
	#include <time.h>
	#include <stdio.h>
	#include <stdarg.h>
	#include <string.h>
	#include <stdlib.h>
	#include <sys/stat.h>

	#if defined (ENABLE_THREADS)
		typedef CRITICAL_SECTION	port_mutex_t;
	#endif

	#define inline
	#define snprintf			_snprintf
	#define vsnprintf			_vsnprintf
	#define GetSocketError()	WSAGetLastError()

	#define EWOULDBLOCK			WSAEWOULDBLOCK
	#define EINPROGRESS			WSAEINPROGRESS
	#define EINTR				WSAEINTR

	typedef unsigned int	socklen_t;
	typedef SOCKET			socket_t;

#endif

#ifndef INADDR_NONE
	#define INADDR_NONE 	0xFFFFFFFF
#endif


#if defined (ENABLE_THREADS)


static inline int libirc_mutex_init (port_mutex_t * mutex)
{
#if defined (WIN32)
	InitializeCriticalSection (mutex);
	return 0;
#else
	pthread_mutexattr_t	attr;

	return (pthread_mutexattr_init (&attr)
		|| pthread_mutexattr_settype (&attr, PTHREAD_MUTEX_RECURSIVE)
		|| pthread_mutex_init (mutex, &attr));
#endif
}


static inline void libirc_mutex_destroy (port_mutex_t * mutex)
{
#if defined (WIN32)
	DeleteCriticalSection (mutex);
#else
	pthread_mutex_destroy (mutex);
#endif
}


static inline void libirc_mutex_lock (port_mutex_t * mutex)
{
#if defined (WIN32)
	EnterCriticalSection (mutex);
#else
	pthread_mutex_lock (mutex);
#endif
}


static inline void libirc_mutex_unlock (port_mutex_t * mutex)
{
#if defined (WIN32)
	LeaveCriticalSection (mutex);
#else
	pthread_mutex_unlock (mutex);
#endif
}

#else

	typedef void *	port_mutex_t;

static inline int libirc_mutex_init (port_mutex_t * mutex) { return 0; }
static inline void libirc_mutex_destroy (port_mutex_t * mutex) {}
static inline void libirc_mutex_lock (port_mutex_t * mutex) {}
static inline void libirc_mutex_unlock (port_mutex_t * mutex) {}

#endif


static int libirc_make_socket_unblocking (int sock)
{
#if !defined (WIN32)
	return fcntl (sock, F_SETFL, fcntl (sock, F_GETFL,0 ) | O_NONBLOCK) != 0;
#else
	unsigned long mode = 0;
	return ioctlsocket (sock, FIONBIO, &mode) == SOCKET_ERROR;
#endif
}


/*
 * Stub for WIN32 dll to initialize winsock API
 */
#if defined (WIN32)
BOOL WINAPI DllMain (HINSTANCE hinstDll, DWORD fdwReason, LPVOID lpvReserved)
{
	WORD wVersionRequested = MAKEWORD (1, 1);
    WSADATA wsaData;

	switch(fdwReason)
	{
		case DLL_PROCESS_ATTACH:
			if ( WSAStartup (wVersionRequested, &wsaData) != 0 )
				return FALSE;

			DisableThreadLibraryCalls (hinstDll);
			break;

		case DLL_PROCESS_DETACH:
			WSACleanup();
			break;
	}

	return TRUE;
}
#endif