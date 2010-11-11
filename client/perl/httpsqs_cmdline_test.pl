#!/usr/bin/perl

# This script Just test programe HttpSQS module 
# Autho : Tony 
# Date : 2010.3.10
# Email : tonny0830@gmail.com

use strict ;
use HttpSQS ;
use v5.8.8 ;

$| = 1 ;

my $sqs = HttpSQS->new("127.0.0.1","1218",'tcp','utf-8') ;

# Just testing in my perl source code
my $put = $sqs->put("perl_cmdline_test","hello") ;

print "HttpSQS put return result $put\n" ;

my $status = $sqs->status("perl_cmdline_test") ;

print "HttpSQS status return result $status\n" ;

my $get = $sqs->get("perl_cmdline_test") ;

print "HttpSQS get return result $get\n" ;

my $view = $sqs->view("perl_cmdline_test",1) ;

print "HttpSQS view return result $view\n" ;

# If reset OK, will return 1, reset fail will return 0 ;
my $reset = $sqs->reset("perl_cmdline_test") ;

print "HttpSQS reset return result $reset\n" ;

# If maxqueue set OK, will return 1, If fail will return 0 ;
my $maxqueue = $sqs->maxqueue("perl_cmdline_test",1_0_0_0) ;

print "HttpSQS maxqueue return result $maxqueue\n" ;

#$print "HttpSQS urlencode return result $urlencode\n" ;
my $pput = $sqs->pput("p_perl_cmdline_test","abcdefghi") ;

print "HttpSQS pput return result $pput\n" ;

# If get Data OK,then will return data,else return 0 ;
my $pget = $sqs->pget("p_perl_cmdline_test") ;

print "HttpSQS pget return result $pget\n" ;

my $pstatus = $sqs->pstatus("p_perl_cmdline_test") ;

print "HttpSQS pstatus return result $pstatus\n" ;

# If view data OK,then will return data, else return 0 
my $pview = $sqs->pview("p_perl_cmdline_test",1) ;

print "HttpSQS pview return result $pview\n" ;

# If set pmaxqueue OK, will return 1, fail return 0 
my $pmaxqueue = $sqs->pmaxqueue("p_perl_cmdline_test",1_0_0_0) ;

print "HttpSQS pmaxqueue return result $pmaxqueue\n" ;

# If reset OK,will return 1, fail will return 0 
my $preset = $sqs->preset("p_perl_cmdline_test") ;

print "HttpSQS preset return result $preset\n" ;
