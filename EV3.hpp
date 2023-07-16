#include "bytecodes.hpp"
#include <cstdint>
#include <Windows.h>


class EV3 {
private:
	HANDLE bluetoothHandle;

	void sendMessage(uint8_t* data, int size);
public:
	enum MOTORS {
		MOTOR_A = 0x01,
		MOTOR_B = 0x02,
		MOTOR_C = 0x04,
		MOTOR_D = 0x08
	};

	EV3();
	void connect(unsigned int comPort);
	void motor(int Motor, int Speed);
	~EV3();
};