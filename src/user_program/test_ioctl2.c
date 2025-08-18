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
    	char write_buf[] = "This is a test message.";
    	char read_buf[256] = {0, };
    	ssize_t bytes_written, bytes_read;

	// 1. Open device file
    	// 드라이버 수정으로 인해 장치 노드 이름이 'keyringctl' 대신 'keyringctl0' 같은 식으로 생성될 수 있다.
    	// ls /dev/keyringctl* 명령어로 정확한 장치 노드를 확인하자.
	fd = open("/dev/keyringctl0", O_RDWR);
	if (fd < 0){
		perror("Error: Failed to open the device file. Check if the module is loaded and the device file exists.");
		return -1;
	}
	printf("Device file opened successfully.\n");

	// 2. SET_PASSWORD test
	printf("\n--- Test 1: SET_PASSWORD ---\n");
	if(ioctl(fd, SET_PASSWORD, password) < 0){
		perror("Error: Failed to set password.");
		close(fd);
		return -1;
	}
	printf("Success: Password set and device is now locked.\n");

    	// 3. READ/WRITE test (When device is LOCKED)
    	// 잠금 상태이므로, 읽기/쓰기 작업이 거부되어야 한다.
    	printf("\n--- Test 2: READ/WRITE when device is LOCKED ---\n");
    	printf("Attempting to write to the locked device...\n");
    	bytes_written = write(fd, write_buf, strlen(write_buf));
    	if (bytes_written < 0) {
        	perror("Success: Write operation denied as expected");
    	} else {
        	printf("Error: Write succeeded, but should have been denied.\n");
    	}

    	printf("Attempting to read from the locked device...\n");
    	bytes_read = read(fd, read_buf, sizeof(read_buf));
    	if (bytes_read < 0) {
        	perror("Success: Read operation denied as expected");
    	} else {
        	printf("Error: Read succeeded, but should have been denied.\n");
    	}

	// 4. VERIFY_PASSWORD test (Correct Password)
	printf("\n--- Test 3: VERIFY_PASSWORD (Correct Password) ---\n");
	if (ioctl(fd, VERIFY_PASSWORD, password) < 0) {
		perror("Error: Failed to verify password with the correct one.");
	} else {
		printf("Success: Password verified. Device is now unlocked.\n");
	}

    	// 5. READ/WRITE test (When device is UNLOCKED)
    	// 잠금 해제 상태이므로, 읽기/쓰기 작업이 성공해야 한다.
    	printf("\n--- Test 4: READ/WRITE when device is UNLOCKED ---\n");
    	printf("Attempting to write to the unlocked device...\n");
    	bytes_written = write(fd, write_buf, strlen(write_buf));
    	if (bytes_written >= 0) {
        	printf("Success: Write operation succeeded. Wrote %zd bytes.\n", bytes_written);
    	} else {
        	perror("Error: Write failed, but should have succeeded");
    	}

    	printf("Attempting to read from the unlocked device...\n");
    	bytes_read = read(fd, read_buf, sizeof(read_buf));
    	if (bytes_read >= 0) {
        	printf("Success: Read operation succeeded. Read %zd bytes. Received message: %s\n", bytes_read, read_buf);
    	} else {
        	perror("Error: Read failed, but should have succeeded");
    	}

	// 6. VERIFY_PASSWORD test (Incorrect Password, should fail as expected)
	printf("\n--- Test 5: VERIFY_PASSWORD (Incorrect Password) ---\n");
    	// 현재 잠금 해제 상태이므로, 다시 잠그기 위해 ioctl을 호출한다.
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

	// 7. RESET_PASSWORD test
	printf("\n--- Test 6: RESET_PASSWORD ---\n");
	if(ioctl(fd, RESET_PASSWORD) < 0){
		perror("Error: Failed to reset password");
	} else {
		printf("Success: Password reset. Device is unlocked.\n");
	}

	// 8. Close device file
	close(fd);
	printf("\nTests completed.\n");
	return 0;
}
