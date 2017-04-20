#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <strsafe.h>
#include <assert.h>
#include <string>

#define DBJINLINE static __forceinline
#define BUFFERSIZE 5
DWORD g_BytesTransferred = 0;

void dbjbox(
	const char prompt[],
	...)
{
	const std::string & title = "DBJ";
	va_list args;
	// Expand the formatting string.
	const unsigned bufsiz_ = BUFSIZ * 2; // 1024
	char tmp[bufsiz_]; //1024
	std::memset(tmp, 0, bufsiz_);
	va_start(args, prompt);
	vsnprintf(tmp, bufsiz_, prompt, args);
	// tmp[sizeof(tmp) - 1] = 0;
	va_end(args);

	MessageBox(
		NULL,
		(LPCWSTR)tmp,
		(LPCWSTR)title.data(),
		MB_ICONEXCLAMATION | MB_OK
	);

}
std::string format_error()
// Routine Description:
// Retrieve and output the system error message for the last-error code
{
	std::string lpMsgBuf;
//	LPVOID lpDisplayBuf;
	DWORD dw = GetLastError();

	if (!dw) return "UNKNOWN";

	lpMsgBuf.resize(BUFSIZ,0);

	assert( 0 != FormatMessage(
		/*FORMAT_MESSAGE_ALLOCATE_BUFFER |*/
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dw,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPWSTR)&lpMsgBuf[0],
		lpMsgBuf.size(),
		NULL));

	return lpMsgBuf;
	/*
	lpDisplayBuf =
	(LPVOID)LocalAlloc(LMEM_ZEROINIT,
	(lstrlen((LPCWSTR)lpMsgBuf)
	+ lstrlen((LPCWSTR)lpszFunction)
	+ 40) // account for format string
	* sizeof(TCHAR));

	if (FAILED(StringCchPrintf((LPTSTR)lpDisplayBuf,
	LocalSize(lpDisplayBuf) / sizeof(TCHAR),
	TEXT("%s failed with error code %d as follows:\n%s"),
	lpszFunction,
	dw,
	lpMsgBuf)))
	{
	printf("FATAL ERROR: Unable to output error code.\n");
	}

	_tprintf(TEXT("ERROR: %s\n"), (LPCTSTR)lpDisplayBuf);

	dbj_box(L"ERROR", lpDisplayBuf);

	LocalFree(lpMsgBuf);
	LocalFree(lpDisplayBuf);
	*/
}
#if 0
VOID CALLBACK FileIOCompletionRoutine(
	__in  DWORD dwErrorCode,
	__in  DWORD dwNumberOfBytesTransfered,
	__in  LPOVERLAPPED lpOverlapped
);

VOID CALLBACK FileIOCompletionRoutine(
	__in  DWORD dwErrorCode,
	__in  DWORD dwNumberOfBytesTransfered,
	__in  LPOVERLAPPED lpOverlapped)
{
	_tprintf(TEXT("Error code:\t%x\n"), dwErrorCode);
	_tprintf(TEXT("Number of bytes:\t%x\n"), dwNumberOfBytesTransfered);
	g_BytesTransferred = dwNumberOfBytesTransfered;
}
#endif

DBJINLINE std::string & allocate_string_for_file_buffer( HANDLE file, std::string & buf_ ) {
	
	DWORD fileSize = GetFileSize(file, &fileSize);
	buf_.reserve((size_t)fileSize);
	return buf_;
}
//
// Note: CreateFile and ReadFile
// do not use parameters to differentiate between text and binary file types.
//

extern std::string win32fread( char file_name [] )
{
	HANDLE hFile;
	DWORD  dwBytesRead = 0;
	OVERLAPPED ol = { 0 };

	hFile = CreateFile(
		(LPCWSTR)file_name,    // file to open
		GENERIC_READ,          // open for reading
		FILE_SHARE_READ,       // share for reading
		NULL,                  // default security
		OPEN_EXISTING,         // existing file only
		FILE_ATTRIBUTE_NORMAL, // normal file NO ovelapped mumbo-jumbo
		NULL);                 // no attr. template

	if (hFile == INVALID_HANDLE_VALUE)
	{
		dbjbox("Terminal failure: unable to open file [%s] for read. Error is: [%s]\n", 
			file_name, format_error());
		return "";
	}

	std::string   ReadBuffer{ };
	
	allocate_string_for_file_buffer(hFile, ReadBuffer);

	DWORD  read = -1;

	if (FALSE == ReadFile(hFile, &ReadBuffer[0], (DWORD)ReadBuffer.size(), &read, 0))
	{
		dbjbox("Terminal failure: Unable to read from file.\n LastError: %s\n", GetLastError());
		CloseHandle(hFile);
		return "";
	}
	// SleepEx(0, TRUE); // this is clever dbj in action :)

	// It is always good practice to close the open file handles even though
	// the app will exit eventually and clean up open handles anyway.
	CloseHandle(hFile);

	return ReadBuffer;

#if 0	
	dwBytesRead = g_BytesTransferred;
	// This is the section of code that assumes the file is ANSI text. 
	// Modify this block for other data types if needed.

	if (dwBytesRead > 0 && dwBytesRead <= BUFFERSIZE - 1)
	{
		ReadBuffer[dwBytesRead] = '\0'; // NULL character

		_tprintf(TEXT("Data read from %s (%d bytes): \n"), file_name, dwBytesRead);
		printf("%s\n", ReadBuffer);
	}
	else if (dwBytesRead == 0)
	{
		_tprintf(TEXT("No data read from file %s\n"), file_name);
	}
	else
	{
		printf("\n ** Unexpected value for dwBytesRead ** \n");
	}
#endif
	
}



