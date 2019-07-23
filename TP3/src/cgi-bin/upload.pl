#!/usr/bin/perl

#print "Content-Type: text/html\n\n";


use CGI;

use strict;

my $PROGNAME = "upload.pl";

my $cgi = new CGI();

my $upfile = $cgi->param('module');

my $filename = GetBasename($upfile);

#system("pwd");

no strict 'refs';

if ($filename =~ m/\.ko$/){
	if (! open(OUTFILE, ">loaded_modules/$filename") ) {
		print "Error abriendo archivo para escritura\n";
		exit(-1);
	}
} else {
	print "Archivo no es del tipo .ko\n";
	exit(-1);
}



my $nBytes = 0;
my $totBytes = 0;
my $buffer = "";
binmode($upfile);
while ( $nBytes = read($upfile, $buffer, 1024) ) {
	print OUTFILE $buffer;
	$totBytes += $nBytes;
}

close(OUTFILE);

system "/var/www/cgi-bin/module_wrapers/insmod /var/www/cgi-bin/loaded_modules/$filename";

use strict 'refs';

print $cgi->redirect('http://192.168.1.14/modulos.html');

sub GetBasename {
	my $fullname = shift;

	my(@parts);
	# check which way our slashes go.
	if ( $fullname =~ /(\\)/ ) {
		@parts = split(/\\/, $fullname);
	} else {
		@parts = split(/\//, $fullname);
	}
	return(pop(@parts));
}



