### **USB 보안 드라이버**
 
 이 프로젝트는 리눅스 환경에서 특정 USB 저장 장치에 대한 보안 기능을 구현하는 커널 모듈을 개발하는 것을 목표로 한다.

#### **요약 및 최종 결과**

특정 USB 장치 연결 시 `probe` 함수가 정상적으로 호출되는 것을 확인했다. 여러 문제를 해결한 끝에 **"Found taget USB device! Driver controls this device."** 라는 로그를 성공적으로 출력했다.

USB 드라이버이지만, 사용자 프로그램과 커널 모듈 간의 통신을 위해 Char Device Driver의 뼈대를 활용했다.

USB 보안 드라이버의 핵심 기능인 ioctl을 성공적으로 구현하고 테스트했다. 드라이버와 사용자 프로그램 간의 ioctl 통신을 통해 비밀번호를 설정, 검증, 그리고 초기화하는 모든 기능이 완벽하게 동작함을 확인했다.

---

### **프로젝트 파일 구조**
/usb-security-driver  
├── README.md  
└── src  
    ├── drivers  
    │   ├── Kconfig.patch  
    │   ├── Makefile.patch  
    │   └── usb_security  
    │       ├── Kconfig  
    │       ├── Makefile  
    │       └── main.c  
    └── user_program  
        └── test_ioctl.c  



