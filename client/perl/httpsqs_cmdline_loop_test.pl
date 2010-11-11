#!/usr/bin/perl -w

use HttpSQS ;
use Benchmark qw(:all) ;
use v5.8.8 ;

# clear block buffer

$| = 1 ;

my $number = 10 ;

my $mesg = 'aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa' ;

my $sqs = HttpSQS->new("127.0.0.1","1218",'tcp','utf-8') ;

# Just testing in my perl source code
my $tm_0 = new Benchmark ;

for(my $i=1;$i<=$number;$i++)
{
    my $str = $i.$mesg ;
    my $rtn = $sqs->pput("perl_cmdline_test","$str") ;
    print "pput method return value $rtn\n" ;
}
for(my $j=1;$j<=$number;$j++)
{
	my $result = $sqs->pget("perl_cmdline_test") ;
	print "HttpSQS return result $result\n" ;
}
my $tm_1 = new Benchmark ;

my $td = timediff($tm_1,$tm_0) ;

print "The code diff run time:",timestr($td),"\n" ;
