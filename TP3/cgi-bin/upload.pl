#!/usr/bin/perl
##
## file_upload.pl - Demonstration script for file uploads
## over HTML form.
##
## This script should function as is. Copy the file into
## a CGI directory, set the execute permissions, and point
## your browser to it. Then modify if to do something
## useful.
##
## Author: Kyle Dent
## Date: 3/15/01
##

use CGI;
use strict;
use Net::Address::IP::Local;

my $PROGNAME = "upload.pl";

my $cgi = new CGI();

my $upfile = $cgi->param('upfile');

my $basename = GetBasename($upfile);

my $address = Net::Address::IP::Local->public;

no strict 'refs';

if (! open(OUTFILE, "${basename}") ) {
exit(-1);
}
#print "Saving the file to /tmp<br>\n";

my $nBytes = 0;
my $totBytes = 0;
my $buffer = "";
binmode($upfile);
while ( $nBytes = read($upfile, $buffer, 1024) ) {
print OUTFILE $buffer;
$totBytes += $nBytes;
}

close(OUTFILE);

use strict 'refs';


print $cgi->redirect("http://${address}/modules.html");

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

