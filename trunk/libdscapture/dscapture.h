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

/* 
	Do not put here anything that does not work with C!!!!
*/

#ifndef _DSCAPTURE_H_
#define _DSCAPTURE_H_

#ifdef __cplusplus
extern "C" {
#endif

struct videoinfo
{
	long width;
	long height;
	int bitcount;
};

#define	DLLEXPORT __declspec(dllexport)

DLLEXPORT int dscapture_init( void );
DLLEXPORT int dscapture_numdevices( void );
DLLEXPORT char *dscapture_getdevicename( int devnumber );
DLLEXPORT long dscapture_setdevice( char *devname );
DLLEXPORT void dscapture_getcaps( struct videoinfo *vinfo );
DLLEXPORT int dscapture_start( void );
DLLEXPORT int dscapture_stop( void );
DLLEXPORT void dscapture_getbuffer( long nMyBufferSize, long *pMyBuffer );
DLLEXPORT void dscapture_buffertobmp( long nMyBufferSize, long *pMyBuffer, const char *name );
DLLEXPORT void dscapture_uninit( void );

#ifdef __cplusplus
}
#endif


#endif

/* 
	Do not put here anything that does not work with C!!!!
*/
