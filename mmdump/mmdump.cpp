// mmdump.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "windows.h"
#include "winbase.h"
#include <string.h>
#include <iostream>
#include <process.h>
#include "mms.h"

using namespace std;

// a - beginning 
// b - end 
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
	/*
	if (argc < 1)
	{
		cout << "Usage: mmdump <physical>/<virtual>/<all>" << endl;
	}
	*/

	FILE* dumpfile = fopen("mmdump.out","r+b");
	lpvMem = mms_shared_memory();
	fprintf(dumpfile,"\nPhysical:\n");
	dump(dumpfile,lpvMem->pm,0,lpvMem->pm_size,0);


	FILE* memfile = fopen("mms.mem","rb");
	char buffer[32];
	int r =0;
	int addr = 0;
	fprintf(dumpfile,"\r\nVirtual:\r\n");
	while (r == fread(buffer,sizeof(char),sizeof(buffer),memfile)) 
	{
		dump(dumpfile,buffer,0,r,addr);
		addr+=r;
	}

	fclose(dumpfile);
	fclose(memfile);
	return 0;
}