#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include "hello_service.h"

void
run_service() {
  SERVICE_TABLE_ENTRY service_table[] = {
    { SERVICE_NAME, service_main },
    { 0, 0 }
  };

  if(!StartServiceCtrlDispatcher(service_table)) {
    log("Service start fucked up.");
  }
}

void WINAPI
service_main(DWORD argc, LPSTR *argv) {
  initialize_service_status();
  if(service_status_handle) {
    start_service();
    for(; WaitForSingleObject(stop_service_event, 5000) != WAIT_TIMEOUT;) {
      log("zbs");
    }
    stop_service();
  }
}

void
initialize_service_status() {
  service_status.dwServiceType = SERVICE_WIN32;
  service_status.dwCurrentState = SERVICE_STOPPED;
  service_status.dwControlsAccepted = 0;
  service_status.dwWin32ExitCode = NO_ERROR;
  service_status.dwServiceSpecificExitCode = NO_ERROR;
  service_status.dwCheckPoint = 0;
  service_status.dwWaitHint = 0;

  service_status_handle = RegisterServiceCtrlHandler(SERVICE_NAME,
    service_control_handler);
  log("Service status initialized.");
}

void WINAPI
service_control_handler(DWORD control_code) {
  switch(control_code) {
    case SERVICE_CONTROL_INTERROGATE:
      break;
    case SERVICE_CONTROL_STOP:
    case SERVICE_CONTROL_SHUTDOWN:
      on_stop();
      return;
    case SERVICE_CONTROL_PAUSE:
      break;
    case SERVICE_CONTROL_CONTINUE:
      break;
    default:
      break;
  }
}

void
on_stop() {
  service_status.dwCurrentState = SERVICE_STOP_PENDING;
  set_default_service_status();
  SetEvent(stop_service_event);
}

void
start_service() {
  service_status.dwCurrentState = SERVICE_START_PENDING;
  set_default_service_status();

  stop_service_event = CreateEvent(0, FALSE, FALSE, 0);

  service_status.dwControlsAccepted |=
    (SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN);
  service_status.dwCurrentState = SERVICE_RUNNING;
  set_default_service_status();
}

void
stop_service() {
  service_status.dwCurrentState = SERVICE_STOP_PENDING;
  set_default_service_status();

  CloseHandle(stop_service_event);
  stop_service_event = 0;

  service_status.dwControlsAccepted &=
    ~(SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN);
  service_status.dwCurrentState = SERVICE_STOPPED;
  set_default_service_status();
}

void
set_default_service_status() {
  SetServiceStatus(service_status_handle, &service_status);
}

int log(char *str) {
  FILE *log = fopen(LOG_FILE, "a+");
  if(log == NULL) { return -1; }
  fprintf(log, "%s\n", str);
  fclose(log);
  return 0;
}

void
install_service() {
  SC_HANDLE service_control_manager = OpenSCManager(0, 0,
    SC_MANAGER_CREATE_SERVICE);
  if(!service_control_manager) { return; }

  char path[500];
  if(GetModuleFileName(0, path, sizeof(path) / sizeof(path[0])) > 0) {
    SC_HANDLE service = CreateService(service_control_manager, SERVICE_NAME,
        SERVICE_NAME, SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS,
        SERVICE_AUTO_START, SERVICE_ERROR_CRITICAL, path, 0, 0, 0, 0, 0);
    if(service) {
      CloseServiceHandle(service);
    }
  }
  CloseServiceHandle(service_control_manager);
  log("Service installed.");
}

void
uninstall_service() {
  SC_HANDLE service_control_manager = OpenSCManager(0, 0,
    SC_MANAGER_CONNECT);
  if(!service_control_manager) { return; }

  SC_HANDLE service = OpenService(service_control_manager, SERVICE_NAME,
      SERVICE_QUERY_STATUS | DELETE);
  if(service) {
    SERVICE_STATUS service_status;
    if(QueryServiceStatus(service, &service_status)) {
      if(service_status.dwCurrentState == SERVICE_STOPPED) {
        DeleteService(service);
      }
    }
    CloseServiceHandle(service);
  }
  CloseServiceHandle(service_control_manager);
  log("Service uninstalled.");
}

int
main(int argc, char **argv) {
  if(argc < 2) {
    log("Running service...");
    run_service();
    return 0;
  }
  if(!strcmp(argv[1], "install")) { install_service(); }
  else if(!strcmp(argv[1], "uninstall")) { uninstall_service(); }
  return 0;
}
