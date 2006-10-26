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

#include <atlbase.h>
#include <dshow.h>
#include <comutil.h>
#include <qedit.h> // SampleGrabber

#ifndef _DSHOWCAPTURE_H_
#define _DSHOWCAPTURE_H_

#define DEVICE_LIST_MAX 10 // There can be numb. max devices

class DShowCapture
{
private:

	// Some com stuff
	CComPtr<IGraphBuilder> pGraph;
	CComPtr<ICaptureGraphBuilder2> pBuilder;
	CComPtr<IBaseFilter> pSrc;

	IBaseFilter *pSampleGrabberFilter;
	ISampleGrabber *pSampleGrabber; // SAmpleGrabber

	VIDEOINFOHEADER *pVideoInfoHeader; // Video Info Header

	AM_MEDIA_TYPE am_media_type;

	// to the devices
	int nDevices;
	char *pDeviceList[ DEVICE_LIST_MAX ]; // list of the devices

	// private methods
	void InitDeviceList( void );
	void FreeDeviceList( void );

	long width;
	long height;
	int bitcount;

public:

	DShowCapture( void );
	~DShowCapture( void );

	HRESULT Init( void );
	HRESULT PostInit( long *pnBufferSize );

	int GetNumDevices( void );
	char *GetDeviceName( int nDevNumber );
	HRESULT SetDevice( const char *name );

	long GetWidth( void );
	long GetHeight( void );
	long GetBitCount( void );

	HRESULT Start( void );
	HRESULT Stop( void );

	void GetBuffer( long *pnMyBufferSize, long **ppMyBuffer );
	void SaveBufferToBmp( const char *pName, long nBufferSize, long *pBuffer );

};

#endif
