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
	yearPointer = strtok(input_aux,"&");
	strtok(NULL, "=");
	dayPointer = strtok(NULL, " ");
	printf("year: %s",yearPointer);
	printf("day: %s",dayPointer);

	char command[300];
	strcpy(command, "aws s3 --no-sign-request ls --human-readable --recursive noaa-goes16/ABI-L2-CMIPF/");
	strcat(command,yearPointer);
	strcat(command,"/");
	strcat(command,dayPointer);
	strcat(command,"/");
	strcat(command," | grep M3C13_G16");
	//printf("command: %s", command);
	//system(command);

	printf("Content-Type: text/html\n\n");
	FILE * process;
	char * line = NULL;
	char buffer[500];
	size_t len = 0;
	ssize_t read;
	process = popen(command,"r");

  printf("<html>\n");
  printf("<head>\n");
  printf("<meta http-equiv='content-type' content='text/html; charset=UTF-8'>\n");
  printf("<meta charset='utf-8'>\n");
  printf("<meta name='viewport' content='width=device-width, initial-scale=1, maximum-scale=1'>\n");
  printf("<meta name='author' content='Torce'/>\n");
  printf("<link rel='icon' href='misc/Pi.ico'>\n");
  printf("<title>Raspberry Pi SO2</title>\n");
  printf("<!-----CSS----->\n");
  printf("<link rel='stylesheet' href='https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/css/bootstrap.min.css' integrity='sha384-BVYiiSIFeK1dGmJRAkycuHAHRg32OmUcww7on3RYdg4Va+PmSTsz/K68vbdEjh4u' crossorigin='anonymous'>\n");
  printf("<link rel='stylesheet' href='https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/css/bootstrap-theme.min.css' integrity='sha384-rHyoN1iRsVXV4nD0JutlnGaslCJuC7uwjduW9SVrLvRYooPp2bWYgmgJQIXwl/Sp' crossorigin='anonymous'>\n");
  printf("<link href='https://gitcdn.github.io/bootstrap-toggle/2.2.2/css/bootstrap-toggle.min.css' rel='stylesheet'>\n");
  printf("<link href='css/dashboard.css' rel='stylesheet'>\n");
  printf("<!--Javascript-->\n");
  printf("<script type='text/javascript' src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>\n");
  printf("<script type='text/javascript' src='//maxcdn.bootstrapcdn.com/bootstrap/3.3.7/js/bootstrap.min.js'></script>\n");
  printf("<script src='https://gitcdn.github.io/bootstrap-toggle/2.2.2/js/bootstrap-toggle.min.js'></script>\n");
  printf("</head>\n");
  printf("<body>\n");
  printf("<nav class='navbar navbar-inverse navbar-fixed-top' role='navigation'>\n");
  printf("<div class='container-fluid'>\n");
  printf("<div class='navbar-header'>\n");
  printf("<button type='button' class='navbar-toggle' data-toggle='collapse' data-target='.navbar-collapse'>\n");
  printf("<span class='sr-only'>Toggle navigation</span>\n");
  printf("<span class='icon-bar'></span>\n");
  printf("<span class='icon-bar'></span>\n");
  printf("<span class='icon-bar'></span>\n");
  printf("</button>\n");
  printf("<a class='navbar-brand' href='home.html'>Raspberry Pi SO2</a>\n");
  printf("</div>	\n");
  printf("</div>\n");
  printf("</nav>\n");
  printf("<div class='container-fluid'>\n");
  printf("<div class='row row-offcanvas row-offcanvas-left'>\n");
  printf("<div class='col-sm-3 col-md-2 sidebar-offcanvas' id='sidebar' role='navigation'>\n");
  printf("<ul class='nav nav-sidebar'>\n");
  printf("<li><a href='index.html'>Principal</a></li>\n");
  printf("<li><a href='sistema.html' target=''>Sistema</a></li>\n");
  printf("<li><a href='goes.html' target=''>Goes</a></li>\n");
  printf("<li><a href='modulos.html' target=''>Módulos</a></li>\n");
  printf("<li class='active'><a href='cargarmodulo.html' target=''>Cargar Módulo</a></li>\n");
  printf("</ul>\n");
  printf("</div>\n");
  printf("</div>\n");
  printf("</div>\n");


  printf("<table>\n");
  printf("<tr>\n");
  printf("<th> DOI  </th>");
  printf("<th> Time </th>");
  printf("<th> Size </th>");
  printf("<th> Name </th>");
  printf("</tr>\n");
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
  printf("</table>");



  printf("<footer>\n");
  printf("<p class='pull-right'>Raspberry Pi SO2</p>\n");
  printf("</footer>\n");
  printf("</body>\n");
  printf("</html>\n");

  return 0;
}
