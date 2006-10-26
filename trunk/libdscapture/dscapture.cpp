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

#include "dscapture.h"
#include "DShowCapture.h"

/* FIXME: set list of DShowCapture objects */
static DShowCapture *capt;


/*
	int dscapture_init( void )

	inits capture

	returns 0 if success else > 0
*/
int dscapture_init( void )
{
	HRESULT hr;

	CoInitialize( NULL );

	printf("Create\n");
	capt = NULL;
	capt = new DShowCapture(); // create new capture

	printf("Init\n");

	hr = capt->Init( ); //ppBuffer, pnBufferSize );
	if( FAILED( hr ) ) {
		printf("Failed : Init\n");
		delete capt;
		return 1;
	}

	return 0;
}

/*
    int dscapture_numdevices( void )

	returns number of connected devices
*/
int dscapture_numdevices( void )
{
	int numdev = capt->GetNumDevices();
	printf( "Number of devices: %d\n", numdev );

	if( numdev < 1 ) {
		printf("No devices available!!!\n"); 
		return 0;
	}

	return numdev;
}

/*

	Gets device name with given number

	params:
		@devnumber - device number to get name

	side effect none here buf user should duplicate given string

	returns pointer to char created with strdup() 
*/
char *dscapture_getdevicename( int devnumber )
{
	return capt->GetDeviceName( 0 );
}


/*
	Activates device and inits buffers

	params:
		@devname - device name to activate

	returns:
		@nBufferSize - returns buffer size

*/
long dscapture_setdevice( char *devname )
{
	HRESULT hr;
	long nBufferSize;

	capt->SetDevice( devname );

	hr = capt->PostInit( &nBufferSize );
	if( FAILED( hr ) ) {
		return -1;
	}

	return nBufferSize;
}

/*
	struct videoinfo get_caps( struct videoinfo *vinfo )
	gets video capabilities

	params:
		@vinfo - pointer to DSCapture:s own preallocated videoinfo structure

	if there is no videoinfoheader videoinfo struct is NULL

*/
void dscapture_getcaps( struct videoinfo *vinfo )
{
	/* Get capabilities */
	vinfo->width = capt->GetWidth();
	vinfo->height = capt->GetHeight();
	vinfo->bitcount = capt->GetBitCount();
}

/*
    int dscapture_start( void )
	Starts capturing

	returns 0 on success else 1
*/
int dscapture_start( void )
{
	HRESULT hr;

	hr = capt->Start();
	if( FAILED( hr ) ) {
		return 1;
	}

	return 0;
}

/*
	int dscapture_stop( void )
	Stops capturing

	returns 0 on success else 1
*/
int dscapture_stop( void )
{
	HRESULT hr;

	hr = capt->Stop();
	if( FAILED( hr ) ) {
		return 1;
	}

	return 0;
}

/*
	Fills buffer with capture data

	params:
		@nMyBufferSize - buffer size
		@pMyBuffer - pointer to preallocated buffer
*/
void dscapture_getbuffer( long nMyBufferSize, long *pMyBuffer )
{
	capt->GetBuffer( &nMyBufferSize, &pMyBuffer );
}


/*
	Saves buffer to bmp picture

	params:
		@nMyBufferSize - buffer size
		@pMyBuffer - pointer to preallocated buffer

*/
void dscapture_buffertobmp( long nMyBufferSize, long *pMyBuffer, const char *name )
{
	capt->SaveBufferToBmp( name , nMyBufferSize, pMyBuffer );
}


/*
	Uninits capturing
*/
void dscapture_uninit( void )
{
	//printf("Delete\n");
	delete capt;

	CoUninitialize(); // This have to do here
}

/* Some nessessary Dll stuff */
BOOL WINAPI DllMain(
    HINSTANCE hinstDLL,  // handle to DLL module
    DWORD fdwReason,     // reason for calling function
    LPVOID lpReserved )  // reserved
{
    // Perform actions based on the reason for calling.
    switch( fdwReason ) 
    { 
        case DLL_PROCESS_ATTACH:
         // Initialize once for each new process.
         // Return FALSE to fail DLL load.
            break;

        case DLL_THREAD_ATTACH:
         // Do thread-specific initialization.
            break;

        case DLL_THREAD_DETACH:
         // Do thread-specific cleanup.
            break;

        case DLL_PROCESS_DETACH:
         // Perform any necessary cleanup.
            break;
    }
    return TRUE;  // Successful DLL_PROCESS_ATTACH.
}