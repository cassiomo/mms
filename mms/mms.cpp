// mms.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "windows.h"
#include "winbase.h"
#include "mms.h"
#include <iostream>
#include <stdio.h>
#include <memory.h>
#include <process.h>
#include "psapi.h"

using namespace std;

// The DLL entry-point function sets up shared
// memory using a named file-mapping object.

BOOL DllMain(HINSTANCE hinstDLL,  // DLL module handle
			 DWORD fdwReason,              // reason called
			 LPVOID lpvReserved)           // reserved
{
	BOOL fInit, fIgnore;

	switch (fdwReason)
	{
		// The DLL is loading due to process
		// initialization or a call to LoadLibrary.

	case DLL_PROCESS_ATTACH:

		// Create a named file mapping object.

		hMapObject = CreateFileMapping(
			INVALID_HANDLE_VALUE, // use paging file
			NULL,                 // default security attributes
			PAGE_READWRITE,       // read/write access
			0,                    // size: high 32-bits
			SHMEMSIZE,            // size: low 32-bits
			"dllmemfilemap");     // name of map object
		if (hMapObject == NULL)
			return FALSE;

		// The first process to attach initializes memory.

		fInit = (GetLastError() != ERROR_ALREADY_EXISTS);

		// Get a pointer to the file-mapped shared memory.

		lpvMem  = (shared_memory*)MapViewOfFile(
			hMapObject,     // object to map view of
			FILE_MAP_WRITE, // read/write access
			0,              // high offset:  map from
			0,              // low offset:   beginning
			0);             // default: map entire file
		if (lpvMem  == NULL)
			return FALSE;

		// Initialize memory if this is the first process.

		if (fInit)
			memset(lpvMem , '\0', SHMEMSIZE);

		break;

		// The attached process creates a new thread.

	case DLL_THREAD_ATTACH:
		break;

		// The thread of the attached process terminates.

	case DLL_THREAD_DETACH:
		break;

		// The DLL is unloading from a process due to
		// process termination or a call to FreeLibrary.

	case DLL_PROCESS_DETACH:

		// Unmap shared memory from the process's address space.

		fIgnore = UnmapViewOfFile(lpvMem );

		// Close the process's handle to the file-mapping object.

		fIgnore = CloseHandle(hMapObject);

		break;

	default:
		break;
	}

	return TRUE;
	UNREFERENCED_PARAMETER(hinstDLL);
	UNREFERENCED_PARAMETER(lpvReserved);
}

void log_msg(char* msg)
{
	SYSTEMTIME st;
	GetSystemTime(&st);
	FILE* logfile = fopen("mms.log","a");
	fprintf(logfile,"%04d%02d%02d%02d%02d%02d %s %d %s\r\n",
		st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond,
		strrchr(_pgmptr, '\\') + sizeof(char), _getpid(), msg);
	fclose(logfile);
}

unsigned actual_size(unsigned size)
{
	unsigned actual = ((size / lpvMem->boundary ) * lpvMem->boundary) +
		((( size % lpvMem->boundary) == 0) ? 0 : lpvMem->boundary);
	return actual;
}

int mms_free_inner(char* mem_ptr, int pid)
{
	unsigned int error_code = 0;
	char* empty = "\0";
	char msg[256];

	unsigned int vm_block = 4096;
	int chunk = 0;
	int pm_segment = 0;
	bool found_address = false;

	unsigned mem_addr = (unsigned)mem_ptr;
	if (mem_addr <= 0){
		error_code = INVALID_MEM_ADDR;
		sprintf(msg,"mms_free %d %d", error_code, mem_addr);
		log_msg(msg);
		return error_code;
	}

	for (int cnt = 1; cnt < MMAP_LIMIT - 1; cnt ++) {
		if (mem_addr == lpvMem->mmap[cnt].address)
		{
			found_address = true;
		}
	}

	if (!found_address)
	{
		sprintf(msg,"mms_free %d %d", INVALID_MEM_ADDR, mem_addr);
		log_msg(msg);
		return INVALID_MEM_ADDR;
	}

	FILE* memfile = fopen("mms.mem","r+b");
	int read_byte = 0;
	int byte_left = 0;

	for (int cnt = 1; cnt < MMAP_LIMIT - 1 ; cnt ++)
	{
		if (lpvMem->mmap[cnt].pid == pid)
		{
			if (mem_addr == lpvMem->mmap[cnt].address)
			{
				//keep reading until eof (compressing).
				fseek(memfile, lpvMem->mmap[cnt].offset + vm_block, SEEK_SET);
				read_byte = fread(lpvMem->pm,sizeof(char),lpvMem->pm_size,memfile);
				int moved = 0;
				while (read_byte > 0)
				{
					fseek(memfile, lpvMem->mmap[cnt].offset + moved, SEEK_SET);
					fwrite(lpvMem->pm,sizeof(char),read_byte,memfile);
					fseek(memfile,lpvMem->mmap[cnt].offset + vm_block + moved, SEEK_SET);
					read_byte = fread(lpvMem->pm,sizeof(char),lpvMem->pm_size,memfile);
					moved += read_byte;
					if (moved > lpvMem->mmap[0].offset)
						break;
				}

				//fseek(memfile,lpvMem->mmap[0].offset,SEEK_SET);
				if (lpvMem->pm_size < vm_block) {
					chunk = vm_block / lpvMem->pm_size;
					pm_segment = 0;
					for (int k = 0; k < chunk; k++)
					{
						//seek the next block
						fseek(memfile, lpvMem->mmap[0].offset + pm_segment - vm_block,SEEK_SET);
						//read the whole block of data
						byte_left = fread(lpvMem->pm,sizeof(char),lpvMem->pm_size,memfile);
						if (byte_left > 0) {
							//seek the current block offset
							fseek(memfile, lpvMem->mmap[0].offset + pm_segment - vm_block,SEEK_SET);
							// empty the last block of data
							memset(lpvMem->pm,'\0',lpvMem->pm_size);
							fwrite(lpvMem->pm,sizeof(char),lpvMem->pm_size,memfile);
							pm_segment += lpvMem->pm_size;
						} else {
							break;
						}
					}
				}

				for (int mm = 1; mm < MMAP_LIMIT - 1; mm++ )
				{
					//reset all offset in the memory map that is greater than
					//the current block
					if (lpvMem->mmap[mm].address > lpvMem->mmap[cnt].address)
					{
						lpvMem->mmap[mm].offset = lpvMem->mmap[mm].offset - vm_block;
						// increase the memory
						lpvMem->memory_usage += lpvMem->mmap[cnt].asize;
					}
				}

				// reduce the absolute offset for one vm_block
				lpvMem->mmap[0].offset = lpvMem->mmap[0].offset - vm_block;
				//reset the memory table to all 0
				lpvMem->mmap[cnt].pid = 0;
				lpvMem->mmap[cnt].rsize = 0;
				lpvMem->mmap[cnt].asize = 0;
				lpvMem->mmap[cnt].address = 0;
				lpvMem->mmap[cnt].offset = 0;
				lpvMem->mmap[cnt].lastRef.wDay = 0;
				lpvMem->mmap[cnt].lastRef.wDayOfWeek = 0;
				lpvMem->mmap[cnt].lastRef.wHour = 0;
				lpvMem->mmap[cnt].lastRef.wMilliseconds = 0;
				lpvMem->mmap[cnt].lastRef.wMinute = 0;
				lpvMem->mmap[cnt].lastRef.wMonth = 0;
				lpvMem->mmap[cnt].lastRef.wSecond = 0;
				lpvMem->mmap[cnt].lastRef.wYear = 0;
			}
		}
	}
	fclose(memfile);
	sprintf(msg,"mms_free %d %d", error_code, mem_addr);
	log_msg(msg);
	return 0;
}

void scan_mem() {

	DWORD aProcess[1024], cbNeeded, cProcesses;
	if (!EnumProcesses(aProcess, sizeof(aProcess), &cbNeeded))
		return;

	cProcesses = cbNeeded / sizeof(DWORD);

	bool found;

	for (int cnt = 1; cnt < MMAP_LIMIT - 1; cnt++) {
		found = false;
		if (lpvMem->mmap[cnt].pid == 0)
			continue;
		for (int k=0; k < cProcesses; k++) {
			if (lpvMem->mmap[cnt].pid == aProcess[k]) {
				found = true;
				break;
			}
		}
		if (!found && lpvMem->mmap[cnt].pid !=0)
			mms_free_inner((char*)lpvMem->mmap[cnt].address, lpvMem->mmap[cnt].pid);
	}
}

MMS_API bool mms_init(unsigned pm_size, unsigned vm_size, unsigned boundary_size)
{
	if ((pm_size < 0) || (vm_size < 0)  || (boundary_size < 0))
		return false;

	FILE* logfile = fopen("mms.log","w");

	fprintf(logfile,"start mms_init successfully\r\n");
	fclose(logfile);

	FILE* dumpfile = fopen("mmdump.out","w");
	fclose(dumpfile);

	FILE* memfile = fopen("mms.mem","w");
	fclose(memfile);

	

	lpvMem->boundary = boundary_size;
	lpvMem->pm_size = pm_size;
	lpvMem->vm_size = vm_size;
	lpvMem->mmap[0].pid = 0;
	lpvMem->mmap[0].rsize = 0;
	lpvMem->mmap[0].asize = 0;
	lpvMem->mmap[0].address = 0;
	lpvMem->mmap[0].offset = 0;
	
	lpvMem->memory_usage = pm_size;

	// initialize the memory table
	for (int cnt =1; cnt < MMAP_LIMIT - 1 ; cnt++)
	{
		lpvMem->mmap[cnt].pid = 0;
		lpvMem->mmap[cnt].rsize = 0;
		lpvMem->mmap[cnt].asize = 0;
		lpvMem->mmap[cnt].address = 0;
		lpvMem->mmap[cnt].lastRef.wDay = 0;
		lpvMem->mmap[cnt].lastRef.wDayOfWeek = 0;
		lpvMem->mmap[cnt].lastRef.wHour = 0;
		lpvMem->mmap[cnt].lastRef.wMilliseconds = 0;
		lpvMem->mmap[cnt].lastRef.wMinute = 0;
		lpvMem->mmap[cnt].lastRef.wMonth = 0;
		lpvMem->mmap[cnt].lastRef.wSecond = 0;
		lpvMem->mmap[cnt].lastRef.wYear = 0;
		lpvMem->mmap[cnt].offset = 0;
	}

	return true;
}



MMS_API char* mms_malloc(int size, int* error_code)
{
	char msg[256];

	if (size <= 0 || size > vm_block)  {
		*error_code = OUT_OF_MEM;
		sprintf(msg,"mms_malloc %d %d %d", 0, size, *error_code);
		log_msg(msg);
		return (char*)0;
	}

	SYSTEMTIME st;
	GetSystemTime(&st);


	if (lpvMem->memory_usage <= 0) {
		*error_code = OUT_OF_MEM;
		sprintf(msg,"mms_malloc %d %d %d", 0, size, *error_code);
		log_msg(msg);
		return (char*)0;
	}

	lpvMem->memory_usage -=size;

	scan_mem();
	unsigned int next_block = 4096;
	unsigned int block_size = 4096;
	for (int cnt = 1; cnt < MMAP_LIMIT - 1 ; cnt ++)
	{
		if (lpvMem->mmap[cnt].pid == _getpid())
		{
			if (lpvMem->mmap[cnt].address == next_block)
			{
				// next block is found, increment next_block
				next_block  += block_size;
				cnt = 1;
			}
		}
	}

	for (int cnt = 1; cnt < MMAP_LIMIT - 1; cnt ++) {
		//find the unused slot of entry
		if (lpvMem->mmap[cnt].pid == 0) {
			lpvMem->mmap[cnt].pid = _getpid();
			lpvMem->mmap[cnt].rsize = size;
			lpvMem->mmap[cnt].asize = actual_size(size);
			lpvMem->mmap[cnt].address +=next_block;
			lpvMem->mmap[cnt].lastRef = st;
			lpvMem->mmap[cnt].offset = lpvMem->mmap[0].offset;
			lpvMem->mmap[0].offset += block_size;	
			break;
		}
	}

	*error_code = 0;
	sprintf(msg,"mms_malloc %d %d %d", next_block, size, *error_code);
	log_msg(msg);

	return (char*)next_block;
}

MMS_API shared_memory* mms_shared_memory()
{
	return lpvMem;
}

MMS_API int mms_memset(char* dest_ptr, char c, int size)
{
	char msg[256];
	int error_code = 0;

	unsigned addr = (unsigned)dest_ptr;

	if (size <= 0) {
		error_code = MEM_TOO_SMALL;
		sprintf(msg,"mms_memset %d %d %d %d", error_code, addr, c, size);
		log_msg(msg);
		return error_code;
	}

	if ((addr <=0) || (size > addr))
	{
		error_code = INVALID_DEST_ADDR;
		sprintf(msg,"mms_memset %d %d %d %d", error_code, addr, c, size);
		log_msg(msg);
		return error_code;
	}

	for (int cnt = 1; cnt < MMAP_LIMIT - 1; cnt ++) {
		if (addr == lpvMem->mmap[cnt].address)
		{
			if (size > lpvMem->mmap[cnt].rsize)
			{
				error_code = MEM_TOO_SMALL;
				return error_code;
			}
		}
	}

	SYSTEMTIME st;
	GetSystemTime(&st);

	unsigned int block_size = 4096;
	unsigned int low_range = 0;
	unsigned int high_range = 0;
	unsigned int chunk = 0;
	unsigned int offset = 0;

	//If size is greater than physical memory, fill up the pm then copy 
	//the data to vm.
	memset(lpvMem->pm,c,size);

	//mmp and virtual memory
	for (int cnt = 1; cnt < MMAP_LIMIT - 1 ; cnt ++)
	{
		if (lpvMem->mmap[cnt].pid == _getpid())
		{
			low_range = lpvMem->mmap[cnt].address;
			high_range = lpvMem->mmap[cnt].address + block_size;
			if ((addr >= low_range) && (addr < high_range))
			{
				chunk = 0;
				if (size > lpvMem->pm_size) {
					chunk = size / lpvMem->pm_size;
				}
				FILE* memfile = fopen("mms.mem","r+b");
				//fseek the offset
				if (chunk > 1) {
					for (int repeat = 0; repeat < chunk; repeat++) {
						offset = lpvMem->mmap[cnt].offset + (addr - lpvMem->mmap[cnt].address);
						//fseek(memfile, lpvMem->mmap[cnt].offset + ((unsigned)dest_ptr - lpvMem->mmap[cnt].address),SEEK_SET);
						fseek(memfile, offset +  (repeat * lpvMem->pm_size) ,SEEK_SET);
						//fwrite(lpvMem->pm,sizeof(char),size,memfile);
						fwrite(lpvMem->pm,sizeof(char),lpvMem->pm_size,memfile);
					}
				} else {
						offset = lpvMem->mmap[cnt].offset + (addr - lpvMem->mmap[cnt].address);
						fseek(memfile, offset,SEEK_SET);
						fwrite(lpvMem->pm,sizeof(char),size,memfile);
				}
				fclose(memfile);
				lpvMem->mmap[cnt].lastRef = st;
			}
		}
	}

	sprintf(msg,"mms_memset %d %u %c %d", error_code, addr, c, size);
	log_msg(msg);
	return 0;
}

MMS_API int mms_memcpy(char* dest_ptr, char* src_ptr, int size)
{
	char msg[256];
	char* buffer;
	SYSTEMTIME st;
	GetSystemTime(&st);
	unsigned int block_size = 4096;
	int low_range = 0;
	int high_range = 0;
	bool found = false;
	unsigned int error_code = 0;
	unsigned int chunk = 0;
	unsigned int addr_offset = 0;
	unsigned int offset = 0;

	if (size <= 0) {
		error_code = MEM_TOO_SMALL;
		sprintf(msg,"mms_memcpy %d %s %s %d", error_code, dest_ptr, src_ptr, size);
		log_msg(msg);
		return error_code;
	}

	unsigned from_addr = (unsigned)src_ptr;
	unsigned to_addr = (unsigned)dest_ptr;

	if ((from_addr <= 0) || (to_addr <= 0) || (size > to_addr) ) {
		error_code = INVALID_CPY_ADDR;
		sprintf(msg,"mms_memcpy %d %d %d %d", error_code, to_addr, from_addr, size);
		log_msg(msg);
		return error_code;
	}

	for (int cnt = 1; cnt < MMAP_LIMIT - 1; cnt ++) {
		if (from_addr == lpvMem->mmap[cnt].address)
		{
			if (size > lpvMem->mmap[cnt].asize)
			{
				error_code = MEM_TOO_SMALL;
				return error_code;
			}
		}
	}

	//Copy the data from virtual memory to physical
	for (int cnt = 1; cnt < MMAP_LIMIT - 1 ; cnt ++)
	{
		if (lpvMem->mmap[cnt].pid == _getpid())
		{
			low_range = lpvMem->mmap[cnt].address;
			high_range = lpvMem->mmap[cnt].address + block_size;
			if ((from_addr >= low_range) && (from_addr < high_range))
			{
				addr_offset = from_addr - lpvMem->mmap[cnt].address;
				FILE* memfile = fopen("mms.mem","r+b");
				//fseek the offset
				offset = lpvMem->mmap[cnt].offset + addr_offset;
				if ( fseek(memfile,offset,SEEK_SET) == 0) 
				{
					fread(lpvMem->pm,sizeof(char),size,memfile);
				}
				fclose(memfile);
			}
		}
	}

	//Copy the data from physical memory to destination virutal memory pointer
	for (int cnt = 1; cnt < MMAP_LIMIT - 1 ; cnt ++)
	{
		if (lpvMem->mmap[cnt].pid == _getpid())
		{
			low_range = lpvMem->mmap[cnt].address;
			high_range = lpvMem->mmap[cnt].address + block_size;
			if ((to_addr >= low_range) && (to_addr < high_range))
			{
				int addr_offset = to_addr - lpvMem->mmap[cnt].address;
				FILE* memfile = fopen("mms.mem","r+b");
				fseek(memfile,lpvMem->mmap[cnt].offset + addr_offset,SEEK_SET);
				fwrite(lpvMem->pm,sizeof(char),size,memfile);
				fclose(memfile);
				lpvMem->mmap[cnt].lastRef = st;
				found = true;
			}
		}
	}

	if (!found) {
		memcpy(dest_ptr, lpvMem->pm, size);
	}

	sprintf(msg,"mms_memcpy %d %d %d %d", 0, to_addr, from_addr, size);
	log_msg(msg);
	return 0;
}



MMS_API int mms_free(char* mem_ptr)
{
	return mms_free_inner(mem_ptr, _getpid());
}
