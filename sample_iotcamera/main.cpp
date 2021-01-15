#include <stdio.h>

extern "C"
{
	extern int venc_main(int argc, char *argv[]);
	extern int aenc_main(int argc, char *argv[]);
}
int main(int argc , char ** argv)
{
	aenc_main(0,0);
	venc_main(0,0);
	return 0;
}
