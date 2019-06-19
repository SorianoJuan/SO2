#!/usr/bin/perl

use CGI;

use strict;

my $cgi = new CGI();

my $modulename = $cgi->param('module');

no strict 'refs';

system "/var/www/cgi-bin/module_wrapers/rmmod $modulename";

print $cgi->redirect('http://192.168.1.14/modulos.html');
