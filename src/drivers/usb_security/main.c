#include <linux/module.h>
#include <linux/init.h>
#include <linux/usb.h>

#define DRIVER_NAME "custom_usb_security"

#define USB_DEV_VENDOR_ID 0x0781 // sandisk
#define USB_DEV_PRODUCT_ID 0x5581 // sandisk's ultra

// USB Device Driver가 지원하는 Device list
// 이 테이블의 VID와 PID를 가진 장치가 연결되면 드라이버의 probe 함수가 호출된다.

static const struct usb_device_id custom_usb_table[] = {
	{ USB_DEVICE(USB_DEV_VENDOR_ID, USB_DEV_PRODUCT_ID) },
	{ } // 테이블의 끝을 알리는 빈 엔트리
};

// usb device가 system에 연결 시 호출되는 함수
// 이 함수에서 특정 장치임을 확인하고 드라이버를 초기화한다.
static int custom_probe(struct usb_interface *interface, const struct usb_device_id *id){
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
	
	return 0;
}

// USB 장치가 시스템에서 분리될 때 호출되는 함수
static void custom_disconnect(struct usb_interface *interface){
	printk(KERN_INFO "%s: USB device is disconnected.\n", DRIVER_NAME);
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
	printk(KERN_INFO "%s: Load a module.\n", DRIVER_NAME);
	ret = usb_register(&custom_driver);
	if (ret) {
		printk(KERN_ALERT "%s: USB Driver 등록 failure. (%d)\n", DRIVER_NAME, ret);
		return ret;
	}
	printk(KERN_INFO "%s: USB 드라이버가 등록되었다. USB 장치를 기다린다..\n", DRIVER_NAME);
	return 0;
}

// 모듈 언로드 시 호출되는 종료 함수.
static void __exit custom_exit(void){
	printk(KERN_INFO "%s: 모듈을 언로드한다.\n", DRIVER_NAME);
	usb_deregister(&custom_driver);	 
	printk(KERN_INFO "%s: USB 드라이버가 등록 해제되었다.\n", DRIVER_NAME);
}

module_init(custom_init);
module_exit(custom_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jiwan Seo");
MODULE_DESCRIPTION("Custom USB device identification test driver");

