#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>

#define DATA_PIN "7"
#define CLOCK_PIN "11"
#define LATCH_PIN "12"

#define GPIO_DEVICE_PATH "/sys/devices/virtual/gpio"
#define GPIO_CLASS_PATH "/sys/class/gpio/"

int export_pins_set_dirs();
int unexport_pins();
int send_bits(int *bits);

int main(int argc, char *argv[]){
	//if(export_pins_set_dirs()) {fprintf(stderr, "Failed to export pins.\nQuitting.\n"); return -1;}

	unsigned int motor_speed;
	int *speed_bits = malloc(sizeof(int)*8);
	int i = 1;
	while(i){
		printf("Please enter a speed (0-255):");
		scanf("%u", &motor_speed);
		if(0<=motor_speed && motor_speed <=255) i=0;
		else printf("Speed outside of range!\n");
	}
	printf("speed_bits = {");
	for(i=0; i<8; i++){
		if(motor_speed % 2) speed_bits[i] = 1;
		else speed_bits[i] = 0;
		motor_speed /= 2;
		printf("%d, ",speed_bits[i]);
	}
	printf("}\n");

	if(send_bits(speed_bits)) fprintf(stderr, "Failed to send the bits correctly!\n");
	else printf("Seem to have correctly sent the bits.\n");
	
	unexport_pins();
	return 0;
}

#define DATA_DIR (GPIO_DEVICE_PATH DATA_PIN "/direction")
#define DATA_VAL (GPIO_DEVICE_PATH DATA_PIN "/value")
#define CLOCK_DIR (GPIO_DEVICE_PATH CLOCK_PIN "/direction")
#define CLOCK_VAL (GPIO_DEVICE_PATH CLOCK_PIN "/value")
#define LATCH_DIR (GPIO_DEVICE_PATH LATCH_PIN "/direction")
#define LATCH_VAL (GPIO_DEVICE_PATH LATCH_PIN "/value")
#define EXPORT_PATH (GPIO_CLASS_PATH "export")
#define UNEXPORT_PATH (GPIO_CLASS_PATH "unexport")


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
	// Use gpio-admin to unexport pins
	char unexport_cmd[PATH_MAX];
	snprintf(unexport_cmd, PATH_MAX, "%s %s", UNEXPORT_PATH, DATA_PIN);
	snprintf(unexport_cmd, PATH_MAX, "%s %s", UNEXPORT_PATH, CLOCK_PIN);
	snprintf(unexport_cmd, PATH_MAX, "%s %s", UNEXPORT_PATH, LATCH_PIN);

	return 0;
}

int send_bits(int *bits){
	int i;
	FILE *data_file, *clock_file, *latch_file;
	data_file = fopen(DATA_VAL, "w");
	if(data_file==NULL) {fprintf(stderr, "Unable to write to data pin!\n"); return -1;}
	clock_file = fopen(CLOCK_VAL, "w");
	if(clock_file==NULL) {fprintf(stderr, "Unable to write to clock pin!\n"); fclose(data_file); return -1;}
	latch_file = fopen(LATCH_VAL, "w");
	if(latch_file==NULL) {fprintf(stderr, "Unable to write to latch pin!\n"); fclose(data_file); fclose(clock_file); return -1;}
	for(i=0; i<8; i++){
		fprintf(clock_file, "0");
		if(bits[i]) fprintf(data_file, "1");
		else fprintf(data_file, "0");
		usleep(1000);
		fprintf(clock_file, "1");
		usleep(1000);
	}
	// Latch the bits
	fprintf(latch_file, "1");
	usleep(1000);
	fprintf(latch_file, "0");

	fclose(data_file); fclose(clock_file); fclose(latch_file);
	return 0;
}
