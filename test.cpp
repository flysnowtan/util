#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int hashLockTest()
{

}


int main(int argc, char * argv[])
{
	char func[128];

	char ch;

	while((ch = getopt(argc, argv, "f:")) != -1)
	{
		switch(ch) {
			case 'f':
				strncpy(func, optarg, 127);
				break;
			default:
				break;
		}
	}
	return 0;
}
