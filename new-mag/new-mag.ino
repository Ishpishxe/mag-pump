#define DisplaySerial Serial1
//-------Picaso DISPLAYS-------

#include <Picaso_Const4D.h>
#include <Picaso_Serial_4DLib.h>

//use Serial0 to communicate with the display.
Picaso_Serial_4DLib Display(&DisplaySerial);
//---------END-----------------

bool menu;
bool start;
bool settings;
bool runn;

bool current;
bool position;
bool velocity;
bool sendtemp;
bool config;
bool killit;
bool overheat;

int temp;
int x, y;
int counter;
int T;
int Rt;

int itr = 0;
int table1[13] = { 20, 25, 30, 40, 50, 60, 70, 80, 90, 100, 110, 120, 130 };
int table2[13] = { 972, 1010, 1049, 1130, 1214, 1301, 1392, 1487, 1585, 1687,
		1792, 1900, 2012 };
int temps[40];

long R1 = 11803;
long R2 = 4596;

double G;
double Req;
double Vint;
double Voutt;

float KTYadj = 1000;

int average() {
	int sum;
	for (int i = 0; i < 40; i++) {
		sum += temps[i];
	}
	return sum / 40;
}

float getTemperature(double R) {
	if (R < table2[0]) {
		return 20;
	}
	for (int i = 0; i < 12; i++) {
		if ((R > table2[i]) && (R < table2[i + 1])) {
			overheat = false;
			return ((table1[i + 1] - table1[i]) * (R - table2[i])
					/ (table2[i + 1] - table2[i])) + table1[i];
		}
		if (R == table2[i]) {
			overheat = false;
			return table1[i];
		}
		if (R == table2[i + 1]) {
			overheat = false;
			return table1[i + 1];
		}
	}
	killit = true;
	overheat = true;
	return -1;
}

bool In_Rectangle(int x, int y, int x1, int y1, int x2, int y2) {
	if (x >= x1 && x <= x2) {
		if (y >= y1 && y <= y2) {
			return true;
		}
		return false;
	}
	return false;
}

void Update_Temp() {
	Display.txt_Width(3);
	Display.txt_Height(3);
	Display.txt_BGcolour(BLACK);
	Display.txt_MoveCursor(1, 8);
	Display.print(" ");
	Display.print(T);
	Display.print("C");
}

void Mainmenu() {
	if (start) {
		/*
		 Display.gfx_Cls();
		 Display.gfx_RectangleFilled(0,0,160,120,BLUE);
		 Display.txt_MoveCursor(3,2);
		 Display.txt_Width(2);
		 Display.txt_Height(2);
		 Display.txt_BGcolour(BLUE);
		 Display.print("Settings ");
		 */
		Display.txt_Width(3);
		Display.txt_Height(3);
		Display.txt_BGcolour(BLACK);
		Display.print("  ");
		Display.print(T);
		Display.print("C");
		if (!runn) {
			Display.gfx_RectangleFilled(0, 122, 320, 240, BLUE);
			Display.txt_BGcolour(BLUE);
			Display.txt_MoveCursor(5, 0);
			Display.txt_Width(2);
			Display.txt_Height(2);
			Display.print(" Run ");
			Display.println("Continuously");
		} else {
			Display.gfx_RectangleFilled(0, 122, 320, 240, RED);
			Display.txt_BGcolour(RED);
			Display.txt_MoveCursor(4, 4);
			Display.txt_Width(2);
			Display.txt_Height(2);
			Display.println(" Running ");
			Display.txt_MoveCursor(7, 4);
			Display.println("Press to Stop");
		}
		start = false;
	} else {
		if (Display.touch_Get(0) == 1) {
			x = Display.touch_Get(1);
			y = Display.touch_Get(2);
			// Serial.println(pushed);
			if (In_Rectangle(x, y, 0, 122, 320, 240)) {
				if (runn) {
					Serial3.println("i r1 0");
					runn = false;
				} else {
					// Serial.println("Pushed");
					// Serial.println(obtainDataRam(0x32));
					Serial3.println("i r1 1");
					delay(10);
					Serial3.println("i r0 32773");
					// Serial3.println("g r0x32");
					runn = true;
				}
				start = true;
			}
			/*
			 if (In_Rectangle(x,y, 0,0,160,120)) {
			 start = true;
			 settings = true;
			 menu = false;
			 }
			 */
		}
		if (Display.touch_Get(0) == 2) {
		}
	}
}

void setup() {
	menu = true;
	start = true;
	settings = false;
	runn = false;

	current = false;
	position = false;
	velocity = false;
	sendtemp = false;
	config = true;

	killit = false;
	overheat = false;

	temp = 20;
	counter = 0;

	//For handling errors
	T = 20;
	Display.Callback4D = mycallback;

	//5 second timeout on all commands
	Display.TimeLimit4D = 5000;
	Serial.begin(115200);
	DisplaySerial.begin(9600);

	// Leaf's updated temp pin
	pinMode(A0, INPUT);
	analogReference (INTERNAL1V1);

	//--------------------------------Optional reset routine-----------------------------------
	//Reset the Display using D4 of the Arduino (if using the new 4D Arduino Adaptor - Rev 2)
	//If using the old 4D Arduino Adaptor (Rev 1), change D4 to D2 below.
	//If using jumper wires, reverse the logic states below.
	//Refer to the accompanying application note for important information.
	pinMode(4, OUTPUT); // Set D4 on Arduino to Output (4D Arduino Adaptor V2 - Display Reset)
	digitalWrite(4, 0);  // Reset the Display via D4
	delay(100);
	digitalWrite(4, 1);  // unReset the Display via D4
	//-----------------------------------------END---------------------------------------------

	delay(5000); //let the display start up
	// Serial.print("Set");

	Display.gfx_ScreenMode(LANDSCAPE);
	Display.gfx_BGcolour(WHITE); //change background color to white
	Display.gfx_Cls();            //clear the screen

	Display.touch_Set(0);
	delay(200);

	Serial3.begin(9600);
	Req = R1 * R2;
	pinMode(22, INPUT);

	//Serial.println("Hello");
}

//what data are you collecting/sending to the db
void configure() {

	Display.gfx_Cls();
	Display.gfx_RectangleFilled(0, 0, 160, 120, GREEN);
	Display.txt_MoveCursor(3, 2);
	Display.txt_Width(2);
	Display.txt_Height(2);
	Display.txt_BGcolour(GREEN);
	Display.print("Send temp data ");

	Display.gfx_RectangleFilled(0, 122, 320, 240, RED);
	Display.txt_BGcolour(RED);
	Display.txt_MoveCursor(5, 0);
	Display.txt_Width(2);
	Display.txt_Height(2);
	Display.println("Don't send");
	while (true) {
		if (Display.touch_Get(0) == 1) {
			x = Display.touch_Get(1);
			y = Display.touch_Get(2);
			if (In_Rectangle(x, y, 0, 122, 320, 240)) {
				break;
			}
			if (In_Rectangle(x, y, 0, 0, 160, 120)) {
				sendtemp = true;
				break;
			}
		}

	}

	Display.gfx_Cls();
	Display.gfx_RectangleFilled(0, 0, 160, 120, GREEN);
	Display.txt_MoveCursor(3, 2);
	Display.txt_Width(2);
	Display.txt_Height(2);
	Display.txt_BGcolour(GREEN);
	Display.print("Send current data ");

	Display.gfx_RectangleFilled(0, 122, 320, 240, RED);
	Display.txt_BGcolour(RED);
	Display.txt_MoveCursor(5, 0);
	Display.txt_Width(2);
	Display.txt_Height(2);
	Display.println("Don't send");
	while (true) {
		if (Display.touch_Get(0) == 1) {
			x = Display.touch_Get(1);
			y = Display.touch_Get(2);
			if (In_Rectangle(x, y, 0, 122, 320, 240)) {
				break;
			}
			if (In_Rectangle(x, y, 0, 0, 160, 120)) {
				current = true;
				break;
			}
		}
	}
	Display.gfx_Cls();
	Display.gfx_RectangleFilled(0, 0, 160, 120, GREEN);
	Display.txt_MoveCursor(3, 2);
	Display.txt_Width(2);
	Display.txt_Height(2);
	Display.txt_BGcolour(GREEN);
	Display.print("Send position data ");

	Display.gfx_RectangleFilled(0, 122, 320, 240, RED);
	Display.txt_BGcolour(RED);
	Display.txt_MoveCursor(5, 0);
	Display.txt_Width(2);
	Display.txt_Height(2);
	Display.println("Don't send");
	while (true) {
		if (Display.touch_Get(0) == 1) {
			x = Display.touch_Get(1);
			y = Display.touch_Get(2);
			if (In_Rectangle(x, y, 0, 122, 320, 240)) {
				break;
			}
			if (In_Rectangle(x, y, 0, 0, 160, 120)) {
				position = true;
				break;
			}
		}
	}
	Display.gfx_Cls();
	Display.gfx_RectangleFilled(0, 0, 160, 120, GREEN);
	Display.txt_MoveCursor(3, 2);
	Display.txt_Width(2);
	Display.txt_Height(2);
	Display.txt_BGcolour(GREEN);
	Display.print("Send velocity data ");

	Display.gfx_RectangleFilled(0, 122, 320, 240, RED);
	Display.txt_BGcolour(RED);
	Display.txt_MoveCursor(5, 0);
	Display.txt_Width(2);
	Display.txt_Height(2);
	Display.println("Don't send");
	while (true) {
		if (Display.touch_Get(0) == 1) {
			x = Display.touch_Get(1);
			y = Display.touch_Get(2);
			if (In_Rectangle(x, y, 0, 122, 320, 240)) {
				break;
			}
			if (In_Rectangle(x, y, 0, 0, 160, 120)) {
				velocity = true;
				break;
			}
		}
	}
}

void emergency() {
	Display.gfx_BGcolour(RED);
	Display.gfx_Cls();
	Display.gfx_RectangleFilled(0, 0, 160, 120, BLACK);
	Display.txt_MoveCursor(3, 2);
	Display.txt_Width(2);
	Display.txt_Height(2);
	Display.txt_BGcolour(BLACK);
	Display.print("Emergency");

	if (overheat) {
		Display.txt_Width(3);
		Display.txt_Height(3);
		Display.txt_BGcolour(BLACK);
		Display.print("  ");
		Display.print(T);
		Display.print("C");

		Display.txt_BGcolour(BLACK);
		Display.txt_MoveCursor(5, 0);
		Display.txt_Width(2);
		Display.txt_Height(2);
		Display.print(" Overheat ");

		Serial3.println("i r1 0");

	} else {
		overheat = false;
		killit = false;
		menu = true;
		start = true;
	}

}

void senddata() {
	if (current) {
		sendInt("17582d45-6afa-11e7-971b-6c0b843e9461", obtainDataRam(0x0c)); //current
	}
	if (position) {
		sendInt("256cb785-6afa-11e7-971b-6c0b843e9461", obtainDataRam(0x32)); // position
	}
	if (velocity) {
		sendInt("294ad913-6afa-11e7-971b-6c0b843e9461", obtainDataRam(0x18)); // velocity
	}
	if (sendtemp) {
		sendInt("a0fb2bc7-a85c-11e7-971b-6c0b843e9461", KTYadj); // temp
	}
}

void loop() {
	//Serial.println("WHY");
	if (config) {
		configure();
	}
	if (killit) {
		emergency();
	} else if (menu) {
		Mainmenu();
	}
	/*if(digitalRead(22) == LOW)
	 {
	 //Emergency
	 Serial.println("Emergency");
	 }*/
	delay(10);
	counter++;

	// while (Serial3.available()) {
	//     Serial.write(Serial3.read());
	// }
	// Serial.println(counter);
	if (counter % 5 == 0) {

		float Vout = analogRead(A0);
		float Rt = ((Vout / 1024) * 1.1 * 10000) / (12 - (Vout / 1024) * 1.1);
		float KTY = (2200 * Rt) / (2200 - Rt);
		KTYadj = 0.1 * KTY + 0.9 * KTYadj;
		temps[itr] = getTemperature(KTYadj);
		//Serial.println(KTYadj);

		itr++;
		if (counter >= 200) {
			T = average();
			if (menu) {
				Update_Temp();
				//Serial.print("It is done");
			}
			counter = 1;
			itr = 0;
		}
	}

	// Serial.println("Howdy");

	// Serial.println(obtainDataRam(0x32));
	// (dbl) [ TestInstrument / TestField ]
	senddata();
	// Serial.println(obtainDataRam(0x32));
}

void mycallback(int ErrCode, unsigned char Errorbyte) {
	// Pin 13 has an LED connected on most Arduino boards. Just give it a name
	int led = 13;
	pinMode(led, OUTPUT);
	while (1) {
		digitalWrite(led, HIGH);  // turn the LED on (HIGH is the voltage level)
		delay(200);                // wait for 200 ms
		digitalWrite(led, LOW);    // turn the LED off by making the voltage LOW
		delay(200);                // wait for 200 ms
	}
}

// Sends data to the computer
void sendInt(const String& guid, int data) {
	Serial.print(guid);
	Serial.print(",");
	Serial.println(data);
}

// Gets data from the RAM bank.
long obtainDataRam(int paramId) {
	Serial3.print("g r");
	Serial3.println(paramId);

	// Probably long enough for a whole message to get buffered
	// 10 was small, 20 worked, added safety x2
	delay(40);

	killChar();  // v
	killChar();  // space

	bool negative = false;
	long result = 0;
	while (Serial3.available()) {
		char val = Serial3.read();
		// Serial.print("GET CHAR: ");
		// Serial.println(val);
		if (val == '-') {
			negative = true;
		} else if (val >= '0' && val <= '9') {
			result *= 10;
			result += (val - '0');
		} else {
			// Serial.print("\nERROR: ");
			// Serial.println(val);
		}
	}
	return result * (negative ? -1 : 1);
	// return -1;
}

// Consumes a single character from Serial3
void killChar() {
	while (!Serial3.available()) {
	}
	char val = Serial3.read();
	// Serial.print("KILL CHAR: ");
	// Serial.println(val);
}
