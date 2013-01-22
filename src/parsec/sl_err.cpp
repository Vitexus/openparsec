/*
 * PARSEC - Error Handling Code
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/15 12:25:33 $
 *
 * Orginally written by:
 *   Copyright (c) Markus Hadwiger     <msh@parsec.org>   1996-1999
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
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */ 
// compilation flags/debug support
#include "config.h"


// C library
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "debug.h"

// general definitions
#include "general.h"
#include "objstruc.h"

// global externals
#include "globals.h"

// subsystem headers
#include "aud_defs.h"
#include "inp_defs.h"
#include "net_defs.h"
#include "sys_defs.h"
#include "vid_defs.h"

// local module header
#include "inp_init.h"
#include "sl_err.h"



// SL_CleanUp() must be provided externally (currently by SL_MAIN.C) ----------
//
extern void SL_CleanUp();
#define CLEAN_UP(x) if ( init_done ) SL_CleanUp(); else {}


// flag if init already done --------------------------------------------------
//
static int init_done = FALSE;


// clean up functions that must be called in any case (if at all possible) ----
//
void SYSs_CriticalCleanUp()
{
	//NOTE:
	// this function is not declared in any header,
	// since it is only used by DEBUG.C, which
	// contains a prototype in order to call
	// SYSs_CriticalCleanUp().

	int static was_here = FALSE;

	if ( !was_here ) {
		was_here = TRUE;

		// kill networking system;
		// must not be done after SYSs_KillFrameTimer()
		NETs_KillAPI();

		// kill input code
		INP_KillSubsystem();

		// switch back to text mode
		VIDs_RestoreDisplay();

		// kill sound system
		AUDs_KillSoundSys();

		// kill timer code
		SYSs_KillFrameTimer();
	}
}


// common error strings -------------------------------------------------------
//
static char err_exit[] = "Parsec error exit";
static char err_emem[] = "Parsec memory error exit";
static char err_ndef[] = "Unrecoverable error";
static char err_nmem[] = "Memory allocation failed";
static char err_msg1[] = "\n%s: %s";
static char err_msg2[] = "\n%s: %s (%s, line %u).";


// exit function if critical error occurred -----------------------------------
//
void SYSs_PanicFunction( const char *msg, const char *file, unsigned line )
{
	// switch back to text mode
	VIDs_RestoreDisplay();

	// print error message
	if ( ( msg != NULL ) && ( *msg != 0 ) ) {

		// print custom message if provided and not empty
		fprintf( stderr, err_msg1, err_exit, msg );

	} else if ( ( msg != NULL ) && ( errno > 0 ) ) {

		// print errno error if provided message is empty
		fprintf( stderr, err_msg2, err_exit, strerror( errno ), file, line );

	} else {

		// print generic error message if no message provided
		fprintf( stderr, err_msg2, err_exit, err_ndef, file, line );
	}
	fprintf( stderr, "\n" );

	// just to be sure
	fflush( stderr );

	CLEAN_UP(0);
	exit( EXIT_FAILURE );
}


// exit function if memory allocation failed ----------------------------------
//
void SYSs_OutOfMemFunction( const char *msg, const char *file, unsigned line )
{
	// switch back to text mode
	VIDs_RestoreDisplay();

	// print error message
	if ( ( msg != NULL ) && ( *msg != 0 ) ) {

		// print custom message if provided and not empty
		fprintf( stderr, err_msg1, err_emem, msg );

	} else if ( ( msg != NULL ) && ( errno > 0 ) ) {

		// print errno error if provided message is empty
		fprintf( stderr, err_msg2, err_emem, strerror( errno ), file, line );

	} else {

		// print generic error message if no message provided
		fprintf( stderr, err_msg2, err_emem, err_nmem, file, line );
	}
	fprintf( stderr, "\n" );

	// just to be sure
	fflush( stderr );

	CLEAN_UP(0);
	exit( EXIT_FAILURE );
}


// prints error message and exits ---------------------------------------------
//
void SYSs_PError( const char *message, ... )
{
	va_list	ap;
	va_start( ap, message );

	// switch back to text mode
	VIDs_RestoreDisplay();

	// print custom message if provided
	if ( message != NULL ) {
		fprintf ( stderr, "\n%s: ", sys_ProgramName );
		vfprintf( stderr, message, ap );
		fprintf ( stderr, "\n" );
	} else {
		fprintf( stderr, "\n%s: SYSs_PError().\n", sys_ProgramName );
	}
	fflush( stderr );

	va_end( ap );

	CLEAN_UP(0);
	exit( EXIT_FAILURE );
}


// prints error message and string for errno and exits ------------------------
//
void SYSs_SError( const char *message )
{
	// switch back to text mode
	VIDs_RestoreDisplay();

	// print custom message if provided
	if ( message != NULL ) {
		fprintf( stderr, "\n%s: ", sys_ProgramName );
		fprintf( stderr, "%s", message );
		fprintf( stderr, ": %s\n", strerror( errno ) );
	} else {
		fprintf( stderr, "\n%s: SYSs_SError().\n", sys_ProgramName );
	}
	fflush( stderr );

	CLEAN_UP(0);
	exit( EXIT_FAILURE );
}


// prints error message, filename, string for errno, and exits ----------------
//
void SYSs_FError( const char *message, const char *filename )
{
	// switch back to text mode
	VIDs_RestoreDisplay();

	// print custom message if provided
	if ( ( message != NULL ) && ( filename != NULL ) ) {
		fprintf( stderr, "\n%s: ", sys_ProgramName );
		fprintf( stderr, message, filename );
		fprintf( stderr, ": %s\n", strerror( errno ) );
	} else {
		fprintf( stderr, "\n%s: SYSs_FError().\n", sys_ProgramName );
	}
	fflush( stderr );

	CLEAN_UP(0);
	exit( EXIT_FAILURE );
}


// init error handling code ---------------------------------------------------
//
void SYSs_InitErrorHandling()
{
	ASSERT( !init_done );

	if ( !init_done ) {
		init_done = TRUE;
	}
}



