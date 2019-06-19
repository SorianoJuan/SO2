#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main(void)
{
	printf("Content-Type: text/html\n\n");
	char * line = NULL;
	char buffer[500];
	size_t len = 0;
	ssize_t read;
	FILE * command;
	command = popen("lsmod | tail -n+2 | awk '{printf(\"%s %s\\n\", $1, $2)}'", "r");
	printf("<table>\n");
	printf("<tr>\n");
	printf("<th> Modulo </th>");
	printf("<th> Size </th>");
	printf("</tr>\n");
	while((read = getline(&line, &len, command)) != -1)
	{memset(buffer, '\0', 500);
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

		strcat(buffer, "</tr>\n");
		printf("%s", buffer);
	}
	printf("</table>");
	return 0;
}
