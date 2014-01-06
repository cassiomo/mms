#include "stdafx.h"
#include "windows.h"
#include "winbase.h"
#include <iostream>
#include <process.h>
#include "mms.h"

char inputbuffer[4096];
unsigned output;
unsigned arg1;
unsigned arg2;
unsigned arg3;

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

void print_output() {

	char buffer[128];
	int addr=0;
	FILE* memfile;
	char inputbuffer[4096];
	if (lpvMem !=NULL) 
		dump(stdout,lpvMem->pm,0,lpvMem->pm_size,0);
	else 
		printf("Physical memory is empty\n");
	printf("Virtual: \n");
	memfile = fopen("mms.mem","rb");
	while (int r = fread(buffer,sizeof(char),sizeof(buffer),memfile)) 
	{	
		dump(stdout,buffer,0,r,addr);
		addr+=r;
	}
	fclose(memfile);
}

void mms_test(char* input) {
	char buffer[128];
	char carg1;
	if (strncmp(input, "mms_malloc", 10) == 0) {
		sscanf(input, "%s %d %d", buffer, &arg1, &arg2);
		output = (unsigned)mms_malloc(arg1, (int*)&arg2);
		printf("Return: %u, error_code: %d\n", (char*)output, arg2);
		print_output();
	} else if (strncmp(input, "mms_memset", 10) == 0) {
		sscanf(input, "%s %d %c %d", buffer, &arg1, &carg1, &arg2);
		output = mms_memset((char*)arg1, carg1, arg2);
		printf("Return: %d\n", output);
		print_output();
	} else if (strncmp(input, "mms_memcpy", 10) == 0) {
		sscanf(input, "%s %d %d %d", buffer, &arg1, &arg2, &arg3);
		if (arg1 == 0)	{
			memset(inputbuffer, 0, sizeof(inputbuffer));
			arg1 = (int)&inputbuffer;
		}		
		output = mms_memcpy((char*)arg1, (char*)arg2, arg3);
		printf("Return: %d\n", output);
		//if (arg1 == (int)&inputbuffer) printf("Buffer: %s\n", inputbuffer);
		print_output();
	} else if (strncmp(input, "mms_free", 8) == 0) {
		sscanf(input, "%s %d", buffer, &arg1);
		output = mms_free((char*)arg1);
		printf("Return: %d\n", output);
		print_output();
	}
	return;
}


int _tmain(int argc, _TCHAR* argv[])
{

	if (argc != 2) {
		printf("Usage: %s <testfile>\n", strrchr(_pgmptr, '\\') + sizeof(char));
		return 0;
	}

	int failures = 0;
	int passes = 0;

	char input[128];
	FILE* fp = fopen(argv[1], "r");
	while (fgets(input, sizeof(input), fp) != NULL) {
		printf(">%s", input);
		switch(input[0]) {
			case 10:
				break;
			case '/':
				printf("");
				break;
			case 'm':
				mms_test(input);
				break;
			default:
				break;
		}
	}

	printf("\n\n");
	printf("Hit return to exit.\n");
	gets(input);
	return 0;
}

