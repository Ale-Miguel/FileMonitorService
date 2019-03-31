// FileMonitorService.cpp : Este archivo contiene la función "main". La ejecución del programa comienza y termina ahí.
//

#include "pch.h"
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>

#define MAX_STRING_LENGTH 100
wchar_t wtext[MAX_STRING_LENGTH];

int main() {
	char dirStr[] = "C:\\Users\\ProgAva\\Temp";
	LPWSTR LdirStr;
	char dirFileStr[] = "C:\\Users\\ProgAva\\Temp\\Algo.txt";
	LPWSTR LdirFileStr;
	char dirFileStr2[] = "C:\\Users\\ProgAva\\Temp\\Algo2.txt";
	LPWSTR LdirFileStr2;
	HANDLE changeNotifHandle;
	HANDLE fileHandle;
	char chr = 0;
	DWORD waitResult;
	int flag = 0;
	FILETIME lastWriteTime, t1, t2, t3;
	DWORD lastError;

	//Asegurar tiempos de facto para el archivo
	lastWriteTime.dwHighDateTime = 0;	//Asignar los 32bits HIGH
	lastWriteTime.dwLowDateTime = 0;	//Asignar los 32bits LOW

	mbstowcs(wtext, dirFileStr, strlen(dirFileStr) + 1);//Plus null
	LdirFileStr = wtext;

	//Abrir un "handle" con el archivo para obtener (inicialmente) el tiempo de ultima escritura
	fileHandle = CreateFile(LdirFileStr, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	//Si el 
	if (fileHandle != INVALID_HANDLE_VALUE) {
		if (GetFileTime(fileHandle, &t1, &t2, &t3))
			memcpy(&lastWriteTime, &t3, sizeof(FILETIME));
		CloseHandle(fileHandle);
	}
	else {
		lastError = GetLastError();
		if (lastError == ERROR_FILE_NOT_FOUND)
			return 0;	//Archivo no localizado, quizas lo borraron ... terminar el programa
		//Otra posibilidad del error es que el archivo este abierto y LOCKED ...
		// ... habr� que decidir como proceder (terminar el programa tambi�n o ciclarse esperando)
		// ... pero esto no lo vamos a hacer aqu� simplemente tomamos valores de tiempo en ceros.
	}

	mbstowcs(wtext, dirStr, strlen(dirStr) + 1);//Plus null
	LdirStr = wtext;

	while (chr != 27) {
		if (!flag)
			changeNotifHandle = FindFirstChangeNotification(LdirStr, FALSE, FILE_NOTIFY_CHANGE_LAST_WRITE);
		else
			FindNextChangeNotification(changeNotifHandle);
		waitResult = WaitForSingleObject(changeNotifHandle, 500);	//Esperar hasta 500 mseg.
		flag = 1;
		switch (waitResult) {
		case WAIT_OBJECT_0:
			printf("Cambio algun archivo\n");
			Sleep(50);	//Dar un tiempo para evitar el "LOCK" cuando se esta grabando el archivo

			mbstowcs(wtext, dirFileStr, strlen(dirFileStr) + 1);//Plus null
			LdirFileStr = wtext;

			fileHandle = CreateFile(LdirFileStr, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
			if (fileHandle != INVALID_HANDLE_VALUE) {
				if (GetFileTime(fileHandle, &t1, &t2, &t3)) {
					//printf("Time: %ld, %ld\n",t3.dwHighDateTime,t3.dwLowDateTime);
					if ((t3.dwHighDateTime != lastWriteTime.dwHighDateTime) || (t3.dwLowDateTime != lastWriteTime.dwLowDateTime)) {
						printf("El archivo cambio\n");
						memcpy(&lastWriteTime, &t3, sizeof(FILETIME));
						//unlink(dirFileStr2);
						
						mbstowcs(wtext, dirFileStr2, strlen(dirFileStr2) + 1);//Plus null
						LdirFileStr2 = wtext;

						CopyFile(LdirFileStr, LdirFileStr2, FALSE);
					}
					else {
						printf("El archivo NO cambio\n");
					}
				}
				CloseHandle(fileHandle);
			}
			else {
				lastError = GetLastError();
				if (lastError == ERROR_SHARING_VIOLATION)	//�Ups!, que mala suerte el archivo cambio y esta LOCKED
					printf("El archivo cambio!\n");
				else if (lastError == ERROR_FILE_NOT_FOUND) {
					printf("Borraron el achivo ... que hacemos?\n");
					return 0;		//Por lo pronto finalizar este programa
				}
				else
					printf("INVALID_HANDLE_VALUE! ... %ld\n", lastError);
			}
			break;
		case WAIT_TIMEOUT:
			printf(".");
			break;
		default:
			printf("ERROR!!!\n");
			break;
		}
		if (_kbhit())
			chr = _getch();
	}
	FindCloseChangeNotification(changeNotifHandle);
	printf("Listo\n");

	return 0;
}