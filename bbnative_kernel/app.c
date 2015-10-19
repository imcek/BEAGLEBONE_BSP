#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

int main()
{
	int fd;
	char Ubuff[]="SAMPLE CHARDRIVER USING LOWLEVEL FRAMEWORK";
	char Kbuff[50];

	fd = open("/dev/bbnative", O_RDWR);
	if(fd < 0) {
		perror("Unable to open the device file\n");
		return -1;
	}
	char gpioNum = 67;
	char gpioVal = 1;
	int result;
		if(fd!=-1) {
		   result = ioctl(fd, 10, &gpioNum, &gpioVal);
		}
	/* Write the data into the device */
	write(fd , Ubuff , strlen(Ubuff) + 1);

	/* Read the data back from the device */
	memset(Kbuff , 0 ,sizeof(Kbuff));
	read(fd , Kbuff , sizeof(Kbuff));
	printf("Data from kernel : %s\n", Kbuff);

	close(fd);	
	return 0;
}

