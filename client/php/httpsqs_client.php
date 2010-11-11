<?php
/*
----------------------------------------------------------------------------------------------------------------
HTTP Simple Queue Service - httpsqs client class for PHP v1.3

Author: Zhang Yan (http://blog.s135.com), E-mail: net@s135.com
This is free software, and you are welcome to modify and redistribute it under the New BSD License
----------------------------------------------------------------------------------------------------------------
Useage:
<?php
include_once("httpsqs_client.php");
$httpsqs = new httpsqs;

//http connect without Keep-Alive  (Use in Webserver)
$result = $httpsqs->put($host, $port, $charset, $name, $data); //1. PUT text message into a queue. If PUT successful, return boolean: true. If an error occurs, return boolean: false. If queue full, return text: HTTPSQS_PUT_END
$result = $httpsqs->get($host, $port, $charset, $name); //2. GET text message from a queue. Return the queue contents. If an error occurs, return boolean: false. If there is no unread queue message, return text: HTTPSQS_GET_END
$result = $httpsqs->gets($host, $port, $charset, $name); //3. GET text message and pos from a queue. Return example: array("pos" => 7, "data" => "text message"). If an error occurs, return boolean: false. If there is no unread queue message, return: array("pos" => 0, "data" => "HTTPSQS_GET_END")
$result = $httpsqs->status($host, $port, $charset, $name); //4. View queue status
$result = $httpsqs->status_json($host, $port, $charset, $name); //5. View queue status in json. Return example: {"name":"queue_name","maxqueue":5000000,"putpos":130,"putlap":1,"getpos":120,"getlap":1,"unread":10}
$result = $httpsqs->view($host, $port, $charset, $name, $pos); //6. View the contents of the specified queue pos (id). Return the contents of the specified queue pos.
$result = $httpsqs->reset($host, $port, $charset, $name); //7. Reset the queue. If reset successful, return boolean: true. If an error occurs, return boolean: false
$result = $httpsqs->maxqueue($host, $port, $charset, $name, $num); //8. Change the maximum queue length of per-queue. If change the maximum queue length successful, return boolean: true. If  it be cancelled, return boolean: false
$result = $httpsqs->synctime($host, $port, $charset, $name, $num); //9. Change the interval to sync updated contents to the disk. If change the interval successful, return boolean: true. If  it be cancelled, return boolean: false

//http pconnect with Keep-Alive (Very fast in Command line mode)
$result = $httpsqs->pput($host, $port, $charset, $name, $data); //1. PUT text message into a queue. If PUT successful, return boolean: true. If an error occurs, return boolean: false.  If queue full, return text: HTTPSQS_PUT_END
$result = $httpsqs->pget($host, $port, $charset, $name); //2. GET text message from a queue. Return the queue contents. If an error occurs, return boolean: false. If there is no unread queue message, return text: HTTPSQS_GET_END
$result = $httpsqs->pgets($host, $port, $charset, $name); //3. GET text message and pos from a queue. Return example: array("pos" => 7, "data" => "text message"). If there is no unread queue message, return: array("pos" => 0, "data" => "HTTPSQS_GET_END")
$result = $httpsqs->pstatus($host, $port, $charset, $name); //4. View queue status
$result = $httpsqs->pstatus_json($host, $port, $charset, $name); //5. View queue status in json. Return example: {"name":"queue_name","maxqueue":5000000,"putpos":130,"putlap":1,"getpos":120,"getlap":1,"unread":10}
$result = $httpsqs->pview($host, $port, $charset, $name, $pos); //6. View the contents of the specified queue pos (id). Return the contents of the specified queue pos.
$result = $httpsqs->preset($host, $port, $charset, $name); //7. Reset the queue. If reset successful, return boolean: true. If an error occurs, return boolean: false
$result = $httpsqs->pmaxqueue($host, $port, $charset, $name, $num); //8. Change the maximum queue length of per-queue. If change the maximum queue length successful, return boolean: true. If  it be cancelled, return boolean: false
$result = $httpsqs->psynctime($host, $port, $charset, $name, $num); //9. Change the interval to sync updated contents to the disk. If change the interval successful, return boolean: true. If  it be cancelled, return boolean: false
?>
----------------------------------------------------------------------------------------------------------------
*/

$GLOBAL_HTTPSQS_PSOCKET = false;

class httpsqs
{
    function http_get($host, $port, $query)
    {
        $httpsqs_socket = @fsockopen($host, $port, $errno, $errstr, 5);
        if (!$httpsqs_socket)
        {
            return false;
        }
        $out = "GET ${query} HTTP/1.1\r\n";
        $out .= "Host: ${host}\r\n";
        $out .= "Connection: close\r\n";
        $out .= "\r\n";
        fwrite($httpsqs_socket, $out);
        $line = trim(fgets($httpsqs_socket));
        $header = $line;
        list($proto, $rcode, $result) = explode(" ", $line);
        $len = -1;
        while (($line = trim(fgets($httpsqs_socket))) != "")
        {
            $header .= $line;
            if (strstr($line, "Content-Length:"))
            {
                list($cl, $len) = explode(" ", $line);
            }
            if (strstr($line, "Pos:"))
            {
                list($pos_key, $pos_value) = explode(" ", $line);
            }
        }
        if ($len < 0)
        {
            return false;
        }
        $body = @fread($httpsqs_socket, $len);
        fclose($httpsqs_socket);
		$result_array["pos"] = (int)$pos_value;
		$result_array["data"] = $body;
        return $result_array;
    }

    function http_post($host, $port, $query, $body)
    {
        $httpsqs_socket = @fsockopen($host, $port, $errno, $errstr, 5);
        if (!$httpsqs_socket)
        {
            return false;
        }
        $out = "POST ${query} HTTP/1.1\r\n";
        $out .= "Host: ${host}\r\n";
        $out .= "Content-Length: " . strlen($body) . "\r\n";
        $out .= "Connection: close\r\n";
        $out .= "\r\n";
        $out .= $body;
        fwrite($httpsqs_socket, $out);
        $line = trim(fgets($httpsqs_socket));
        $header = $line;
        list($proto, $rcode, $result) = explode(" ", $line);
        $len = -1;
        while (($line = trim(fgets($httpsqs_socket))) != "")
        {
            $header .= $line;
            if (strstr($line, "Content-Length:"))
            {
                list($cl, $len) = explode(" ", $line);
            }
            if (strstr($line, "Pos:"))
            {
                list($pos_key, $pos_value) = explode(" ", $line);
            }
        }
        if ($len < 0)
        {
            return false;
        }
        $body = @fread($httpsqs_socket, $len);
        fclose($httpsqs_socket);
		$result_array["pos"] = (int)$pos_value;
		$result_array["data"] = $body;
        return $result_array;
    }
	
    function http_pget($host, $port, $query)
    {
		global $GLOBAL_HTTPSQS_PSOCKET;
		$hostport = md5($host.":".$port);
        if (!$GLOBAL_HTTPSQS_PSOCKET[$hostport])
        {
			$GLOBAL_HTTPSQS_PSOCKET[$hostport] = @pfsockopen($host, $port, $errno, $errstr, 5);
        }
        if (!$GLOBAL_HTTPSQS_PSOCKET[$hostport])
        {
            return false;
        }
        $out = "GET ${query} HTTP/1.1\r\n";
        $out .= "Host: ${host}\r\n";
        $out .= "Connection: Keep-Alive\r\n";
        $out .= "\r\n";
        fwrite($GLOBAL_HTTPSQS_PSOCKET[$hostport], $out);
        $line = trim(fgets($GLOBAL_HTTPSQS_PSOCKET[$hostport]));
		if (empty($line) == true) {
			unset($GLOBAL_HTTPSQS_PSOCKET[$hostport]);
			$GLOBAL_HTTPSQS_PSOCKET[$hostport] = @pfsockopen($host, $port, $errno, $errstr, 5);
			if ($GLOBAL_HTTPSQS_PSOCKET[$hostport])
			{
				fwrite($GLOBAL_HTTPSQS_PSOCKET[$hostport], $out);
				$line = trim(fgets($GLOBAL_HTTPSQS_PSOCKET[$hostport]));
			} else {
				return false;
			}
		}
        $header = $line;
        list($proto, $rcode, $result) = explode(" ", $line);
        $len = -1;
        while (($line = trim(fgets($GLOBAL_HTTPSQS_PSOCKET[$hostport]))) != "")
        {
            $header .= $line;
            if (strstr($line, "Content-Length:"))
            {
                list($cl, $len) = explode(" ", $line);
            }
            if (strstr($line, "Pos:"))
            {
                list($pos_key, $pos_value) = explode(" ", $line);
            }			
        }
        if ($len < 0)
        {
            return false;
        }
        $body = @fread($GLOBAL_HTTPSQS_PSOCKET[$hostport], $len);
		$result_array["pos"] = (int)$pos_value;
		$result_array["data"] = $body;
        return $result_array;
    }

    function http_ppost($host, $port, $query, $body)
    {
		global $GLOBAL_HTTPSQS_PSOCKET;
		$hostport = md5($host.":".$port);
        if (!$GLOBAL_HTTPSQS_PSOCKET[$hostport])
        {
			$GLOBAL_HTTPSQS_PSOCKET[$hostport] = @pfsockopen($host, $port, $errno, $errstr, 5);
        }
        if (!$GLOBAL_HTTPSQS_PSOCKET[$hostport])
        {
            return false;
        }
        $out = "POST ${query} HTTP/1.1\r\n";
        $out .= "Host: ${host}\r\n";
        $out .= "Content-Length: " . strlen($body) . "\r\n";
        $out .= "Connection: Keep-Alive\r\n";
        $out .= "\r\n";
        $out .= $body;
        fwrite($GLOBAL_HTTPSQS_PSOCKET[$hostport], $out);
        $line = trim(fgets($GLOBAL_HTTPSQS_PSOCKET[$hostport]));
		if (empty($line) == true) {
			unset($GLOBAL_HTTPSQS_PSOCKET[$hostport]);
			$GLOBAL_HTTPSQS_PSOCKET[$hostport] = @pfsockopen($host, $port, $errno, $errstr, 5);
			if ($GLOBAL_HTTPSQS_PSOCKET[$hostport])
			{
				fwrite($GLOBAL_HTTPSQS_PSOCKET[$hostport], $out);
				$line = trim(fgets($GLOBAL_HTTPSQS_PSOCKET[$hostport]));
			} else {
				return false;
			}
		}		
        $header = $line;
        list($proto, $rcode, $result) = explode(" ", $line);
        $len = -1;
        while (($line = trim(fgets($GLOBAL_HTTPSQS_PSOCKET[$hostport]))) != "")
        {
            $header .= $line;
            if (strstr($line, "Content-Length:"))
            {
                list($cl, $len) = explode(" ", $line);
            }
            if (strstr($line, "Pos:"))
            {
                list($pos_key, $pos_value) = explode(" ", $line);
            }
        }
        if ($len < 0)
        {
            return false;
        }
        $body = @fread($GLOBAL_HTTPSQS_PSOCKET[$hostport], $len);
		$result_array["pos"] = (int)$pos_value;
		$result_array["data"] = $body;
        return $result_array;
    }
    
	/* ------------------ http connect without Keep-Alive  (Use in Webserver)  ------------------------ */
	
    function put($host, $port, $charset='utf-8', $name, $data)
    {
    	$result = $this->http_post($host, $port, "/?charset=".$charset."&name=".$name."&opt=put", $data);
		if ($result["data"] == "HTTPSQS_PUT_OK") {
			return true;
		} else if ($result["data"] == "HTTPSQS_PUT_END") {
			return $result["data"];
		}
		return false;
    }
    
    function get($host, $port, $charset='utf-8', $name)
    {
    	$result = $this->http_get($host, $port, "/?charset=".$charset."&name=".$name."&opt=get");
		if ($result == false || $result["data"] == "HTTPSQS_ERROR" || $result["data"] == false) {
			return false;
		}
        return $result["data"];
    }
	
    function gets($host, $port, $charset='utf-8', $name)
    {
    	$result = $this->http_get($host, $port, "/?charset=".$charset."&name=".$name."&opt=get");
		if ($result == false || $result["data"] == "HTTPSQS_ERROR" || $result["data"] == false) {
			return false;
		}
        return $result;
    }	
	
    function status($host, $port, $charset='utf-8', $name)
    {
    	$result = $this->http_get($host, $port, "/?charset=".$charset."&name=".$name."&opt=status");
		if ($result == false || $result["data"] == "HTTPSQS_ERROR" || $result["data"] == false) {
			return false;
		}
        return $result["data"];
    }
	
    function view($host, $port, $charset='utf-8', $name, $pos)
    {
    	$result = $this->http_get($host, $port, "/?charset=".$charset."&name=".$name."&opt=view&pos=".$pos);
		if ($result == false || $result["data"] == "HTTPSQS_ERROR" || $result["data"] == false) {
			return false;
		}
        return $result["data"];
    }
	
    function reset($host, $port, $charset='utf-8', $name)
    {
    	$result = $this->http_get($host, $port, "/?charset=".$charset."&name=".$name."&opt=reset");
		if ($result["data"] == "HTTPSQS_RESET_OK") {
			return true;
		}
        return false;
    }
	
    function maxqueue($host, $port, $charset='utf-8', $name, $num)
    {
    	$result = $this->http_get($host, $port, "/?charset=".$charset."&name=".$name."&opt=maxqueue&num=".$num);
		if ($result["data"] == "HTTPSQS_MAXQUEUE_OK") {
			return true;
		}
        return false;
    }
	
    function status_json($host, $port, $charset='utf-8', $name)
    {
    	$result = $this->http_get($host, $port, "/?charset=".$charset."&name=".$name."&opt=status_json");
		if ($result == false || $result["data"] == "HTTPSQS_ERROR" || $result["data"] == false) {
			return false;
		}
        return $result["data"];
    }

    function synctime($host, $port, $charset='utf-8', $name, $num)
    {
    	$result = $this->http_get($host, $port, "/?charset=".$charset."&name=".$name."&opt=synctime&num=".$num);
		if ($result["data"] == "HTTPSQS_SYNCTIME_OK") {
			return true;
		}
        return false;
    }	
	
	/* ------------------ http pconnect with Keep-Alive (Very fast in Command line mode)  ------------------------ */	
	
    function pput($host, $port, $charset='utf-8', $name, $data)
    {
    	$result = $this->http_ppost($host, $port, "/?charset=".$charset."&name=".$name."&opt=put", $data);
		if ($result["data"] == "HTTPSQS_PUT_OK") {
			return true;
		} else if ($result["data"] == "HTTPSQS_PUT_END") {
			return $result["data"];
		}
		return false;
    }
    
    function pget($host, $port, $charset='utf-8', $name)
    {
    	$result = $this->http_pget($host, $port, "/?charset=".$charset."&name=".$name."&opt=get");	
		if ($result == false || $result["data"] == "HTTPSQS_ERROR" || $result["data"] == false) {
			return false;
		}
        return $result["data"];
    }
	
    function pgets($host, $port, $charset='utf-8', $name)
    {
    	$result = $this->http_pget($host, $port, "/?charset=".$charset."&name=".$name."&opt=get");	
		if ($result == false || $result["data"] == "HTTPSQS_ERROR" || $result["data"] == false) {
			return false;
		}
        return $result;
    }	
	
    function pstatus($host, $port, $charset='utf-8', $name)
    {
    	$result = $this->http_pget($host, $port, "/?charset=".$charset."&name=".$name."&opt=status");
		if ($result == false || $result["data"] == "HTTPSQS_ERROR" || $result["data"] == false) {
			return false;
		}
        return $result["data"];
    }
	
    function pview($host, $port, $charset='utf-8', $name, $pos)
    {
    	$result = $this->http_pget($host, $port, "/?charset=".$charset."&name=".$name."&opt=view&pos=".$pos);
		if ($result == false || $result["data"] == "HTTPSQS_ERROR" || $result["data"] == false) {
			return false;
		}
        return $result["data"];
    }
	
    function preset($host, $port, $charset='utf-8', $name)
    {
    	$result = $this->http_pget($host, $port, "/?charset=".$charset."&name=".$name."&opt=reset");
		if ($result["data"] == "HTTPSQS_RESET_OK") {
			return true;
		}
        return false;
    }
	
    function pmaxqueue($host, $port, $charset='utf-8', $name, $num)
    {
    	$result = $this->http_pget($host, $port, "/?charset=".$charset."&name=".$name."&opt=maxqueue&num=".$num);
		if ($result["data"] == "HTTPSQS_MAXQUEUE_OK") {
			return true;
		}
        return false;
    }
	
    function pstatus_json($host, $port, $charset='utf-8', $name)
    {
    	$result = $this->http_pget($host, $port, "/?charset=".$charset."&name=".$name."&opt=status_json");
		if ($result == false || $result["data"] == "HTTPSQS_ERROR" || $result["data"] == false) {
			return false;
		}
        return $result["data"];
    }

    function psynctime($host, $port, $charset='utf-8', $name, $num)
    {
    	$result = $this->http_pget($host, $port, "/?charset=".$charset."&name=".$name."&opt=synctime&num=".$num);
		if ($result["data"] == "HTTPSQS_SYNCTIME_OK") {
			return true;
		}
        return false;
    }	
}
?>
