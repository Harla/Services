// services.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include<iostream>
#include<Windows.h>
using namespace std;
#define SERVICE_NAME TEXT("ASP Service") //Service Name
SERVICE_STATUS ServiceStatus = { 0 }; //Service Status Structure 
				      //Stores all information about the service
SERVICE_STATUS_HANDLE hServiceStatusHandle = NULL;//Service Status Handle to Register the Service
HANDLE hServiceEvent = NULL;//Event Handle for Service

//Windows Service Functions Declarations
void WINAPI ServiceMain(DWORD dwArgc, LPTSTR* lpArgv);// Service Main Function
void WINAPI ServiceControlHandler(DWORD dwControl); //Service Control Handler
void ServiceReportStatus(DWORD dwCurrentStatus, DWORD dwWin32ExitCode, DWORD dwWaitHint);
//Service Report Status
void ServiceInit(DWORD dwArgc, LPTSTR* lpArgv);//ServiceInit Fun
void ServiceInstall(void);//Service Install Fun
void ServiceDelete(void);//Service delete Fun
void ServiceStart(void);//Service start Fun
void ServiceStop(void);//Service Stop Fun

int main(int argc, CHAR* argv[])
{
	cout << "In main fun Start\n";
	//Local Variable Definition
	BOOL bStServiceCtrlDispatcher = FALSE;
	//Function Logic starts here
	//lstrcmpiA compares 2 strings
	if (lstrcmpiA(argv[1],"install")==0)
	{
	//Call Service Install Fun
		ServiceInstall();
	cout << "Installation Success\n";
	}
	else if (lstrcmpiA(argv[1],"Start")==0)
	{
		//Call Service start fun
		ServiceStart();
		cout << "ServiceStart success\n";
	}
	else if (lstrcmpiA(argv[1], "stop") == 0)
	{
		//Call Service Stop fun
		ServiceStop();
		cout << "ServiceStop Success\n";
	}
	else if (lstrcmpiA(argv[1], "delete") == 0)
	{
		//Call service delete fun
		ServiceDelete();
		cout << "ServiceDelete\n";
	}
	else
	{
		//Fill the Service Table Entry (2D Array)
		SERVICE_TABLE_ENTRY DispatchTable[] =
		{
			{(LPWSTR)SERVICE_NAME,(LPSERVICE_MAIN_FUNCTION)ServiceMain},
		{NULL, NULL}
		};

		//Start Service Control Dispatcher
		//Connects the main thread of a service process to the service control manager,
		//which causes the thread to be the service control dispatcher thread for 
		//the calling process
		bStServiceCtrlDispatcher = StartServiceCtrlDispatcher(DispatchTable);
		if(FALSE == bStServiceCtrlDispatcher)
		{
			cout << "StartServiceCtrlDispatcher Failed " << GetLastError() << "\n";

		}
		else
		{
			cout << "StartServiceCtrlDispatcher Success\n";
		}
	}
}
 
//ServiceMain Function Definition
void WINAPI ServiceMain(DWORD dwArgc, LPTSTR* lpArgv)
{
	cout << "Service Main Start\n";
	//Local Variable definition
	BOOL bServiceStatus = FALSE;
		//Step-1->Registering Service Control Handler Function to SCM
	//RegisterServiceCtrlHandler should be the first nonfailing function in ServiceMain 
	//so the service can use the status handle returned by this function to call SetServiceStatus 
	//with the SERVICE_STOPPED state if an error occurs.
									//A pointer to the handler function to be registered
	hServiceStatusHandle = RegisterServiceCtrlHandler(SERVICE_NAME, ServiceControlHandler);
	//Registers a function to handle service control requests.
	//If the function succeeds, the return value is a service status handle.
	//If the function fails, the return value is zero.
	if (NULL == hServiceStatusHandle)
		cout << "RegisterServiceCtrlHandler Failed " << GetLastError() << "\n";
	else
		cout << "RegisterServiceCtrlHandler Success " << GetLastError() << "\n";
	// Step-2->Service Status initial setup
	ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	ServiceStatus.dwServiceSpecificExitCode = 0;// error code the service uses to report an error that occurs when it is starting or stopping

	//Step-3->Call service report status for notifying initial setup
	ServiceReportStatus(SERVICE_START_PENDING, NO_ERROR, 3000);
								//Time			
	//Check the Service Status
	bServiceStatus = SetServiceStatus(hServiceStatusHandle, &ServiceStatus);

	if (FALSE == bServiceStatus)
	{
		cout << "Service Status initial setup FAILED\n";
	}
	else
		cout << "Service Status initial setup SUCCESS\n";
	//Call Service init function
	ServiceInit(dwArgc, lpArgv);
	cout << "ServiceMain End\n";
}

//Service control handler definition
void WINAPI ServiceControlHandler(DWORD dwControl)
{

	cout << "ServiceControlHandler\n";
	switch (dwControl)
	{
	case SERVICE_CONTROL_STOP:
	{
		//Call ServiceReportStatus Function
		ServiceReportStatus(SERVICE_STOPPED,NO_ERROR,0);
		cout << "Service stopped \n";
		break;
	}
	default: break;
	}
	cout << "ServiceControlHAndler\n";

}

void ServiceReportStatus(DWORD dwCurrentState, DWORD dwWin32ExitCode, DWORD dwWaitHint)
{
	cout << "ServiceReportStatus Start\n";
	//Local Variable Definitions
	static DWORD dwCheckPoint = 1;
	BOOL bSetServiceStatus = FALSE;
	//Filling the SERVICE_STATUS Structure
	ServiceStatus.dwCurrentState = dwCurrentState;
	ServiceStatus.dwWin32ExitCode = dwWin32ExitCode;
	ServiceStatus.dwWaitHint = dwWaitHint;
	//Check the current State of the service
	if (dwCurrentState == SERVICE_START_PENDING)//Service is about to start
	{
		ServiceStatus.dwControlsAccepted = 0;
	}
	else
	{
		ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
	}
	//Progress for the Service Operation
	if ((dwCurrentState == SERVICE_RUNNING) || (dwCurrentState == SERVICE_STOPPED))
	{
		ServiceStatus.dwCheckPoint = 0;
	}
	else
	{
		ServiceStatus.dwCheckPoint = dwCheckPoint++;
	}
	//Notify the current status  of SCM
	bSetServiceStatus = SetServiceStatus(hServiceStatusHandle, &ServiceStatus);
	if (FALSE == bSetServiceStatus)
	{
		cout << "Service Status Failed = " << GetLastError() << "\n";

	}
}

//Service Init function definition
void ServiceInit(DWORD dwArgc, LPTSTR* lpArgv)
{
	cout << "ServiceInit Start\n";
	//Create Event
	hServiceEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (NULL == hServiceEvent)
	{
		//Call ServiceReportStatus function to notify the SCM for Current Status of Service
		ServiceReportStatus(SERVICE_STOPPED, NO_ERROR, 0);

	}
	else
	{
		//Call the ServiceReportStatus Function to notify the SCM for Current Status of service
		ServiceReportStatus(SERVICE_RUNNING, NO_ERROR, 0);
	}
	//Check whether to stop the Service

	while (1)
	{
		//Wait for SingleObject which wait event o be signaled
		WaitForSingleObject(hServiceEvent,INFINITE);
		//Send report Status to SCM
		ServiceReportStatus(SERVICE_STOPPED, NO_ERROR, 0);	

	}
	cout << "ServiceInit function End\n";
}
//ServiceInstall Definition
void ServiceInstall(void)
{
	cout << "ServiceInstall Start\n";
	//Local Variable Definition
	SC_HANDLE hScOpenSCManager = NULL;
	SC_HANDLE hScCreateService = NULL;
	DWORD dwGetModuleFileName = 0;
	TCHAR szPath[MAX_PATH];
	//GetModuleFileNAme Get the executable file from SCM
	dwGetModuleFileName = GetModuleFileName(NULL, szPath, MAX_PATH);
	if (0 == dwGetModuleFileName)
	{
		cout << "Service Installation Failed =" << GetLastError() << "\n";
	}
	else
		cout << "Successfully installed the FIle\n";
	//Open the Service Control MAnager
	hScOpenSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (NULL == hScOpenSCManager)
		cout << "OpenScManager failed\n";
	else
		cout << "OpenScManager Success\n";
	//Create a service
	hScCreateService = CreateService(hScOpenSCManager, SERVICE_NAME, SERVICE_NAME, SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS, SERVICE_DEMAND_START, SERVICE_ERROR_NORMAL,szPath,NULL,NULL,NULL,NULL,NULL);
	if (NULL == hScCreateService)
	{
		cout << "CreateServie failed\n";
		CloseServiceHandle(hScOpenSCManager);
	}
	else
		cout << "CreateService Success\n";
	//CLose the handle for OpenScMAnager and CreateService
	CloseServiceHandle(hScCreateService);
	CloseServiceHandle(hScOpenSCManager);
	
}

void ServiceDelete(void)
{
	cout << "ServiceDelete Start\n";
	//Local Variable Definition
	SC_HANDLE hScOpenSCManager = NULL;
	SC_HANDLE hScOpenService = NULL;
	BOOL bDeleteService = FALSE;
	//Open the Service Control Manager

	hScOpenSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (NULL == hScOpenSCManager)
	{
		cout << "OpenSCManager Failed =" << GetLastError() << "\n";

	}
	else
		cout << "OpenSCMAnager Success\n";
	//Open the Service
	hScOpenService = OpenService(hScOpenSCManager, SERVICE_NAME, SERVICE_ALL_ACCESS);
	if (NULL == hScOpenService)
		cout << "OpenService failed " << GetLastError() << "\n";
	else
		cout << "OpenService success\n";
	//Delete the Service
	bDeleteService = DeleteService(hScOpenService);
	if (FALSE == bDeleteService)
	{
		cout << "Delete Service failed\n" << GetLastError() << "\n";
	}
	else
		cout << "Delete Service Success\n";
}

void ServiceStart(void)
{
	cout << "Inside ServiceStart Function\n";
	//Local Variable Definitions
	BOOL bStartService = FALSE;
	SERVICE_STATUS_PROCESS SvcStatusProcess;
	SC_HANDLE hOpenSCManager = NULL;
	SC_HANDLE hOpenService = NULL;
	BOOL bQueryServiceStatus = FALSE;
	DWORD dwBytesNeeded;
	//Open Service MAnager
	hOpenSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (NULL == hOpenSCManager)
		cout << "hOpenSCManager failed " << GetLastError() << "\n";
	else
		cout << "hOpenScManager Success\n";
	//OpenService
	hOpenService = OpenService(hOpenSCManager, SERVICE_NAME, SC_MANAGER_ALL_ACCESS);
	if (NULL == hOpenService)
	{
		cout << "OpenService failed" << GetLastError() << "\n";
		CloseServiceHandle(hOpenSCManager);
	}
	else
		cout << "OpenService success\n";
	//Query about current service status
	bQueryServiceStatus = QueryServiceStatusEx(hOpenService, SC_STATUS_PROCESS_INFO, (LPBYTE)& SvcStatusProcess, sizeof(SERVICE_STATUS_PROCESS), &dwBytesNeeded);
	if (FALSE == bQueryServiceStatus)
	{
		cout << "QueryService failed " << GetLastError() << "\n";
	}
	else
		cout << "QueryService success\n";
	//Check if service is running or stopped
	if ((SvcStatusProcess.dwCurrentState!=SERVICE_STOPPED) && (SvcStatusProcess.dwCurrentState !=SERVICE_STOP_PENDING ))
	{
		cout << "Service is already running\n";
	}
	else
	{
		cout << "Service is already stopped\n";
	}
	//If service is stopped then query the service
	while (SvcStatusProcess.dwCurrentState == SERVICE_STOP_PENDING)
	{
		bQueryServiceStatus = QueryServiceStatusEx(hOpenService, SC_STATUS_PROCESS_INFO, (LPBYTE)& SvcStatusProcess, sizeof(SERVICE_STATUS_PROCESS), &dwBytesNeeded);
		if (FALSE == bQueryServiceStatus)
			cout << "QueryService Failed " << GetLastError() << "\n";
		else
			cout << "QueryService success\n";
	}
	//Start the service
	bStartService = StartService(hOpenService, NULL, NULL);
	if (FALSE == bStartService)
	{
		cout << "StartService failed= " << GetLastError() << "\n";
		CloseServiceHandle(hOpenService);
		CloseServiceHandle(hOpenSCManager);
	}
	else
		cout << "StartService success\n";
	//Query the service again
	bQueryServiceStatus = QueryServiceStatusEx(hOpenService, SC_STATUS_PROCESS_INFO, (LPBYTE)& SvcStatusProcess, sizeof(SERVICE_STATUS_PROCESS), &dwBytesNeeded);
	if (FALSE == bQueryServiceStatus)
	{
		cout << "QueryService failed " << GetLastError() << "\n";
	}
	else
		cout << "QueryService success\n";
	//Check if service is running or not
	if (SvcStatusProcess.dwCurrentState == SERVICE_RUNNING)
		cout << "Service started running\n";
	else
	{
		cout << "Service running failed " << GetLastError() << "\n";
		CloseServiceHandle(hOpenService);
		CloseServiceHandle(hOpenSCManager);
	}
	//Close the service handlefor OpenSCManager& OpenService
	CloseServiceHandle(hOpenService);
	CloseServiceHandle(hOpenSCManager);
	cout << "ServiceStart end\n"; 
}
void ServiceStop(void)
{
	cout << "Inside ServiceStop\n";
	//Local Variable Definitions
	BOOL bStartService = FALSE;
	SERVICE_STATUS_PROCESS SvcStatusProcess;
	SC_HANDLE hScOpenSCManager = NULL;
	SC_HANDLE hScOpenService = NULL;
	BOOL bQueryServiceStatus = TRUE;
	BOOL bControlService = TRUE;
	DWORD dwBytesNeeded;
	//Open Service Control MAnager
	hScOpenSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (NULL == hScOpenSCManager)
		cout << "hOpenSCManager failed " << GetLastError() << "\n";
	else
		cout << "hOpenScManager Success\n";
	//OpenService
	hScOpenService = OpenService(hScOpenSCManager, SERVICE_NAME, SC_MANAGER_ALL_ACCESS);
	if (NULL == hScOpenService)
	{
		cout << "OpenService failed" << GetLastError() << "\n";
		CloseServiceHandle(hScOpenSCManager);
	
	}
	else
		cout << "OpenService success\n";
	//Query service status
	bQueryServiceStatus = QueryServiceStatusEx(hScOpenService, SC_STATUS_PROCESS_INFO, (LPBYTE)& SvcStatusProcess, sizeof(SERVICE_STATUS_PROCESS), &dwBytesNeeded);
	if (FALSE == bQueryServiceStatus)
	{
		cout << "QueryService failed " << GetLastError() << "\n";
		CloseServiceHandle(hScOpenService);
		CloseServiceHandle(hScOpenSCManager);
	}
	else
		cout << "QueryService success\n";
	//Send a stop code to the Service Control Manager
	bControlService = ControlService(hScOpenService, SERVICE_CONTROL_STOP, (LPSERVICE_STATUS)& SvcStatusProcess);
	if (TRUE == bControlService)
		cout << "Control Service success\n";
	else
	{
		cout << "Control Service failed " << GetLastError() << "\n";
		CloseServiceHandle(hScOpenService);
	}
	//Step 5 :Wait for service to stop
	while (SvcStatusProcess.dwCurrentState != SERVICE_STOPPED)
	{
		//Step 6: Inside while loop query the service
		bQueryServiceStatus = QueryServiceStatusEx(hScOpenService, SC_STATUS_PROCESS_INFO, (LPBYTE)& SvcStatusProcess, sizeof(SERVICE_STATUS_PROCESS), &dwBytesNeeded);
		if (TRUE == bQueryServiceStatus)
		{
			cout << "QueryService Failed= " << GetLastError() << "\n";
			CloseServiceHandle(hScOpenService);
			CloseServiceHandle(hScOpenSCManager);
		}
		else
			cout << "QueryService success\n";
		//Step 7 Inside while loop check the current state of service
		if (SvcStatusProcess.dwCurrentState == SERVICE_STOPPED)
		{
			cout << "Service Stopped successfully\n";
			break;
		}
		else
		{
			cout << "Service Stop Failed  " << GetLastError() << "\n";
		}
	}//end of while
	//Step 8 Close the handle  for Open SCM and Open Service
	CloseServiceHandle(hScOpenSCManager);
	CloseServiceHandle(hScOpenService);
	cout << "Service stop\n";

	
}
