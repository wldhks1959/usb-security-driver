#include <linux/module.h>
#include <linux/init.h>
#include <linux/usb.h>
#include <linux/fs.h> // for  file_operations
#include <linux/cdev.h> // for cdev sturct
#include <linux/device.h> // class_create, device_create
#include <linux/slab.h> // for kzalloc
#include <linux/printk.h>
#include <linux/uaccess.h> // for copy_from_user
#include <linux/ioctl.h> // for ioctl

// ioctl cmd definition
#define IOC_MAGIC 'K'
#define SET_PASSWORD _IOW(IOC_MAGIC, 0, void*)
#define VERIFY_PASSWORD _IOW(IOC_MAGIC, 1, void*)
#define RESET_PASSWORD _IO(IOC_MAGIC, 2)
#define MAX_PASSWORD_LEN 32

#define DRIVER_NAME "custom_usb_security"
#define DEVICE_NAME "keyringctl"
#define CLASS_NAME "custom"

#define USB_DEV_VENDOR_ID 0x0781 // sandisk
#define USB_DEV_PRODUCT_ID 0x5581 // sandisk's ultra

static dev_t dev_num;
static struct class* custom_class = NULL;

// USB 장치별 정보를 담을 구조체
struct custom_dev {
	struct usb_device *udev;
	struct usb_interface *interface;
	struct cdev custom_cdev;
	// 비밀번호 저장소 추가
	char password[MAX_PASSWORD_LEN];
	bool is_locked;
};

// character device driver function
static int custom_device_open(struct inode *inode, struct file *file){
	struct custom_dev *dev = container_of(inode->i_cdev, struct custom_dev, custom_cdev);
	file->private_data = dev; // private_data에 저장하여 다른 함수에서 사용
	
	printk(KERN_INFO "%s: Device Opened.\n", DRIVER_NAME);
	return 0;
}

static int custom_device_release(struct inode *inode, struct file *file){
	printk(KERN_INFO "%s: Device closed.\n", DRIVER_NAME);
	return 0;
}

// ioctl 명령을 처리하는 함수
static long custom_device_ioctl(struct file *file, unsigned int cmd, unsigned long arg){
	struct custom_dev *dev = file->private_data;
	char user_password[MAX_PASSWORD_LEN];
	int retval = 0;
	
	switch(cmd) {
		case SET_PASSWORD:
			if(copy_from_user(user_password, (char*)arg, MAX_PASSWORD_LEN)){
				retval = -EFAULT;
			} else {
				memcpy(dev->password, user_password, MAX_PASSWORD_LEN);
				dev->is_locked=true;
				printk(KERN_INFO "%s: Password set and device locked.\n", DRIVER_NAME);
			}
			break;

		case VERIFY_PASSWORD:
			if(!dev->is_locked){
				printk(KERN_INFO "%s: Device is not locked.\n", DRIVER_NAME);
				retval = -EPERM;
			} else {
				if(copy_from_user(user_password, (char*)arg, MAX_PASSWORD_LEN)){
					retval = -EFAULT;
				} else if (memcmp(dev->password, user_password, MAX_PASSWORD_LEN)==0){
					printk(KERN_INFO "%s: Password verified. Device unlocked.\n", DRIVER_NAME);
					dev->is_locked = false;
				} else {
					printk(KERN_INFO "%s: Password verification failed.\n", DRIVER_NAME);
					retval = -EACCES;
				}
			}
			break;
		
		case RESET_PASSWORD:
			memset(dev->password, 0, MAX_PASSWORD_LEN);
			dev->is_locked = false;
			printk(KERN_INFO "%s: Password reset. Device is unlocked.\n", DRIVER_NAME);
			break;
		
		default:
			retval = -EINVAL;
			break;
	}
	return retval;
}

// read, write 등은 다음 단계에서 구현
static const struct file_operations custom_fops = {
	.owner = THIS_MODULE,
	.open = custom_device_open,
	.release = custom_device_release,
	.unlocked_ioctl = custom_device_ioctl,
};

// USB Device Driver가 지원하는 Device list
// 이 테이블의 VID와 PID를 가진 장치가 연결되면 드라이버의 probe 함수가 호출된다.
static const struct usb_device_id custom_usb_table[] = {
	{ USB_DEVICE(USB_DEV_VENDOR_ID, USB_DEV_PRODUCT_ID) },
	{ } // 테이블의 끝을 알리는 빈 엔트리
};
MODULE_DEVICE_TABLE(usb, custom_usb_table);

// usb device가 system에 연결 시 호출되는 함수
// 이 함수에서 특정 장치임을 확인하고 드라이버를 초기화한다.
static int custom_probe(struct usb_interface *interface, const struct usb_device_id *id){
	struct usb_device *udev = interface_to_usbdev(interface);
	struct custom_dev *dev;
	int retval;

	// 연결된 장치의 VID와 PID를 로그로 출력 
	printk(KERN_INFO "%s: USB device with VID:PID %04x:%04x connected.\n",
		DRIVER_NAME, id->idVendor, id->idProduct);

	// 타겟 장치와 일치하는지 확인
	if(id->idVendor == USB_DEV_VENDOR_ID && id->idProduct == USB_DEV_PRODUCT_ID){
		printk(KERN_INFO "%s: Found taget USB device! Driver controls this device.\n", DRIVER_NAME);
	} else {
		printk(KERN_INFO "%s: It's a different USB device. Exit control.\n", DRIVER_NAME);
		return -ENODEV;
	}
	
	// 장치가 식별되면 character device를 생성한다.
	dev = kzalloc(sizeof(*dev), GFP_KERNEL);
	if(!dev)
		return -ENOMEM;

	dev->udev = udev;
	dev->interface = interface;
	usb_set_intfdata(interface, dev);
	dev->is_locked = false;

	// cdev init and add
	cdev_init(&dev->custom_cdev, &custom_fops);
	dev->custom_cdev.owner = THIS_MODULE;
	retval = cdev_add(&dev->custom_cdev, dev_num, 1);
	if (retval) {
		printk(KERN_ALERT "%s: Failed to add dev. \n", DRIVER_NAME);
		kfree(dev);
		return retval;
	}

	// /dev에 장치 노드 생성
	device_create(custom_class, NULL, dev_num, NULL, DEVICE_NAME);

	return 0;
}

// USB 장치가 시스템에서 분리될 때 호출되는 함수
static void custom_disconnect(struct usb_interface *interface){
	struct custom_dev *dev = usb_get_intfdata(interface);
	
	printk(KERN_INFO "%s: USB device is disconnected.\n", DRIVER_NAME);
	
	// delete character device 
	device_destroy(custom_class, dev_num);
	cdev_del(&dev->custom_cdev);
	
	if(dev){
		kfree(dev);
		usb_set_intfdata(interface, NULL);
	}
}


// USB Driver struct
// 여기에 probe와 disconnect 함수, id_table을 등록한다.
static struct usb_driver custom_driver = {
	.name = DRIVER_NAME,
	.id_table = custom_usb_table,
	.probe = custom_probe,
	.disconnect = custom_disconnect,
};

// 모듈 로드 시 호출되는 초기화 함수
static int __init custom_init(void){
	int ret;

	// allocate  character device num 
	ret = alloc_chrdev_region(&dev_num, 0, 1, DRIVER_NAME);
	if(ret < 0) {
		printk(KERN_ALERT "Failed to register a major number\n");
		return ret;
	}

	// create device class 
	custom_class = class_create(CLASS_NAME);
	if(IS_ERR(custom_class)){
		unregister_chrdev_region(dev_num, 1);
		printk(KERN_ALERT "Failed to create device class. \n");
		return PTR_ERR(custom_class);
	}
	
	// register usb driver 
	ret = usb_register(&custom_driver);
	if (ret) {
		class_destroy(custom_class);
		unregister_chrdev_region(dev_num, 1);
		printk(KERN_ALERT "%s: Failed to register USB Driver. (%d)\n", DRIVER_NAME, ret);
		return ret;
	}

	printk(KERN_INFO "%s: USB 드라이버가 등록되었다. USB 장치를 기다린다..\n", DRIVER_NAME);
	return 0;
}

// 모듈 언로드 시 호출되는 종료 함수.
static void __exit custom_exit(void){
	// usb driver 등록 해제
	usb_deregister(&custom_driver);
	// 디바이스 클래스 제거
	if(custom_class)	
		class_destroy(custom_class);

	// 디바이스 번호 등록 해제
	unregister_chrdev_region(dev_num, 1);	 
	printk(KERN_INFO "%s: 모듈을 언로드한다.\n", DRIVER_NAME);
	printk(KERN_INFO "%s: USB 드라이버가 등록 해제되었다.\n", DRIVER_NAME);
}

module_init(custom_init);
module_exit(custom_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jiwan Seo");
MODULE_DESCRIPTION("Custom USB device identification test driver");

