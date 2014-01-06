// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the MMS_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// MMS_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.

#ifdef MMS_EXPORTS
#define MMS_API __declspec(dllexport)
#else
#define MMS_API __declspec(dllimport)
#endif

#include "stdafx.h"

typedef struct mmap_t
{	
	unsigned int pid;
	unsigned int rsize;
	unsigned int asize;
	unsigned int address;
	SYSTEMTIME lastRef;
	unsigned int offset;
} memory_map;

#define SHMEMSIZE 20480
//const unsigned int MMAP_LIMIT= 10;
const unsigned int MMAP_LIMIT = 205;

typedef struct shared_memory_t {
	unsigned int boundary;
	unsigned int pm_size;
	unsigned int vm_size;
	unsigned int memory_usage;	
	memory_map mmap[MMAP_LIMIT];
	char pm[4096];
} shared_memory;

const unsigned int vm_block = 4096;
static shared_memory* lpvMem = NULL;       // pointer to shared memory
static HANDLE hMapObject = NULL;  // handle to file mapping

const unsigned int OUT_OF_MEM=100;
const unsigned int MEM_TOO_SMALL=101;
const unsigned int INVALID_DEST_ADDR=102;
const unsigned int INVALID_CPY_ADDR=103;
const unsigned int INVALID_MEM_ADDR=104;


extern "C" {
	
	MMS_API char* mms_malloc(int size, int* error_code);
	
	MMS_API int mms_memset(char* dest_ptr, char c, int size);

	MMS_API int mms_memcpy(char* dest_ptr, char* src_ptr, int size);

	MMS_API int mms_free(char* mem_ptr);

	MMS_API bool mms_init(unsigned pm_size, unsigned vm_size, unsigned boundary_size); 

	MMS_API shared_memory* mms_shared_memory();
}