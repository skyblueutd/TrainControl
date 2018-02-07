#include <windows.h>
#include <stdio.h>

void setCommand(char* p, char str) {
	// three-byte format: 11111110 - 00AAAAAA - ACCDDDDD
	// if the engine is addressed with 23 (in binary: 10111), then AAAAAAA will be 0010111

	// the first byte is OFE
	*p |= 1 << 1;
	*p |= 1 << 2;
	*p |= 1 << 3;
	*p |= 1 << 4;
	*p |= 1 << 5;
	*p |= 1 << 6;
	*p |= 1 << 7;
	// p[0] = 0xFE;

	// the second byte: 00AAAAAA
	//engine addressing code:   000010111
	*(p + 1) |= 1 << 0;
	*(p + 1) |= 1 << 1;
	*(p + 1) |= 1 << 3;

	// the third byte: 1CCDDDDD
	*(p + 2) |= 1 << 7;
	// p[1] = 0x0B;

	switch (str) {
		// remain part for the third byte
	case '1': // speed set 8:   1101000
		
		*(p + 2) |= 1 << 3;
		*(p + 2) |= 1 << 5;
		*(p + 2) |= 1 << 6;
		break;

	case '2': // speed set 0:   1100000
	case '6':
		*(p + 2) |= 1 << 5;
		*(p + 2) |= 1 << 6;
		break;

	case '3': // speed + 3:   1001000 
		
		*(p + 2) |= 1 << 3;
		*(p + 2) |= 1 << 6;
		break;

	case '4': // speed - 3:   1000010
	    *(p + 2) |= 1 << 1;
		*(p + 2) |= 1 << 6;
		break;

	case '5': // h - horn 1:   0011100 -> 0011101
		*(p + 2) |= 1 << 2;
		*(p + 2) |= 1 << 3;
		*(p + 2) |= 1 << 4;
		
		*(p + 2) |= 1;
		break;

	/*case '6': // nothing need to be done to the train
		break;*/

	default: 
		fprintf(stderr, "Error.   Please input valid command.\n");
		break;
	}


}

int main()
{
	// Declare variables and structures
	HANDLE hSerial;
	DCB dcbSerialParams = { 0 };
	COMMTIMEOUTS timeouts = { 0 };

	// Open the right serial port number, normally on this computer should be COM1
	fprintf(stderr, "Opening serial port...");

	hSerial = CreateFile(
		"\\\\.\\COM1", GENERIC_READ | GENERIC_WRITE, 0, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hSerial == INVALID_HANDLE_VALUE)
	{
		fprintf(stderr, "Error\n");
		return 1;
	}
	else fprintf(stderr, "OK\n");


	// Set device parameters (9600 baud, 1 start bit,
	// 1 stop bit, no parity)
	dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
	if (GetCommState(hSerial, &dcbSerialParams) == 0)
	{
		fprintf(stderr, "Error getting device state\n");
		CloseHandle(hSerial);
		return 1;
	}

	dcbSerialParams.BaudRate = CBR_9600;
	dcbSerialParams.ByteSize = 8;
	dcbSerialParams.StopBits = ONESTOPBIT;
	dcbSerialParams.Parity = NOPARITY;
	if (SetCommState(hSerial, &dcbSerialParams) == 0)
	{
		fprintf(stderr, "Error setting device parameters\n");
		CloseHandle(hSerial);
		return 1;
	}

	// Set COM port timeout settings
	timeouts.ReadIntervalTimeout = 50;
	timeouts.ReadTotalTimeoutConstant = 50;
	timeouts.ReadTotalTimeoutMultiplier = 10;
	timeouts.WriteTotalTimeoutConstant = 50;
	timeouts.WriteTotalTimeoutMultiplier = 10;
	if (SetCommTimeouts(hSerial, &timeouts) == 0)
	{
		fprintf(stderr, "Error setting timeouts\n");
		CloseHandle(hSerial);
		return 1;
	}

	printf("Supported commands are: \n1-start\t\t2-stop\t\t3-accelerate\n\t\t4-decelerate\t\t5-horn\n");
	printf("To end the program, please input 6\n");
	char str;
	int speed = 0;// [0 - 31]
	char bytes_to_send[3];
	
	while(1){
		printf("Input command:\n");
		scanf(" %c", &str);
		// Each command is in a three-byte command
		bytes_to_send[0] = 0;
		bytes_to_send[1] = 0;
		bytes_to_send[2] = 0;

		if(speed>=25&&str=='3'||speed<=0&&str=='4'){printf("Speed Out Of Range, resetting mow...\n");continue;}
		setCommand(bytes_to_send, str);
			// Send specified text (remaining command line arguments)
	DWORD bytes_written, total_bytes_written = 0;
	fprintf(stderr, "Sending bytes...");
	if (!WriteFile(hSerial, bytes_to_send, 3, &bytes_written, NULL))
	{
		fprintf(stderr, "Error\n");
		CloseHandle(hSerial);
		return 1;
	}
	fprintf(stderr, "%d bytes written\n", bytes_written);

		//if(str=='6'){
			
		//	continue;
		//}

		if(str=='3'){
			speed += 3;
			//continue;
		}
		if(str=='4'){
			speed -= 3;
			//continue;
		}
		if(str=='1'){
			speed = 8;
			//continue;
		}
		if(str=='2'){
			speed = 0;
			//continue;
		}
		if(str=='5')
		{
			//continue;
		}
		if(str=='6'){
			printf("The train was stopped .\n");
			break;
		}
		printf("Current Speed:%d\n", speed);

	}

	

    // Close serial port
	fprintf(stderr, "Closing serial port...");
	if (CloseHandle(hSerial) == 0)
	{
		fprintf(stderr, "Error\n");
		return 1;
	}
	fprintf(stderr, "OK\n");

	// exit normally

	return 0;
}

// reference: https://batchloaf.wordpress.com/2013/02/13/writing-bytes-to-a-serial-port-in-c/
