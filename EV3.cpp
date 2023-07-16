#include "EV3.hpp"
#include <iostream>
#include <string>

EV3::EV3() {
}
void EV3::connect(unsigned int comPort)
{
	bool connected = 1;

	std::string fileName = R"(\\.\COM)" + std::to_string(comPort);

	bluetoothHandle = CreateFileA(fileName.c_str(),
		GENERIC_READ | GENERIC_WRITE,
		0,    // comm devices must be opened w/exclusive-access
		NULL, // no security attributes
		OPEN_EXISTING, // comm devices must use OPEN_EXISTING
		0,    // overlapped I/O
		NULL  // hTemplate must be NULL for comm devices
	);

	if (bluetoothHandle == INVALID_HANDLE_VALUE)
		connected = 0;

	bool fSuccess = SetCommMask(bluetoothHandle, EV_CTS | EV_DSR);
	if (!fSuccess) {
		bluetoothHandle = INVALID_HANDLE_VALUE;
		connected = 0;
	}

	OVERLAPPED o;
	o.hEvent = CreateEvent(
		NULL,   // default security attributes
		TRUE,   // manual-reset event
		FALSE,  // not signaled
		NULL    // no name
	);

	o.Internal = 0;
	o.InternalHigh = 0;
	o.Offset = 0;
	o.OffsetHigh = 0;

	COMMTIMEOUTS CommTimeOuts;
	CommTimeOuts.ReadIntervalTimeout = 3;
	CommTimeOuts.ReadTotalTimeoutMultiplier = 1;
	CommTimeOuts.ReadTotalTimeoutConstant = 2;
	CommTimeOuts.WriteTotalTimeoutMultiplier = 0;
	CommTimeOuts.WriteTotalTimeoutConstant = 0;

	SetCommTimeouts(bluetoothHandle, &CommTimeOuts);

	DWORD dwEvtMask;
	if (!WaitCommEvent(bluetoothHandle, &dwEvtMask, &o))
	{
		bluetoothHandle = INVALID_HANDLE_VALUE;
		connected = 0;
	}

	if (!connected) {
		std::cout << "Error connecting to EV3" << std::endl;
		system("pause");
		exit(0);
	}
	else
	{
		std::cout << "Connected to EV3" << std::endl;
		motor(EV3::MOTOR_A, 0);
		motor(EV3::MOTOR_B, 0);
		motor(EV3::MOTOR_C, 0);
		motor(EV3::MOTOR_D, 0);
	}
}

void EV3::sendMessage(uint8_t* data, int size)
{
	if (bluetoothHandle == INVALID_HANDLE_VALUE) return;
	DWORD bc;
	do
	{
		WriteFile(bluetoothHandle, data, size, &bc, NULL);
		Sleep(1);
	} while (bc == 0);
}

void EV3::motor(int Motor, int Speed) {
	if (Speed > 127) Speed = 127;
	if (Speed < -127) Speed = -127;
	
	uint8_t Commands[] = { opOUTPUT_SPEED, LC0(0), LC0(Motor), LC1(Speed) };

	uint8_t fCommands[] = { opOUTPUT_START, LC0(0), LC0((Motor)) };
	
	int typeStop = 0; // 0 - Float, 1 - Break

	uint8_t CommandStop[] = { opOUTPUT_STOP, LC0(0), LC0(Motor), LC0(typeStop) };
	

	int endSize = 5, pos;

	if (Speed == 0) {
		endSize += sizeof(CommandStop);
	}
	else {
		endSize += sizeof(Commands);
	}


	if (Speed != 0) {
		endSize += sizeof(fCommands);
	}

	uint8_t* fin = new uint8_t[endSize + 2];

	((uint16_t*)fin)[0] = endSize;
	((uint16_t*)fin)[1] = 1;
	fin[4] = 0x80;
	fin[5] = fin[6] = 0;
	pos = 7;

	if (Speed == 0) {
		memcpy(&fin[pos], CommandStop, sizeof(CommandStop));
		pos += sizeof(CommandStop);
	}
	else {
		memcpy(&fin[pos], Commands, sizeof(Commands));
		pos += sizeof(Commands);
	}

	if (Speed != 0) {
		memcpy(&fin[pos], fCommands, sizeof(fCommands));
		pos += sizeof(fCommands);
	}
	sendMessage(fin, pos);
	delete[] fin;
	Sleep(1);
}
EV3::~EV3()
{
	motor(EV3::MOTOR_A, 0);
	motor(EV3::MOTOR_B, 0);
	motor(EV3::MOTOR_C, 0);
	motor(EV3::MOTOR_D, 0);
	Sleep(100);
	CloseHandle(bluetoothHandle);
	Sleep(1000);
}