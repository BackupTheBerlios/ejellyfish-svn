/* libdscapture
 * Copyright (C) <2006> Movial OY
 * Author: joni.valtanen@movial.fi
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <stdio.h>
#include <stdlib.h>

#include <windows.h>

#include "dscapture.h"

int main( int argc, char **argv )
{
	int retval = 0;
	int i = 0;
	int devices = 0;

	char *devname = NULL;

	long nMyBufferSize = 0;
	long *pMyBuffer = NULL;

	if( argc != 2 ) {
		printf( "Usage: %s picname.bmp\n", argv[0] );
		return 0;
	}
	printf("\n\n");

	printf( "Init device\n");
	retval = dscapture_init( ); //&pMyBuffer, &nMyBufferSize );
	if( retval > 0 ) {
		printf( "Could not init device\n");
		exit(2);
	}

	printf( "Get number of capture devices!!!\n");
	devices = dscapture_numdevices( );
	if( devices < 1 ) {
		printf( "There is no capture devices!!!\n");
		dscapture_uninit( );
		exit(3);
	}

	printf( "Print all capture capable devices!!!\n");
	for( i = 0; i < devices; i++ ) {
		devname = _strdup( dscapture_getdevicename( i ) );
		printf("Name: %s\n", devname);
		free( devname );
	}

	devname = _strdup( dscapture_getdevicename( 0 ) );

	printf( "Set capturedevice!!!\n");
	nMyBufferSize = dscapture_setdevice( devname );
	if( retval < 0 ) {
		printf( "Can not set capturedevice!!!\n");
		dscapture_uninit( );
		exit(5);
	}

	printf( "Alloc memory to buffer!!!\n");
	pMyBuffer = malloc( nMyBufferSize );
	if( pMyBuffer == NULL ) {
		printf( "Can not alloc memory to buffer!!!\n");
		dscapture_uninit();
		exit(6);
	}

	printf( "Start capturing!!!\n");
	retval = dscapture_start( );
	if( retval > 0 ) {
		printf( "Can not start capturing!!!\n");
		dscapture_uninit( );
		exit(7);
	}

	Sleep( 10000 );
	dscapture_getbuffer( nMyBufferSize, pMyBuffer );
	printf( "Buffer: 0x%x, size: %lu\n", pMyBuffer, nMyBufferSize );
	printf( "Buffer information:\n %s\n", pMyBuffer );

	printf( "Saving picture: %s\n", argv[ 1 ] );
	dscapture_buffertobmp( nMyBufferSize, pMyBuffer, argv[ 1 ] );

	retval = dscapture_stop( );

	dscapture_uninit( );
	
	free( ( void * )pMyBuffer );
	free( devname );

	printf("\n\n");

    return 0;

}
