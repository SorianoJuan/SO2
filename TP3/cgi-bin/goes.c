#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main(void)
{
	char * input = strchr(getenv("QUERY_STRING"),'=')+1;
	char input_aux [200];
	strcpy(input_aux, input);
	char day[100],*dayPointer;
	char year[100],*yearPointer;
	char * pch;
	printf("string: %s\n",input);
	//pch = strchr(input_aux, '&');
	//strncpy(year, 0, pch);
	yearPointer = strtok(input_aux,"&");
	strtok(NULL, "=");
	dayPointer = strtok(NULL, " ");
	printf("year: %s",yearPointer);
	printf("day: %s",dayPointer); 
	
	char command[300];
	strcpy(command, "/home/torce/.local/bin/aws s3 --no-sign-request ls --human-readable --recursive noaa-goes16/ABI-L2-CMIPF/");
	strcat(command,yearPointer);
	strcat(command,"/");
	strcat(command,dayPointer);
	strcat(command,"/");
	strcat(command," | grep M3C13_G16");
	//printf("command: %s", command);
	//system(command);
	
	//printf("Content-Type: text/plain\n\n");
	FILE * process;
	char * line = NULL;
	char buffer[500];
	size_t len = 0;
	ssize_t read;
	process = popen(command,"r");
	while((read = getline(&line, &len, process)) != -1){	
		memset(buffer, '\0', 500);
		line[read-1] = '\0';
		strcat(buffer, "<tr>\n");
		char * text = strtok(line," ");
		strcat(buffer, "\t<td>");
		strcat(buffer, text);
		strcat(buffer, "</td>\n");
																										
		text = strtok(NULL, " ");
		strcat(buffer, "\t<td>");				
		strcat(buffer, text);
		strcat(buffer, "</td>\n");
																																	
		text = strtok(NULL, " ");
		strcat(buffer, "\t<td>");
		strcat(buffer, text);
		strcat(buffer, " ");
		text = strtok(NULL, " ");
		strcat(buffer, text);
		strcat(buffer, "</td>\n");
		text = strtok(NULL, " ");
		text = strtok(text, "/");
		text = strtok(NULL, "/");
		text = strtok(NULL, "/");
		text = strtok(NULL, "/");
		text = strtok(NULL, "/");
		strcat(buffer, "\t<td>");
		strcat(buffer, text);
		strcat(buffer, "</td>\n");

		strcat(buffer, "</tr>\n");
		printf("%s", buffer);
	}
	return 0;
}
