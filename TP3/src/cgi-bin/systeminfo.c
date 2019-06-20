#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sysinfo.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

int main(void){
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);
	FILE *fp;
	struct sysinfo s_info;
    	int error = sysinfo(&s_info);
        if (error != 0) {
		printf("Error en system = %d\n", error);
	}
	long double a[4], b[4], loadavg;
	fp = fopen ("/proc/stat","r");
	fscanf(fp,"%*s %Lf %Lf %Lf %Lf",&a[0],&a[1],&a[2],&a[3]);
	fclose(fp);
        sleep(1);

	fp = fopen("/proc/stat","r");
	fscanf(fp,"%*s %Lf %Lf %Lf %Lf",&b[0],&b[1],&b[2],&b[3]);
	fclose(fp);

	loadavg = ((b[0]+b[1]+b[2]) - (a[0]+a[1]+a[2])) / ((b[0]+b[1]+b[2]+b[3]) - (a[0]+a[1]+a[2]+a[3]));
	printf("Consumo de CPU : %Lf\n",loadavg);
	printf("Uptime = %lu seg\n",  s_info.uptime);
	printf("Memoria = %lu bytes\n", s_info.totalram - s_info.freeram);
	printf("Fecha y Hora actual : %d-%d-%d %d:%d:%d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
}
