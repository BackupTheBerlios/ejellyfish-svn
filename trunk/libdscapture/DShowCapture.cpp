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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <atlbase.h>
#include <dshow.h>
#include <comutil.h>

#include <qedit.h> // SampleGrabber

#include <windows.h>

#include "DShowCapture.h"

#define TMPC_SIZE 2048

/*
	Constructor

	Init class vars
*/
DShowCapture::DShowCapture( void )
{
	int i = 0;

	pSampleGrabber = NULL;
	pSampleGrabberFilter = NULL;
	pVideoInfoHeader = NULL;
	nDevices = 0;

	for( i=0; i < DEVICE_LIST_MAX; i++ )
		pDeviceList[ i ] = NULL;

}

/*
	Destructor

	deinit class vars

*/
DShowCapture::~DShowCapture( void )
{
	FreeDeviceList();
}

/*
	void DShowCapture::FreeDeviceList( void )

	free list of devices

*/
void DShowCapture::FreeDeviceList( void )
{
	int i = 0;
	
	for( i=0; i < DEVICE_LIST_MAX; i++ ) {
		if( pDeviceList[ i ] != NULL )
			free( pDeviceList[ i ] );
		pDeviceList[ i ] = NULL;
	}
}


/*
	void DShowCapture::InitDeviceList( void )

	Gets available devices

*/
void DShowCapture::InitDeviceList( void )
{
    HRESULT hr = E_FAIL;

    CComPtr<IBaseFilter> pFilter;
    CComPtr<ICreateDevEnum> pSysDevEnum;
    CComPtr<IEnumMoniker> pEnumCat = NULL;

	int i = 0;
	int len = 0;
	char tmpc[ TMPC_SIZE ];

	FreeDeviceList(  ); // free list if any

    // Create the System Device Enumerator.
    pSysDevEnum.CoCreateInstance(CLSID_SystemDeviceEnum);

    // Obtain a class enumerator for the video compressor category.
    pSysDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnumCat, 0);
    
    // Enumerate the monikers.
    CComPtr<IMoniker> pMoniker;
    ULONG cFetched;
    while(pEnumCat->Next(1, &pMoniker, &cFetched) == S_OK)
    {
        CComPtr<IPropertyBag> pProp;
        pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pProp);
        VARIANT varName;
        VariantInit(&varName); // Try to match the friendly name.
        hr = pProp->Read(L"FriendlyName", &varName, 0); 
        if ( SUCCEEDED(hr) )
        {
			WideCharToMultiByte(CP_ACP,0,varName.bstrVal, -1, tmpc, TMPC_SIZE, NULL, NULL);
			pDeviceList[ i++ ] = _strdup( tmpc );
        }
        VariantClear(&varName);
        pMoniker = NULL; // Release for the next loop.
    }

	nDevices = i;
}

/*
	int DShowCapture::GetNumDevices( void )

	returns number of devices

*/
int DShowCapture::GetNumDevices( void )
{
	return nDevices;
}

/*
	char *DShowCapture::GetDeviceName( int nDevNumber )
	
	params:
		@nDevNumber - device number to get name ( first is 0 )

	returns given devicenumber name

*/
char *DShowCapture::GetDeviceName( int nDevNumber )
{
	return pDeviceList[ nDevNumber ];
}

/*
	HRESULT Init( void )
	Inits capture. 
	
	After this it should select capture device.

	returns success of the initializing

*/
HRESULT DShowCapture::Init(	void ) // ( long **ppBuffer, long *pnBufferSize )
{
    HRESULT hr;

	hr = CoCreateInstance(CLSID_SampleGrabber,
		NULL,
		CLSCTX_INPROC,
		IID_IBaseFilter,
		( LPVOID *)&pSampleGrabberFilter);
	if( FAILED( hr ) ) {
		return hr;
	}

	hr = pSampleGrabberFilter->QueryInterface(IID_ISampleGrabber,
	 (LPVOID *)&pSampleGrabber);
	if( FAILED( hr ) ) {
		return hr;
	}

    // Create the filter graph.
    hr = pGraph.CoCreateInstance(CLSID_FilterGraph);
	if( FAILED(hr) ) {
		return hr;
	}
    
    // Create the capture graph builder.
    hr = pBuilder.CoCreateInstance(CLSID_CaptureGraphBuilder2);
	if( FAILED(hr) ) {
		return hr;
	}

	pBuilder->SetFiltergraph(pGraph);

	// SampleGrapper Stuff
	ZeroMemory(&am_media_type, sizeof(am_media_type));
	am_media_type.majortype = MEDIATYPE_Video;
	am_media_type.subtype = MEDIASUBTYPE_RGB24;
	am_media_type.formattype = FORMAT_VideoInfo;
	pSampleGrabber->SetMediaType(&am_media_type);

	InitDeviceList( ); // get available devices to the list

	return S_OK;

}

/*
	HRESULT DShowCapture::PostInit(	long **ppBuffer, long *pnBufferSize )

	Inits capture. 
	Use this after selecting capture device.

	params:
	  @pBufferSize - pointer to buffer size

    returns success of the adding filters

*/
HRESULT DShowCapture::PostInit( long *pnBufferSize )
{
    HRESULT hr;

	// Adds Filters to Graph
    hr = pGraph->AddFilter(pSrc, L"Capture");
    hr = pGraph->AddFilter(pSampleGrabberFilter, L"Grabber");

	/* Renders stream */
    hr = pBuilder->RenderStream(
	    &PIN_CATEGORY_CAPTURE,  // Pin category
        &MEDIATYPE_Video,       // Media type
        pSrc,                   // Capture filter
        NULL,                   // Compression filter (optional)
	    pSampleGrabberFilter
    );
	if( FAILED(hr) ) {
		return hr;
	}

	// SampleGrapper again
	pSampleGrabber->GetConnectedMediaType(&am_media_type);

	pVideoInfoHeader = (VIDEOINFOHEADER *)am_media_type.pbFormat;

	bitcount = pVideoInfoHeader->bmiHeader.biBitCount;
	height = pVideoInfoHeader->bmiHeader.biHeight;
	width = pVideoInfoHeader->bmiHeader.biWidth;

	pSampleGrabber->SetBufferSamples(TRUE);

	*pnBufferSize = am_media_type.lSampleSize;
	
	return S_OK;
}


/*
	HRESULT DShowCapture::SetDevice( const char *pName )
	Sets device to use by name

	Remember use post init after this!!!!

	params:
		@pName - exact name of the device

	returns success of the setting device
*/
HRESULT DShowCapture::SetDevice( const char *pName )
{
    HRESULT hr = E_FAIL;
    IBaseFilter **pF = &pSrc;
	CComPtr<IBaseFilter> pFilter;
    CComPtr<ICreateDevEnum> pSysDevEnum;
    CComPtr<IEnumMoniker> pEnumCat = NULL;
    BSTR bstrName = _com_util::ConvertStringToBSTR(pName);

    // Create the System Device Enumerator.
    pSysDevEnum.CoCreateInstance(CLSID_SystemDeviceEnum);
    // Obtain a class enumerator for the video compressor category.
    pSysDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnumCat, 0);
    
    // Enumerate the monikers.
    CComPtr<IMoniker> pMoniker;
    ULONG cFetched;
    while(pEnumCat->Next(1, &pMoniker, &cFetched) == S_OK)
    {
        CComPtr<IPropertyBag> pProp;
        pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pProp);
        VARIANT varName;
        VariantInit(&varName); // Try to match the friendly name.
        hr = pProp->Read(L"FriendlyName", &varName, 0); 
        if (SUCCEEDED(hr) && (wcscmp(bstrName, varName.bstrVal) == 0))
        {
            hr = pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void**)&pFilter);
            break;
        }
        VariantClear(&varName);
        pMoniker = NULL; // Release for the next loop.
    }
    *pF = pFilter;
    (*pF)->AddRef();  // Add ref on the way out.
    return hr;

}

/* Get some information */
long DShowCapture::GetWidth( void )
{
	return width;
}

long DShowCapture::GetHeight( void )
{
	return height;
}

long DShowCapture::GetBitCount( void )
{
	return bitcount;
}


/*
	HRESULT StartCapture( void )
	sets capture running

	params:
		@pGraph - pointer to GraphBuilder interface

	returns success of the starting video

*/
HRESULT DShowCapture::Start( void )
{
	CComQIPtr<IMediaControl> pControl(pGraph);
    return pControl->Run();
}

/*
	HRESULT StopCapture( void )
	Stops capture

	returns success of the stopping video

*/
HRESULT DShowCapture::Stop( void )
{
	CComQIPtr<IMediaControl> pControl(pGraph);
    return 	pControl->Stop();
}

/*
	void GetBuffer( ...  )
	Gives pointer to the buffer and its size

	param:
	   @nMyBufferSize - pointer to buffer size
	   @pMyBuffer - pointer to my buffer

*/
void DShowCapture::GetBuffer( long *pnMyBufferSize, long **ppMyBuffer )
{
	pSampleGrabber->GetCurrentBuffer( pnMyBufferSize, *ppMyBuffer);
}

/*
	void SaveBufferToBmp(const char *pName, long nBufferSize, long *pBuffer)
	saves buffer as BMP

	params:
	  @pName - name of the file
	  @nBufferSize - buffer size
	  @pBuffer - pointer to buffer
*/
void DShowCapture::SaveBufferToBmp(const char *pName, long nBufferSize, long *pBuffer )
{
	/* _convert and _lpa should not need 
	   Is there some header to include for this
	*/
	int _convert = 0;
	const char *_lpa = NULL;
	LPCWSTR name = ( LPCWSTR )A2W( pName );
	HANDLE fh;
	BITMAPFILEHEADER bmphdr;
	DWORD nWritten;

	memset(&bmphdr, 0, sizeof(bmphdr));

	bmphdr.bfType = ('M' << 8) | 'B';
	bmphdr.bfSize = sizeof(bmphdr) + sizeof(BITMAPINFOHEADER) + nBufferSize;
	bmphdr.bfOffBits = sizeof(bmphdr) + sizeof(BITMAPINFOHEADER);

	fh = CreateFile( name, 
					 GENERIC_WRITE, 
					 0, 
					 NULL,
					 CREATE_ALWAYS, 
					 FILE_ATTRIBUTE_NORMAL, 
					 NULL );
	WriteFile(fh, &bmphdr, sizeof(bmphdr), &nWritten, NULL);
	WriteFile(	fh,
				&pVideoInfoHeader->bmiHeader,
				sizeof(BITMAPINFOHEADER), 
				&nWritten, 
				NULL );
	WriteFile(fh, pBuffer, nBufferSize, &nWritten, NULL);

	CloseHandle(fh);
}
