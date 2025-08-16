#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>

// ioctl cmd definition
#define IOC_MAGIC 'K'
#define SET_PASSWORD _IOW(IOC_MAGIC, 0, void*)
#define VERIFY_PASSWORD _IOW(IOC_MAGIC, 1, void*)
#define RESET_PASSWORD _IO(IOC_MAGIC, 2)
#define MAX_PASSWORD_LEN 32

int main(void){
	int fd;
	char password[MAX_PASSWORD_LEN] = "hello1234";
	char wrong_password[MAX_PASSWORD_LEN] = "wrong1234";

	// 1. Open device file
	fd = open("/dev/keyringctl", O_RDWR);
	if (fd < 0){
		perror("Error: Failed to open the device file. Check if the module is loaded and the device file exists.");
		return -1;
	}
	printf("Device file opened successfully.\n");

	// 2. SET_PASSWORD test
	printf("\n--- Test: SET_PASSWORD ---\n");
	if(ioctl(fd, SET_PASSWORD, password) < 0){
		perror("Error: Failed to set password.");
		close(fd);
		return -1;
	}
	printf("Success: Password set and device is now locked.\n");
	sleep(1);

	// 3. VERIFY_PASSWORD test (correct password)
	printf("\n--- Test: VERIFY_PASSWORD (Correct Password) ---\n");
	if (ioctl(fd, VERIFY_PASSWORD, password) < 0) {
		perror("Error: Failed to verify password with the correct one.");
	} else {
		printf("Success: Password verified. Device is now unlocked.\n");
	}
	sleep(1);

	// 4. VERIFY_PASSWORD test (wrong password)
	printf("\n--- Test: VERIFY_PASSWORD (Incorrect Password) ---\n");
	// 모듈의 `is_locked` 상태가 다시 잠기도록 시뮬레이션
    	if (ioctl(fd, SET_PASSWORD, password) < 0) {
        	perror("Error: Failed to re-lock device for incorrect password test");
        	close(fd);
        	return -1;
	}
    	if (ioctl(fd, VERIFY_PASSWORD, wrong_password) < 0) {
        	perror("Success: Password verification failed as expected");
    	} else {
        	printf("Error: This should not happen. Password verification succeeded with an incorrect password.\n");
    	}
    	sleep(1);

	// 5. RESET_PASSWORD test
	printf("\n--- Test: RESET_PASSWORD ---\n");
	if(ioctl(fd, RESET_PASSWORD) < 0){
		perror("Error: Failed to reset password");
	} else {
		printf("Success: Password reset. Device is unlocked.\n");
	}

	// 6. Close device file
	close(fd);
	printf("\nTests completed successfully.\n");
	return 0;
}
