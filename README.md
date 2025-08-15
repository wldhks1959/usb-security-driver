### **USB 보안 드라이버**
 
 이 프로젝트는 리눅스 환경에서 특정 USB 저장 장치에 대한 보안 기능을 구현하는 커널 모듈을 개발하는 것을 목표로 한다.

#### **요약 및 최종 결과**

오늘까지의 과정을 통해, 특정 USB 장치 연결 시 `probe` 함수가 정상적으로 호출되는 것을 확인했다. 여러 문제를 해결한 끝에 **"Found taget USB device! Driver controls this device."** 라는 로그를 성공적으로 출력했다.

---

### **프로젝트 파일 구조**
/usb-security-driver  
├── README.md  
└── src/  
├── drivers/  
│   ├── Kconfig.patch  
│   └── Makefile.patch  
└── usb_security/  
├── Kconfig  
├── Makefile  
└── main.c  


