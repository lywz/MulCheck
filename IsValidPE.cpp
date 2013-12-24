#include "main.h"
BOOL IsValidPEFile( LPCTSTR pszPathName )
{
	if ( ! PathFileExists( pszPathName ) )
		return FALSE;

	HANDLE hFile = CreateFile( pszPathName, 
		GENERIC_READ, 
		FILE_SHARE_READ, 
		NULL, 
		OPEN_EXISTING, 
		FILE_ATTRIBUTE_NORMAL, 
		NULL );
	if ( hFile == INVALID_HANDLE_VALUE )
	{
		return FALSE;
	}

	HANDLE hMMFile = CreateFileMapping( hFile, NULL, PAGE_READONLY, 0, 0, NULL );
	if ( hMMFile == INVALID_HANDLE_VALUE ) {
		CloseHandle( hFile );
		return FALSE;
	}

	LPVOID pvMem = MapViewOfFile( hMMFile, FILE_MAP_READ, 0, 0, 0 );
	if ( ! pvMem ) {
		CloseHandle( hMMFile );
		CloseHandle( hFile );
		return FALSE;
	}

	if ( *( USHORT* ) pvMem != IMAGE_DOS_SIGNATURE ) {
		UnmapViewOfFile( pvMem );
		CloseHandle( hMMFile );
		CloseHandle( hFile );
		return FALSE;
	}

	if ( *( ( DWORD* ) ( ( PBYTE ) pvMem + ( ( PIMAGE_DOS_HEADER ) pvMem )->e_lfanew ) ) != IMAGE_NT_SIGNATURE ) {
		UnmapViewOfFile( pvMem );
		CloseHandle( hMMFile );
		CloseHandle( hFile );
		return FALSE;
	}

	UnmapViewOfFile( pvMem );
	CloseHandle( hMMFile );
	CloseHandle( hFile );

	return TRUE;
}