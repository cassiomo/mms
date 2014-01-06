// mmstart.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "windows.h"
#include "winbase.h"
#include <iostream>
#include <process.h>
#include "mms.h"

using namespace std;

void dump(FILE* fp, char* buf, int a, int b, unsigned offset) 
{ 
	for (int i=a; i<b; i+=16) 
	{ 
		fprintf(fp, "%04X ", i + offset); 
		for (int j=0; j<16; j++) 
		{ 
			if (i+j < b) 
				fprintf(fp, "%02X ", (byte)buf[i+j]); 
			else 
				fprintf(fp, " "); 

			if (j==7) fprintf(fp, " "); 
		} 
		fprintf(fp, " "); 
		for (int j=0; j<16; j++) 
		{ 
			if (i+j < b) 
				fprintf(fp, "%c", (buf[i+j] < 0x20 || buf[i+j] > 0x7F) ? '.' : buf[i+j]); 
			else 
				fprintf(fp, " "); 
		} 
		fprintf(fp, "\n"); 
	} 
}

int _tmain(int argc, _TCHAR* argv[])
{

	// run forever

	if (argc < 3) {
		printf("Usage: mmstart <physical size> <virtual size> <boundary size>\n");
		printf("<virtual size> - simluated virtual memory size\n");
		printf("<boundary size> - minimum number of bytes in an allocated memory regioin\n");
		exit(-1);
	}

	int physical_memory = atoi(argv[1]);
	int virtual_memory = atoi(argv[2]);
	int boundary = atoi(argv[3]);

	if (!mms_init(physical_memory,virtual_memory, boundary)) 
	{
		printf("mms_init failed");
		exit(-1);
	}

	printf("mm_init starts succefully\n");
	printf("Physical: %d\n", physical_memory);
	printf("Virtual: %d\n", virtual_memory);
	printf("Boundary: %d\n", boundary);

	int request = 0;
	char input[128];	 

	unsigned arg1, arg2, arg3;
	unsigned result;
	int addr=0;
	FILE* memfile;
	char inputbuffer[4096];

	for (;;) {
		printf(">");
		gets(input);
		switch (input[0]) {
			char buffer[128];
			case 'm':
				if (strncmp(input,"mms_malloc",10) == 0) {
					sscanf(input,"%s %d %d",buffer, &arg1, &arg2);
					arg2 = 0;
					result = (unsigned)mms_malloc(arg1,(int*)&arg2);
					printf("Return address: %u Error Code: %d\n",(char*)result, arg2);
				} else if (strncmp(input,"mms_memset",10) == 0) {
					sscanf(input, "%s %d %c %d", buffer, &arg1, &arg2, &arg3);
					result = mms_memset((char*)arg1,arg2,arg3);
					printf("Return: %d\n", result);
				} else if (strncmp(input,"mms_memcpy",10) == 0) {
					sscanf(input,"%s %d %d %d", buffer, &arg1, &arg2, &arg3);
					if (arg1 == 0)	{
						memset(inputbuffer, 0, sizeof(inputbuffer));
						arg1 = (int)&inputbuffer;
					}		
					result = mms_memcpy((char*)arg1,(char*)arg2,arg3);
					printf("Return: %d\n", result);
					//if (arg1 == (int)&inputbuffer) 
					//	printf("Buffer: %s\n", inputbuffer);
				} else if (strncmp(input,"mms_free",8) == 0) {
					sscanf(input,"%s %d",buffer, &arg1);
					result = mms_free((char*)arg1);
					printf("Return: %d\n", result);
				}
				break;
			case 'p':
				lpvMem = mms_shared_memory();
				printf("Physical: \n");
				if (lpvMem !=NULL) 
					dump(stdout,lpvMem->pm,0,lpvMem->pm_size,0);
				else 
					printf("Physical memory is empty\n");
				break;
			case 'v':
				printf("Virtual: \n");
				memfile = fopen("mms.mem","rb");
				while (int r = fread(buffer,sizeof(char),sizeof(buffer),memfile)) 
				{	
					dump(stdout,buffer,0,r,addr);
					addr+=r;
				}
				fclose(memfile);
				break;
			case 's': 
				lpvMem = mms_shared_memory();
				printf("Start:\n");
				printf("Physical: %d\n",physical_memory);
				printf("Virtual: %d\n", virtual_memory);
				printf("Boundary: %d\n", boundary);
				break;
			case 'a':
				lpvMem = mms_shared_memory();
				if (lpvMem !=NULL){
					printf("  pid   rsize   asize  c_address	  last_ref	offset\n");
					for (int cnt = 0; cnt < MMAP_LIMIT ; cnt++) 
					{
						SYSTEMTIME st;
						st = lpvMem->mmap[cnt].lastRef;
						printf("%5d %6d %6d %10d %10d%02d%02d%02d%02d%02d %8d\n",
							lpvMem->mmap[cnt].pid,lpvMem->mmap[cnt].rsize,
							lpvMem->mmap[cnt].asize,lpvMem->mmap[cnt].address,
							st.wYear,st.wMonth,
							st.wDay,st.wHour,
							st.wMinute,st.wSecond,
							lpvMem->mmap[cnt].offset);
					}
				} else {
					printf("Memory Map is empty");
				}
				break;
			default:
				printf("s - status\n");
				printf("p - dump physical memeory\n");
				printf("v - dump virtual memory\n");
				printf("a - dump the memory map\n");
				printf("mms_malloc <size> <error code>\n");
				printf("mms_memset <dest_ptr> <char> <size>\n");
				printf("mms_memcpy <dest_ptr> <char> <size>\n");
				printf("mms_free <mem_ptr>\n");
				break;
		}
	}
	return 0;
}