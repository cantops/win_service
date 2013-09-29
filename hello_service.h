#define SLEEP_TIME 5000
#define LOG_FILE "sample_service.log"
#define SERVICE_NAME "SampleService"

SERVICE_STATUS service_status;
SERVICE_STATUS_HANDLE service_status_handle = 0;
HANDLE stop_service_event = 0;

void run_service();
void WINAPI service_main(DWORD, LPSTR *);
void initialize_service_status();
void WINAPI service_control_handler(DWORD control_code);
void on_stop();
void start_service();
void stop_service();
void set_default_service_status();
void install_service();
void uninstall_service();
int log(char*);
