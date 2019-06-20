#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main(void)
{
	printf("Content-Type: text/html\n\n");
	char * lineread = NULL;
	char buffer[1000];
	size_t len = 0;
	ssize_t data;
	FILE * exec;
	exec = popen("lsmod | tail -n+2 | awk '{printf(\"%s %s\\n\", $1, $2)}'", "r");
	printf("<table class='table'>\n");
	printf("<tr>\n");
	printf("<th> Modulo </th>");
	printf("<th> Size </th>");
	printf("</tr>\n");
	while((data = getline(&lineread, &len, exec)) != -1)
	{memset(buffer, '\0', 500);
		lineread[data-1] = '\0';
		strcat(buffer, "<tr>\n");
		char * text = strtok(lineread," ");
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
