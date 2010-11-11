# set cindent expandtab ts=4 sw=4
# HTTP Simple Queue Service - httpsqs client class for Perl v0.1.0
# httpsqs Project website : http://code.google.com/p/httpsqs/,  
# Author : Tony E-mail: tonny0830@gmail.com
# Complete Date : 2010.3.10
# Bug Commit : If you find any bug, Please Send Email for me! Thanks, -*^_^*-
# This is free software , you are may modify and redistribute it under the GPL License

package HttpSQS ;

use IO::Socket ;
use v5.8.8 ;

$| = 1 ;

my $DEBUG = 0 ;

sub new {
   my $class = shift ;
   (my $host, my $port, my $protocol, my $charset) = @_ ;
   my $ref = {
                Host => $host || '127.0.0.1',
                Port => $port || '1218',
                Protocol => $protocol || 'tcp',
                CharSet => $charset || 'utf-8',
             };

   bless $ref,$class ;
   return $ref ;
}

sub http_get {
   my $self = shift ;
   my $query = shift ;
   my $body = undef ;
   my $header = undef ;

   print "instant method http_get pass \$query values $query\n" if $DEBUG ;

   my $connect = IO::Socket::INET->new(
                                        PeerAddr => $self->{'Host'},
                                        PeerPort => $self->{'Port'},
                                        Proto => $self->{'Protocol'},
                                        Type => SOCK_STREAM) or warn "I can't connect to $self->{'Host'} on port $self->{'Port'} :$!\n" ;

      $connect->autoflush(1) ;

  if($connect)
  {
      my $write_mesg = "GET $query HTTP/1.1\r\n" ;
         $write_mesg .= "Host: $self->{'Host'}\r\n" ;
         $write_mesg .= "Connection: close\r\n" ;
 
         $write_mesg .= "\r\n" ;
      
      print "http_get \$write_mesg variabel values $write_mesg\n" if $DEBUG ;    

      $connect->print($write_mesg) ;
      
      my $line = _trim($connect->getline()) ;

      print "http_get \$line variable values $line\n" if $DEBUG ;

      $header .= $line ;

      print "http_get \$header variable values $header\n" if $DEBUG ;

      (my $proto, my $rcode, my $result) = split(" ",$line) ;   

      my $len = -1 ;
      my $cl = undef ;
      my $close = undef ;

      while(($line = _trim($connect->getline())) ne "")
      {
          print "http_get method readsocket \$line $line\n" if $DEBUG ;

          $header .= $line ;

          print "http_get method variable values \$header $header\n" if $DEBUG ;

          if(_strstr($line, "Content-Length:"))
          {
            ($cl, $len) = split(/ /,$line) ;
          }
          if(_strstr($line, "Connection: close"))
          { 
              $close = 1 ;
          }
      }
      if($len < 0)
      {
          return 0 ;
      }
      
      while((my $read = $connect->getline()))
      {
            $body .= $read ;
      }
      
      if($close)
      {
          close($connect) ;
      }
      print "http_get return \$body values $body\n" if $DEBUG ;

      return $body ;   

  }else {
	return 0 ;

	close($connect) ;
  }
}
sub http_post {
   my $self = shift ;
   my $query = shift ;
   my $body = shift ;
   my $header = undef ;
   my $return_value = undef ;
   my $connect = IO::Socket::INET->new(
                                        PeerAddr => $self->{'Host'},
                                        PeerPort => $self->{'Port'},
                                        Proto => $self->{'Protocol'},
                                        Type => SOCK_STREAM) or warn "I can't connect to $self->{'Host'} on port $self->{'Port'} :$!\n" ;
    
      $connect->autoflush(1) ;

  print "http_post method \$query variable values $query\n" if $DEBUG ;

  if($connect)
  {
      my $write_mesg = "POST $query HTTP/1.1\r\n" ;
         $write_mesg .= "Host: $self->{'Host'}\r\n" ;
         $write_mesg .= "Content-Length: ".length($body)."\r\n" ;
         $write_mesg .= "Connection: close\r\n" ;
         $write_mesg .= "\r\n" ;
         $write_mesg .= $body ;

      print "http_post method \$write_mesg variable values $write_mesg\n" if $DEBUG ;

      $connect->print($write_mesg) ;

      my $line = _trim($connect->getline()) ;

      print "http_post method \$line variable values $line\n" if $DEBUG ;

      $header .= $line ;

      print "http_post method \$header variable values $header\n" if $DEBUG ;

      (my $proto, my $rcode, my $result) = split(" ",$line) ;   

      my $len = -1 ;
      my $cl = undef ;
      my $close = undef ;
    
      while(($line= _trim($connect->getline())) ne "")
      {
          $header .= $line ;

          if(_strstr($line, "Content-Length:"))
          {
            ($cl, $len) = split(/ /,$line) ;
          }
          if(_strstr($line, "Connection: close"))
          { 
              $close = 1 ;
          }
      }
      if($len < 0)
      {
          return 0 ;
      }
       while((my $read = $connect->getline()))
       {
          $return_value .= $read ;
       }

      if($close)
      {
          close($connect) ;
      }

      print "http_post method \$return_value variable vanues $return_value\n" if $DEBUG ;
       
      return $return_value ;   

  }else{

       return 0 ;

       close($connect) ;
  }
}
sub http_pget {
   my $self = shift ;
   my $query = shift ;
   my $body = undef ;
   my $header = undef ;
   my $connect = IO::Socket::INET->new(
                                        PeerAddr => $self->{'Host'},
                                        PeerPort => $self->{'Port'},
                                        Proto => $self->{'Protocol'},
                                        Type => SOCK_STREAM) or warn "I can't connect to $self->{'Host'} on port $self->{'Port'} :$!\n" ;

  $connect->autoflush(1) ;

  print "http_pget method \$query variable values $query\n" if $DEBUG ;

  if($connect)
  {
      my $write_mesg = "GET $query HTTP/1.1\r\n" ;
         $write_mesg .= "Host: $self->{'Host'}\r\n" ;
         $write_mesg .= "Connection: Keep-Alive\r\n" ;
         $write_mesg .= "\r\n" ;
      
      print "http_pget method \$write_mesg variable values $write_mesg\n" if $DEBUG ;

      $connect->print($write_mesg) ;

      my $line = _trim($connect->getline()) ;

      print "http_pget method \$line variable values $line\n" if $DEBUG ;

      $header .= $line ;

      print "http_pget method \$header variable values $header\n" if $DEBUG ;

      (my $proto, my $rcode, my $result) = split(" ",$line) ;   

      my $len = -1 ;
      my $cl = undef ;
      my $close = undef ;

      while(($line= _trim($connect->getline())) ne "")
      {
            $header .= $line ;

            if(_strstr($line,"Content-Length:"))
            {
                ($cl,$len) = split(/ /,$line) ;
            }
            if(_strstr($line,"Connection: close"))
            {
                $close = 1 ;
            }
      }
      if($len < 0)
      {
          return 0 ;
      }

       while((my $read = $connect->getline()))
       {
          $body .= $read ;
       }

      if($close)
      {
          close($connect) ;
      }
      print "http_pget method \$body variable values $body\n" if $DEBUG ;
    
      return $body ;  

  }else{

      return 0 ;

      close($connect) ;
  }
}
sub http_ppost {
   my $self = shift ;
   my $query = shift ;
   my $body = shift ;
   my $header = undef ;
   my $return_value = undef ;
   my $connect = IO::Socket::INET->new(
                                        PeerAddr => $self->{'Host'},
                                        PeerPort => $self->{'Port'},
                                        Proto => $self->{'Protocol'},
                                        Type => SOCK_STREAM) or warn "I can't connect to $self->{'Host'} on port $self->{'Port'} :$!\n" ;
      $connect->autoflush(1) ;
  print "http_ppost method \$query and \$body variable values $query |||||||| $body\n" if $DEBUG ;

  if($connect)
  {
      my $write_mesg = "POST $query HTTP/1.1\r\n" ;
         $write_mesg .= "Host: $self->{'Host'}\r\n" ;
         $write_mesg .= "Content-Length: ".length($body)."\r\n" ;
         $write_mesg .= "Connection: Keep-Alive\r\n" ;
         $write_mesg .= "\r\n" ;
         $write_mesg .= $body ;
      
      print "http_ppost method \$write_mesg variable values $write_mesg\n" if $DEBUG ;

      $connect->print($write_mesg) ;

      my $line = _trim($connect->getline()) ;

      $header .= $line ;
      
      print "http_ppost method \$header and \$line variable values $header ===== $line\n" if $DEBUG ;

      (my $proto, my $rcode, my $result) = split(" ",$line) ;   
	
	  print "http_ppost method \$proto \$rcode \$result variable values $proto\\$rcode\\$result\n" if $DEBUG ;

      my $len = -1 ;
      my $cl = undef ;
      my $close = undef ;

      while(($line= _trim($connect->getline())) ne "")
      {
          print "\$line trim value $line\n" if $DEBUG ;

          print "http_ppost method \$line variable values $line\n" if $DEBUG ;

          $header .= $line ;

          print "http_ppost method \$header variable values $header\n" if $DEBUG ;

          if(_strstr($line, "Content-Length:"))
          {
            ($cl, $len) = split(/ /,$line) ;
          }
          if(_strstr($line, "Connection: close"))
          { 
              $close = 1 ;
          }
      }
      if($len < 0)
      {
          return 0 ;
      }
       while((my $read = $connect->getline()))
       {
          print "http_ppost method \$read variable values $read\n" if $DEBUG ;

          $return_value .= $read ;
       }

      if($close)
      {
          close($connect) ;
      }

      print "http_ppost method \$return_value variable values $return_value\n" if $DEBUG ;

      return $return_value ;   

  }else{

      return 0 ;

      close($connect) ;
  }
}

sub put {
    my $self= shift ;
    my $name = shift ;
    my $data = shift ;
    my $result = $self->http_post("/?charset=".$self->{'CharSet'}."&name=".$name."&opt=put",$data) ;
    
    chomp $result ;

    if($result eq "HTTPSQS_PUT_OK") 
    {
        return 1 ;
    }
    return 0 ;
}
sub get {
    my $self = shift ;
    my $name = shift ;
    my $result = $self->http_get("/?charset=".$self->{'CharSet'}."&name=".$name."&opt=get") ;

    chomp $result ;

    if($result eq "HTTPSQS_ERROR" or !$result)
    {
        return 0 ;
    }
    return $result ;
}
sub status {
    my $self = shift ;
    my $name = shift ;
    my $result = $self->http_get("/?charset=".$self->{'CharSet'}."&name=".$name."&opt=status") ;

    chomp $result ;

    if($result eq "HTTPSQS_ERROR" or !$result)
    {
        return 0 ;
    }
    return $result ;
}
sub view {
    my $self = shift ;
    my $name = shift ;
    my $pos = shift ;
    my $result = $self->http_get("/?charset=".$self->{'CharSet'}."&name=".$name."&opt=view&pos=".$pos) ;

    chomp $result ;

    if($result eq "HTTPSQS_ERROR" or !$result)
    {
        return 0 ;
    }
    return $result ;
}
sub reset {
    my $self = shift ;
    my $name = shift ;
    my $result = $self->http_get("/?charset=".$self->{'CharSet'}."&name=".$name."&opt=reset") ;

    chomp $result ;

    if($result eq "HTTPSQS_RESET_OK")
    {
        return 1 ;
    }
    return 0 ;
}
sub maxqueue {
    my $self = shift ;
    my $name = shift ;
    my $num = shift ;

    my $result = $self->http_get("/?charset=".$self->{'CharSet'}."&name=".$name."&opt=maxqueue&num=".$num) ;

    chomp $result ;

    if($result eq "HTTPSQS_MAXQUEUE_OK")
    {
        return 1 ;
    }
    return 0 ;
}
sub pput {
    my $self = shift ;
    my $name = shift ;
    my $data = shift ;
    
    my $result = $self->http_ppost("/?charset=".$self->{'CharSet'}."&name=".$name."&opt=put",$data) ;

    chomp $result ;
   
    if($result eq "HTTPSQS_PUT_OK")
    {
        return 1 ;
    }
    return 0 ;
}
sub pget {
    my $self = shift ;
    my $name = shift ;

    my $result = $self->http_pget("/?charset=".$self->{'CharSet'}."&name=".$name."&opt=get") ;

    print "HttpSQS pget method return \$result=$result\n" ;

    chomp $result ;

    if($result eq "HTTPSQS_ERROR" or !$result)
    {
        return 0 ;
    }
    return $result ;
}
sub pstatus {
    my $self = shift ;
    my $name = shift ;

    my $result = $self->http_pget("/?charset=".$self->{'CharSet'}."&name=".$name."&opt=status") ;
    
    chomp $result ;

    if($result eq "HTTPSQS_ERROR" or !$result)
    {
        return 0 ;
    }
    return $result ;
}
sub pview {
    my $self = shift ;
    my $name = shift ;
    my $pos = shift ;

    my $result = $self->http_pget("/?charset=".$self->{'CharSet'}."&name=".$name."&opt=view&pos=".$pos) ;
    
    chomp $result ;

    if($result eq "HTTPSQS_ERROR" or !$result)
    {
        return 0 ;
    }
    return $result ;
}
sub preset {
    my $self = shift ;
    my $name = shift ;

    my $result = $self->http_pget("/?charset=".$self->{'CharSet'}."&name=".$name."&opt=reset") ;

    chomp $result ;

    if($result eq "HTTPSQS_RESET_OK")
    {
        return 1 ;
    }
    return 0 ;
}
sub pmaxqueue {
    my $self = shift ;
    my $name = shift ;
    my $num = shift ;

    my $result = $self->http_pget("/?charset=".$self->{'CharSet'}."&name=".$name."&opt=maxqueue&num=".$num) ;

    chomp $result ;

    if($result eq "HTTPSQS_MAXQUEUE_OK")
    {
        return 1 ;
    }
    return 0 ;
}
sub DESTROY {
    my $self = shift ;
    
    %$self = undef ;
} 
############################################################################
# 		
#			Class Internal tools function
#				
############################################################################

# Explain : _trim function delete character string head and tail blanket
sub _trim {
    my $retn = shift ;
    $retn =~ s/^\s*|\s*$//g ;
    return $retn ;
}
sub _strstr {
    my $str = shift ;
    my $match_str = shift ;
    my $str_rtn = undef ;

    if($str =~ m#$match_str#)
    {
        $str_rtn = $&.$' ;
        return $str_rtn ;
    }
     return 0 ;
}
# Explain: _urlencode encode URL character string , _urldecode decode URL character string.
sub _urlencode {
 ($_, my $mlm) = (join('', @_), $*);
 $* = 1;
 s/[^\w!\$'()*,\-.]/sprintf('%%%02x', ord $&)/ge;
 s/ /+/g;
 $* = $mlm;
 return $_;
}
sub _urldecode {
  ($_, my $mlm) = (join('', @_), $*);
  $* = 1;
  s/\+/ /g;
  s/%([\da-f]{2})/pack('C', hex $1)/gie;
  $* = $mlm;
  return $_;
}
1 ;
