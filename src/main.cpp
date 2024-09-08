#include <Arduino.h>
#include <BluetoothSerial.h>
#include <HardwareSerial.h>
#include "uRTCLib.h"

#define BUTTON_PIN 26

// uRTCLib rtc;
uRTCLib rtc(0x68);

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

const byte numChars = 80;
char receivedChars[numChars]; // an array to store the received data
String stringReceivedChars;
boolean newData = false;

HardwareSerial SerialPort(2); // if using UART2
BluetoothSerial SerialBT;

// String MACadd = "AA:BB:CC:11:22:33";
//uint8_t address[6] = {0x66, 0x32, 0x06, 0x60, 0xF6, 0xE5};
uint8_t address[6]    = {0xDC, 0x0D, 0x30, 0x00, 0x28, 0x89};
// String name = "Printer001";
char *pin = "0000"; //<- standard pin would be provided by default
bool connected;

void recvWithEndMarker()
{
	static byte ndx = 0;
	char endMarker = '\n';
	char rc;

	while (SerialPort.available() > 0 && newData == false)
	{
		rc = SerialPort.read();

		if (rc != endMarker)
		{
			receivedChars[ndx] = rc;
			ndx++;
			if (ndx >= numChars)
			{
				ndx = numChars - 1;
			}
		}
		else
		{
			receivedChars[ndx] = '\0'; // terminate the string
			ndx = 0;
			newData = true;
		}
	}
}

void showNewData()
{
	if (newData == true)
	{
		rtc.refresh();
		Serial.print("This just in ... ");
		stringReceivedChars = receivedChars;
		stringReceivedChars.trim();
		Serial.println(stringReceivedChars);
		if (stringReceivedChars == "------------------------")
		{
			newData = false;
			return;
		}
		if (stringReceivedChars == "........................")
		{
			newData = false;
			return;
		}
		if (stringReceivedChars == "Signature")
		{
			newData = false;
			return;
		}
		if (stringReceivedChars == "NO STABILITY")
		{
			SerialBT.println(stringReceivedChars);
			SerialBT.println("\x1B\x64\x4"); // hex code means ESC d 10 (feed 10 line)
			newData = false;
			return;
		}

		if (digitalRead(BUTTON_PIN) == HIGH)
		{
			SerialBT.println(stringReceivedChars);
			//SerialBT.println("\x1B\x64\x4"); // hex code means ESC d 10 (feed 10 line)
			newData = false;
			return;
		}
		else
		{
			SerialBT.print(rtc.day());
			SerialBT.print('/');
			SerialBT.print(rtc.month());
			SerialBT.print('/');
			SerialBT.print(rtc.year());

			SerialBT.print(" (");
			SerialBT.print(daysOfTheWeek[rtc.dayOfWeek() - 1]);
			SerialBT.print(") ");

			SerialBT.print(rtc.hour());
			SerialBT.print(':');
			if (rtc.minute() < 10)
				SerialBT.print('0');
			SerialBT.print(rtc.minute());
			SerialBT.print(':');
			if (rtc.second() < 10)
				SerialBT.print('0');
			SerialBT.println(rtc.second());
			SerialBT.println(stringReceivedChars);
			SerialBT.println("Signature");
			SerialBT.println("");
			SerialBT.println("------------------------");
			SerialBT.println("........................");
			SerialBT.println("\x1B\x64\x4"); // hex code means ESC d 10 (feed 10 line)
			newData = false;
			return;
		}
		newData = false;
	}
}

void setup()
{
	delay(3000);
	URTCLIB_WIRE.begin();
	Serial.begin(9600);
	pinMode(BUTTON_PIN, INPUT_PULLUP);
	SerialPort.begin(9600, SERIAL_8N1, 16, 17);
	Serial.println("START");
	pinMode(25, OUTPUT);
	digitalWrite(25, LOW);

	SerialBT.begin("ESP32test", true);
	SerialBT.setPin(pin);
	Serial.println("The device started in master mode, make sure remote BT device is on!");
	connected = SerialBT.connect(address);
	if (connected)
	{
		Serial.println("Connected Succesfully!");
		digitalWrite(25, HIGH); // turn the LED on
	}
	else
	{
		while (!SerialBT.connected(5000))
		{
			SerialBT.connect(address);
			Serial.println("Failed to connect. Make sure remote device is available and in range, then restart app.");
		}
	}
	//     this would reconnect to the name(will use address, if resolved) or address used with connect(name/address).
	SerialBT.connect();
}

void loop()
{
	
	recvWithEndMarker();
	showNewData();
	if (!SerialBT.connected(5000))
	{
		SerialBT.connect(address);
		digitalWrite(25, LOW);
	}
	else
	{
		digitalWrite(25, HIGH);
	}
}
