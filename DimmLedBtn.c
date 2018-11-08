/* 
 *  Rainbow LED Button ( GPIO ver )
 *  Author       : sana2dang ( fly9p ) - sana2dang@naver.com / sana2dang@gmail.com
 *  Creation Date: 2017 - 04 - 01
 *  Cafe         : http://cafe.naver.com/raspigamer
 *  Thanks to    : zzeromin, smyani, GreatKStar, KimPanda, StarNDevil, angel
 * 
 * - Reference -
 * wiringPi
 * http://wiringpi.com/the-gpio-utility/
 * apt-get install wiringPi
 * 
 * jstest
 * https://github.com/flosse/linuxconsole/blob/master/utils/jstest.c
  *
 * - complie -
 * sudo gcc DimmLedBtn.c -o DimmLedBtn -lwiringPi -lpthread
 * ./DimmLedBtn /dev/input/js0 4
 */
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <wiringPi.h>
#include <softPwm.h>
#include <pthread.h>

#include <linux/input.h>
#include <linux/joystick.h>

#define NAME_LENGTH 128

#define pinA  21	// GPIO 21 A
#define pinB  20	// GPIO 20 B
#define pinX  7		// GPIO 7 X
#define pinY  12	// GPIO 12 Y
#define pinL  8		// GPIO 8  L
#define pinR  16	// GPIO 16 R
#define pinSE 26 	// GPIO 26 SELECT
#define pinST 19 	// GPIO 19 START

#define rotationDelay 400
#define pwmRange 100

#define AbtnShotPressTime 700	
#define AbtnLongPressTime 1500
#define stayTime 10000

int RGBbtnPress	= 0;	// RGBbtnPress on/off flag
int chargeShot	= 0;	// chargeShot cnt
long sTime	= 0;	// RGBbtnPress Start Time
int chargeCnt 	= 0;	// Charge Count
int k = 0;		// Led Cycle Count


long sStopTime = 0;	// stop time
int rotationFlag = 0;	// rotation
int rotationCnt = 0;
int mode = 0;


static int dimmPinA = -1;
static int dimmPinB = -1;
static int dimmPinX = -1;
static int dimmPinY = -1;
static int dimmPinL = -1;
static int dimmPinR = -1;
static int dimmPinSE = -1;
static int dimmPinST = -1;

static int dimmFlagA = 0;
static int dimmFlagB = 0;
static int dimmFlagX = 0;
static int dimmFlagY = 0;
static int dimmFlagL = 0;
static int dimmFlagR = 0;
static int dimmFlagSE = 0;
static int dimmFlagST = 0;

static int dimmDelay = 3;

void funAllLightOn(int mode)
{
	if( mode == 0 )		// ALL LED ON
	{
		softPwmWrite(pinA,pwmRange);		
		softPwmWrite(pinB,pwmRange);
		softPwmWrite(pinX,pwmRange);
		softPwmWrite(pinY,pwmRange);
		softPwmWrite(pinL,pwmRange);
		softPwmWrite(pinR,pwmRange);
		softPwmWrite(pinSE,pwmRange);
		softPwmWrite(pinST,pwmRange);
	}
	if( mode == 1 )		// ALL LED ON without A btn
	{
		softPwmWrite(pinB,pwmRange);
		softPwmWrite(pinX,pwmRange);
		softPwmWrite(pinY,pwmRange);
		softPwmWrite(pinL,pwmRange);
		softPwmWrite(pinR,pwmRange);
		softPwmWrite(pinSE,pwmRange);
		softPwmWrite(pinST,pwmRange);
	}
}

void funAllLightOff(int mode)
{
	if( mode == 0 )		// ALL LED OFF
	{
		softPwmWrite(pinA,0);		
		softPwmWrite(pinB,0);
		softPwmWrite(pinX,0);
		softPwmWrite(pinY,0);
		softPwmWrite(pinL,0);
		softPwmWrite(pinR,0);
		softPwmWrite(pinSE,0);
		softPwmWrite(pinST,0);
	}
	if( mode == 1 )		// ALL LED OFF without A btn
	{
		softPwmWrite(pinB,0);
		softPwmWrite(pinX,0);
		softPwmWrite(pinY,0);
		softPwmWrite(pinL,0);
		softPwmWrite(pinR,0);
		softPwmWrite(pinSE,0);
		softPwmWrite(pinST,0);
	}

}


void funRainbowLed()
{	
	if( rotationFlag == 1)	// stay button led
	{
		switch( rotationCnt )
		{
			case 0:
				//softPwmWrite(pinA,pwmRange);				
				dimmPinA = pinA;
				dimmFlagA = 1;
				dimmFlagX = 0;
				rotationCnt ++;
			break;
			case 1: 
				//softPwmWrite(pinB,pwmRange);
				dimmPinB = pinB;
				dimmFlagB = 1;
				dimmFlagA = 0;
				rotationCnt ++;
			break;
			case 2: 
				//softPwmWrite(pinR,pwmRange);
				dimmPinR = pinR;
				dimmFlagR = 1;
				dimmFlagB = 0;
				rotationCnt ++;
			break;
			case 3: 
				//softPwmWrite(pinL,pwmRange);
				dimmPinL = pinL;
				dimmFlagL = 1;
				dimmFlagR = 0;
				rotationCnt ++;
			break;
			case 4: 
				//softPwmWrite(pinY,pwmRange);
				dimmPinY = pinY;
				dimmFlagY = 1;
				dimmFlagL = 0;
				rotationCnt ++;
			break;
			case 5: 
				//softPwmWrite(pinX,pwmRange);
				dimmPinX = pinX;
				dimmFlagX = 1;
				dimmFlagY = 0;
				rotationCnt = 0;
			break;	
			default: 
				funAllLightOff(0);	
		}
		delay(rotationDelay);		// 0.5sec 
		funAllLightOff(0);		
	
	}
	else
	{
		rotationCnt = 0;	
	}

	// A Btn Event
	if( RGBbtnPress == 1 && chargeShot == 0 )
	{
		//printf("%d\n", chargeCnt );
		switch( chargeCnt )
		{
			case 0: 
				softPwmWrite(pinA, pwmRange);			
			break;
			case 1:
				softPwmWrite(pinA, pwmRange);					
			break;
			case 2: 
				softPwmWrite(pinA, pwmRange);					
			break;			
			case 3: 
				softPwmWrite(pinA, pwmRange);							
			break;
			case 4: 
				softPwmWrite(pinA, pwmRange);					
			break;
			case 5: 				
				softPwmWrite(pinA, pwmRange);						
			break;
			case 6: 
				softPwmWrite(pinA, pwmRange);						
			break;
			case 7: 
				softPwmWrite(pinA, pwmRange);						
			break;
			default: 
				softPwmWrite(pinA,0);				
				chargeCnt = 0;
			break;
		}
		delay(50);		

		if( ( millis() - sTime ) < AbtnShotPressTime )		// RGBbtnPress keep check1
		{					
			chargeCnt = 0;
		}
		else
		{
			
			if( ( millis() - sTime )  > AbtnLongPressTime )	// RGBbtnPress keep check2
			{				
				chargeShot = 1;				
				//softPwmWrite(pinA, 0);			
				dimmFlagA = 0;
				dimmFlagB = 0;
				dimmFlagX = 0;
				dimmFlagY = 0;
				dimmFlagL = 0;
				dimmFlagR = 0;
				dimmFlagSE = 0;
				dimmFlagST = 0;
				
				funAllLightOff(0);
			}
			else
			{
				chargeCnt++;
			}
		}
	}
	if( RGBbtnPress == 0 )
	{	
		if( chargeShot == 1 )
		{	
			funAllLightOff(0);
			for(k=0; k<2; k++)
			{
				
				funAllLightOn(0);
				delay(40);

				
				funAllLightOff(0);
				delay(60);
	
				
				funAllLightOn(0);
				delay(40);

				
				funAllLightOff(0);
				delay(60);

				/*
				funAllLightOn(0);
				delay(40);

				
				funAllLightOff(0);
				delay(60);
				*/
				
				funAllLightOn(0);
				delay(40);
				funAllLightOff(0);
			}
			funAllLightOn(0);
			delay(400);
			funAllLightOff(0);
		}		
		
		chargeCnt	= 0;		// init value
		chargeShot	= 0;		// init value
	}

}


// LED Btn Thread
void* ledBtnThread(void *arg)
{
	int chargeCnt = 0;	
	int k = 0;
	
	while(1)
	{			
		funRainbowLed();
		usleep(100000);
		
		if( ( millis() - sStopTime ) > stayTime )	// rotation check
		{
			rotationFlag = 1;
		}		
	}	
}







// DIMM Btn Thread
void* dimmLedA(void *arg)
{
	int bright = pwmRange;
	
	while(1)
	{
		usleep(100000);
		if( dimmPinA != -1 )
		{
			for( bright = pwmRange; bright >=0; --bright )
			{
				if( dimmFlagA == 1)
				{
					bright = pwmRange;
				}
				if( chargeShot==1 )
				{
					softPwmWrite(dimmPinA, 0);
					dimmPinA = -1;
					dimmFlagA = 0;
					break;
				}
				
				softPwmWrite(dimmPinA, bright);
				delay(dimmDelay);				
				
				if( bright == 0 )
				{
					dimmPinA = -1;
					dimmFlagA = 0;
				}
			}				
		}		
	}
}







// DIMM Btn Thread
void* dimmLedB(void *arg)
{
	int bright = pwmRange;
	
	while(1)
	{
		usleep(100000);
		if( dimmPinB != -1 )
		{
			for( bright = pwmRange; bright >=0; --bright )
			{
				if( dimmFlagB == 1)
				{
					bright = pwmRange;
				}
				softPwmWrite(dimmPinB, bright);
				delay(dimmDelay);				
				
				if( bright == 0 )
				{
					dimmPinB = -1;
					dimmFlagB = 0;
				}
			}				
		}		
	}
}








void* dimmLedX(void *arg)
{
	int bright = pwmRange;
	
	while(1)
	{
		usleep(100000);
		if( dimmPinX != -1 )
		{
			for( bright = pwmRange; bright >=0; --bright )
			{
				if( dimmFlagX == 1)
				{
					bright = pwmRange;
				}
				softPwmWrite(dimmPinX, bright);
				delay(dimmDelay);				
				
				if( bright == 0 )
				{
					dimmPinX = -1;
					dimmFlagX = 0;
				}
			}				
		}		
	}
}






void* dimmLedY(void *arg)
{
	int bright = pwmRange;
	
	while(1)
	{
		usleep(100000);
		if( dimmPinY != -1 )
		{
			for( bright = pwmRange; bright >=0; --bright )
			{
				if( dimmFlagY == 1)
				{
					bright = pwmRange;
				}
				softPwmWrite(dimmPinY, bright);
				delay(dimmDelay);				
				
				if( bright == 0 )
				{
					dimmPinY = -1;
					dimmFlagY = 0;
				}
			}				
		}		
	}
}


void* dimmLedL(void *arg)
{
	int bright = pwmRange;
	
	while(1)
	{
		usleep(100000);
		if( dimmPinL != -1 )
		{
			for( bright = pwmRange; bright >=0; --bright )
			{
				if( dimmFlagL == 1)
				{
					bright = pwmRange;
				}
				softPwmWrite(dimmPinL, bright);
				delay(dimmDelay);				
				
				if( bright == 0 )
				{
					dimmPinL = -1;
					dimmFlagL = 0;
				}
			}				
		}		
	}
}




void* dimmLedR(void *arg)
{
	int bright = pwmRange;
	
	while(1)
	{
		usleep(100000);
		if( dimmPinR != -1 )
		{
			for( bright = pwmRange; bright >=0; --bright )
			{
				if( dimmFlagR == 1)
				{
					bright = pwmRange;
				}
				softPwmWrite(dimmPinR, bright);
				delay(dimmDelay);				
				
				if( bright == 0 )
				{
					dimmPinR = -1;
					dimmFlagR = 0;
				}
			}				
		}		
	}
}




void* dimmLedSE(void *arg)
{
	int bright = pwmRange;
	/*
	while(1)
	{
		
		if( dimmPinSE != -1 )
		{
			for( bright = pwmRange; bright >=0; --bright )
			{
				if( dimmFlagSE == 1)
				{
					bright = pwmRange;
				}
				softPwmWrite(dimmPinSE, bright);
				delay(dimmDelay);				
				
				if( bright == 0 )
				{
					dimmPinSE = -1;
					dimmFlagSE = 0;
				}
			}				
		}				
	}
	*/
}



void* dimmLedST(void *arg)
{
	int bright = pwmRange;
	
	while(1)
	{
		usleep(100000);
		if( dimmPinST != -1 )
		{
			for( bright = pwmRange; bright >=0; --bright )
			{
				softPwmWrite(dimmPinST, bright);
				delay(dimmDelay);				
				if( dimmFlagST == 1)
				{
					bright = pwmRange;
				}
				if( bright == 0 )
				{
					dimmPinST = -1;
					dimmFlagST = 0;
				}
			}				
		}		
	}
}


int main (int argc, char **argv)
{
	int fd, i;
	unsigned char axes = 2;
	unsigned char buttons = 2;
	int version = 0x000800;
	char name[NAME_LENGTH] = "Unknown";
	
	// thread value
	int res;
	int resA;
	int resB;
	int resX;
	int resY;
	int resL;
	int resR;
	int resSE;
	int resST;
	
	pthread_t a_thread;
	pthread_t dimm_threadA;
	pthread_t dimm_threadB;
	pthread_t dimm_threadX;
	pthread_t dimm_threadY;
	pthread_t dimm_threadL;
	pthread_t dimm_threadR;
	pthread_t dimm_threadSE;
	pthread_t dimm_threadST;
	
	
	void* thread_result;
	void* thread_resultB;
	void* thread_resultX;
	void* thread_resultY;
	void* thread_resultL;
	void* thread_resultR;
	void* thread_resultSE;
	void* thread_resultST;
	
	
	dimmDelay = atoi(argv[2]);
	
	if (argc != 3) 
	{
		puts("");
		puts("Usage: RainbowLedBtn <#joystick>");
		puts("");
		puts("pin number( GPIO ) : LED(6 12 13 16 19) RGB(20 21 26)");		
		puts("");
		puts("ex) ./RainbowLedBtn /dev/input/js0");
		puts("");
		exit(1);
	}
	
	if ((fd = open(argv[argc - 2], O_RDONLY)) < 0) {
		perror("jstest");
		return 1;
	}

	ioctl(fd, JSIOCGVERSION, &version);
	ioctl(fd, JSIOCGAXES, &axes);
	ioctl(fd, JSIOCGBUTTONS, &buttons);
	ioctl(fd, JSIOCGNAME(NAME_LENGTH), name);

	//printf("Driver version is %d.%d.%d.\n",		version >> 16, (version >> 8) & 0xff, version & 0xff);

	//printf("Testing ... (interrupt to exit)\n");


/*
 * Event interface, single line readout.
 */

	if (argc == 3) {

		int *axis;
		char *button;
		int i;
		struct js_event js;

		if( wiringPiSetupGpio() == -1 )
			return 0;
		
		
		// 차지 LED 쓰레드
		res = pthread_create(&a_thread, NULL, ledBtnThread, (void*)NULL);
		resA = pthread_create(&dimm_threadA, NULL, dimmLedA, (void*)NULL);
		resB = pthread_create(&dimm_threadB, NULL, dimmLedB, (void*)NULL);
		resX = pthread_create(&dimm_threadX, NULL, dimmLedX, (void*)NULL);
		resY = pthread_create(&dimm_threadY, NULL, dimmLedY, (void*)NULL);
		resL = pthread_create(&dimm_threadL, NULL, dimmLedL, (void*)NULL);
		resR = pthread_create(&dimm_threadR, NULL, dimmLedR, (void*)NULL);
		//resSE = pthread_create(&dimm_threadSE, NULL, dimmLedSE, (void*)NULL);
		resST = pthread_create(&dimm_threadST, NULL, dimmLedST, (void*)NULL);
				
		if( res )
			printf("thread create error!\n");
		if( resA )
			printf("thread A create error!\n");
		if( resB )
			printf("thread B create error!\n");
		if( resX )
			printf("thread X create error!\n");
		if( resY )
			printf("thread Y create error!\n");
		if( resL )
			printf("thread L create error!\n");
		if( resR )
			printf("thread R create error!\n");
		//if( resSE )
			//printf("thread SE create error!\n");
		if( resST )
			printf("thread ST create error!\n");
	
		
		
		pinMode(pinA, OUTPUT);			
		pinMode(pinB, OUTPUT);	
		pinMode(pinX, OUTPUT);	
		pinMode(pinY, OUTPUT);	
		pinMode(pinL, OUTPUT);	
		pinMode(pinR, OUTPUT);	
		pinMode(pinSE, OUTPUT);	
		pinMode(pinST, OUTPUT);	
		
		softPwmCreate(pinA, 0, pwmRange);
		softPwmCreate(pinB, 0, pwmRange);
		softPwmCreate(pinX, 0, pwmRange);
		softPwmCreate(pinY, 0, pwmRange);
		softPwmCreate(pinL, 0, pwmRange);
		softPwmCreate(pinR, 0, pwmRange);
		softPwmCreate(pinSE, 0, pwmRange);
		softPwmCreate(pinST, 0, pwmRange);
		
		/*
		pinMode(pinA, PWM_OUTPUT);			
		pinMode(pinB, PWM_OUTPUT);	
		pinMode(pinX, PWM_OUTPUT);	
		pinMode(pinY, PWM_OUTPUT);	
		pinMode(pinL, PWM_OUTPUT);	
		pinMode(pinR, PWM_OUTPUT);
		*/
		/*
		pinMode(pinA, OUTPUT);			
		pinMode(pinB, OUTPUT);	
		pinMode(pinX, OUTPUT);	
		pinMode(pinY, OUTPUT);	
		pinMode(pinL, OUTPUT);	
		pinMode(pinR, OUTPUT);	
		*/
		axis = calloc(axes, sizeof(int));
		button = calloc(buttons, sizeof(char));

		while (1) {
			if (read(fd, &js, sizeof(struct js_event)) != sizeof(struct js_event)) {
				perror("\njstest: error reading");
				return 1;
			}

			switch(js.type & ~JS_EVENT_INIT) {
			case JS_EVENT_BUTTON:
				button[js.number] = js.value;
				break;
			case JS_EVENT_AXIS:
				axis[js.number] = js.value;
				break;
			}

			//printf("\r");

			/*
			if (axes) {
				printf("Axes: ");
				for (i = 0; i < axes; i++)
					printf("%2d:%6d ", i, axis[i]);
			}
			*/

			if (buttons) { 
				for (i = 0; i < buttons; i++)
				{
					//printf("%2d:%s ", i, button[i] ? "on " : "off");
					
					sStopTime = millis();		// rotation time Start
					rotationFlag = 0;		// stop rotation

					if( i==0 && button[i] == 1 )
					{
						
						
						if( RGBbtnPress  != 1 )
						{
							dimmPinA = pinA;
							dimmFlagA = 1;
						
							RGBbtnPress  = 1;							
							sTime = millis();
						}
						
					}
					if( i==0 && button[i] == 0 )
					{						
						RGBbtnPress  = 0;							
						sTime = millis();
						
						dimmFlagA = 0;
						//digitalWrite(pinA,0);		// RGB Led Off
						
					}

					
					
					// B 버튼
					if( i==1 && button[i] == 1 )
					{
						dimmPinB = pinB;
						dimmFlagB = 1;
					}
					if( i==1 && button[i] == 0 )
						dimmFlagB = 0;

					
					
					
					
					
					
					// X 버튼
					if( i==3 && button[i] == 1 )
					{
						dimmPinX = pinX;
						dimmFlagX = 1;
					}
					if( i==3 && button[i] == 0 )
						dimmFlagX = 0;

					
					
					
					// Y 버튼
					if( i==4 && button[i] == 1 )
					{
						dimmPinY = pinY;
						dimmFlagY = 1;					
					}
					if( i==4 && button[i] == 0 )
						dimmFlagY = 0;
					
					
					
					
					// L 버튼
					if( i==6 && button[i] == 1 )
					{
						dimmPinL = pinL;
						dimmFlagL = 1;
					}
					if( i==6 && button[i] == 0 )
						dimmFlagL = 0;

					
					
					
					// R 버튼
					if( i==7 && button[i] == 1 )
					{
						dimmPinR = pinR;
						dimmFlagR = 1;
					}
					if( i==7 && button[i] == 0 )
						dimmFlagR = 0;
					

					// Select 버튼( Coin )
					if( i==10 && button[i] == 1 )
					{
						
						funAllLightOn(0);
						delay(60);						
						funAllLightOff(0);
						delay(60);						
						funAllLightOn(0);
						delay(60);						
						funAllLightOff(0);						
					}			


					// Start 버튼
					if( i==11 && button[i] == 1 )
					{
						dimmPinST = pinST;
						dimmFlagST = 1;
					}
					if( i==11 && button[i] == 0 )
						dimmFlagST = 0;
					
				}
			}

			fflush(stdout);
		}
	}

	printf("jstest: unknown mode: %s\n", argv[1]);
	return -1;
}
