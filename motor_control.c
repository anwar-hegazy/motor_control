#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>

#define DATA_PIN "24"
#define CLOCK_PIN "18"
#define LATCH_PIN "21"

#define GPIO_DEVICE_PATH "/sys/devices/virtual/gpio/gpio"
#define GPIO_CLASS_PATH "/sys/class/gpio/"
#define WRITE_PAUSE 1000

int export_pins_set_dirs();
int unexport_pins();
int send_bits(int *bits, int num_bits);

int main(int argc, char *argv[]){
	if(export_pins_set_dirs()) {fprintf(stderr, "Failed to export pins.\nQuitting.\n"); return -1;}

	int left_speed, right_speed;
	int *speed_bits = malloc(sizeof(int)*18);
	int i = 1;
	while(i){
		i=0;

		printf("Please enter left motor speed [-255,255]:");
		scanf("%d", &left_speed);
		if(!(-255<=left_speed && left_speed <=255)){
			printf("Speed outside of range!\n"); i=1;}

		printf("Please enter right motor speed [-255,255]:");
		scanf("%d", &right_speed);
		if(!(-255<=right_speed && right_speed <=255)){
			printf("Speed outside of range!\n"); i=1;}
	}
	if(left_speed <0) speed_bits[0] = 1; else speed_bits[0] = 0;
	if(right_speed<0) speed_bits[1] = 1; else speed_bits[1] = 0;
	for(i=0; i<8; i++){
		speed_bits[2+i] = (abs(left_speed) >> (7-i)) % 2;
		//if(motor_speed % 2) speed_bits[i] = 1;
		//else speed_bits[i] = 0;
		//motor_speed /= 2;
		//printf("%d, ",speed_bits[i]);
	}
	for(i=0; i<8; i++)
		speed_bits[10+i] = (abs(right_speed) >> (7-i)) % 2;
	printf("speed_bits = {");
	for(i=0; i<18; i++)
		printf("%d, ", speed_bits[i]);
	printf("}\n");

	if(send_bits(speed_bits, 18)) fprintf(stderr, "Failed to send the bits correctly!\n");
	else printf("Seem to have correctly sent the bits.\n");
	
	if(unexport_pins()) fprintf(stderr, "A possible error occurred unexporting the pins.\n");
	return 0;
}

#define DATA_DIR (GPIO_DEVICE_PATH DATA_PIN "/direction")
#define DATA_VAL (GPIO_DEVICE_PATH DATA_PIN "/value")
#define CLOCK_DIR (GPIO_DEVICE_PATH CLOCK_PIN "/direction")
#define CLOCK_VAL (GPIO_DEVICE_PATH CLOCK_PIN "/value")
#define LATCH_DIR (GPIO_DEVICE_PATH LATCH_PIN "/direction")
#define LATCH_VAL (GPIO_DEVICE_PATH LATCH_PIN "/value")
//#define EXPORT_PATH (GPIO_CLASS_PATH "export")
#define EXPORT_PATH ("gpio-admin " "export")
//#define UNEXPORT_PATH (GPIO_CLASS_PATH "unexport")
#define UNEXPORT_PATH ("gpio-admin " "unexport")


int export_pins_set_dirs(){
	// Use gpio-admin to export pins
	char export_cmd[PATH_MAX];
	snprintf(export_cmd, PATH_MAX, "%s %s", EXPORT_PATH, DATA_PIN);
	if (system(export_cmd)) return -1;
	snprintf(export_cmd, PATH_MAX, "%s %s", EXPORT_PATH, CLOCK_PIN);
	if (system(export_cmd)) return -1;
	snprintf(export_cmd, PATH_MAX, "%s %s", EXPORT_PATH, LATCH_PIN);
	if (system(export_cmd)) return -1;

	// Set the directions to out
	FILE *gpiofile;
	gpiofile = fopen(DATA_DIR, "w");
	if (gpiofile==NULL) return -1;
	fprintf(gpiofile, "out");
	fclose(gpiofile);
	gpiofile = fopen(CLOCK_DIR, "w");
	if (gpiofile==NULL) return -1;
	fprintf(gpiofile, "out");
	fclose(gpiofile);
	gpiofile = fopen(LATCH_DIR, "w");
	if (gpiofile==NULL) return -1;
	fprintf(gpiofile, "out");
	fclose(gpiofile);

	return 0;
}

int unexport_pins(){
	int ret_val = 0;
	// Use gpio-admin to unexport pins
	char unexport_cmd[PATH_MAX];
	snprintf(unexport_cmd, PATH_MAX, "%s %s", UNEXPORT_PATH, DATA_PIN);
	if (system(unexport_cmd)) ret_val = -1;
	snprintf(unexport_cmd, PATH_MAX, "%s %s", UNEXPORT_PATH, CLOCK_PIN);
	if (system(unexport_cmd)) ret_val = -1;
	snprintf(unexport_cmd, PATH_MAX, "%s %s", UNEXPORT_PATH, LATCH_PIN);
	if (system(unexport_cmd)) ret_val = -1;

	return ret_val;
}

int send_bits(int *bits, int num_bits){
	int i;
	FILE *data_file, *clock_file, *latch_file;
	//data_file = fopen(DATA_VAL, "w");
	//if(data_file==NULL) {fprintf(stderr, "Unable to write to data pin!\n"); return -1;}
	//clock_file = fopen(CLOCK_VAL, "w");
	//if(clock_file==NULL) {fprintf(stderr, "Unable to write to clock pin!\n"); fclose(data_file); return -1;}
	//latch_file = fopen(LATCH_VAL, "w");
	//if(latch_file==NULL) {fprintf(stderr, "Unable to write to latch pin!\n"); fclose(data_file); fclose(clock_file); return -1;}
	for(i=0; i<num_bits; i++){
		// Move the clock pin low
		clock_file = fopen(CLOCK_VAL, "w");
		fprintf(clock_file, "0");
		fclose(clock_file);

		// Write the data bit
		data_file = fopen(DATA_VAL, "w");
		if(bits[i]) fprintf(data_file, "1");
		else fprintf(data_file, "0");
		fclose(data_file);
		usleep(WRITE_PAUSE);

		// Take the clock pin high
		clock_file = fopen(CLOCK_VAL, "w");
		fprintf(clock_file, "1");
		fclose(clock_file);
		usleep(WRITE_PAUSE);
		fprintf(stderr, "%d\n", bits[i]);
	}
	// Latch the bits
	latch_file = fopen(LATCH_VAL, "w");
	fprintf(latch_file, "0");
	fclose(latch_file);
	usleep(WRITE_PAUSE);
	latch_file = fopen(LATCH_VAL, "w");
	fprintf(latch_file, "1");
	fclose(latch_file);

	//fclose(data_file); fclose(clock_file); fclose(latch_file);
	return 0;
}
