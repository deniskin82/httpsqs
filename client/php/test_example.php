<?php
include_once("httpsqs_client.php");
$httpsqs = new httpsqs;
$result = $httpsqs->put("127.0.0.1", 1218, "utf-8", "your_queue_name1", urlencode("text_message1"));
echo "###1.put result:\r\n";
var_dump($result);
echo "\r\n\r\n";

$result = $httpsqs->get("127.0.0.1", 1218, "utf-8", "your_queue_name1");
echo "###2.get result:\r\n";
var_dump($result);
echo "\r\n\r\n";

$result = $httpsqs->put("127.0.0.1", 1218, "utf-8", "your_queue_name1", urlencode("text_message2"));
echo "###3.put result:\r\n";
var_dump($result);
echo "\r\n\r\n";

$result = $httpsqs->gets("127.0.0.1", 1218, "utf-8", "your_queue_name1");
echo "###4.gets result:\r\n";
var_dump($result);
echo "\r\n\r\n";

$result = $httpsqs->pput("127.0.0.1", 1218, "gb2312", "your_queue_name2", urlencode("text_message3"));
echo "###5.pput result:\r\n";
var_dump($result);
echo "\r\n\r\n";

$result = $httpsqs->pget("127.0.0.1", 1218, "gb2312", "your_queue_name2");
echo "###6.pget result:\r\n";
var_dump($result);
echo "\r\n\r\n";

$result = $httpsqs->status("127.0.0.1", 1218, "utf-8", "your_queue_name1");
echo "###7.status result:\r\n";
var_dump($result);
echo "\r\n\r\n";

$result = $httpsqs->status_json("127.0.0.1", 1218, "utf-8", "your_queue_name1");
echo "###8.status_json result:\r\n";
var_dump($result);
echo "\r\n\r\n";

$result = $httpsqs->view("127.0.0.1", 1218, "utf-8", "your_queue_name1", 1);
echo "###9.view result:\r\n";
var_dump($result);
echo "\r\n\r\n";

$result = $httpsqs->reset("127.0.0.1", 1218, "utf-8", "your_queue_name1");
echo "###10.reset result:\r\n";
var_dump($result);
echo "\r\n\r\n";

$result = $httpsqs->maxqueue("127.0.0.1", 1218, "utf-8", "your_queue_name1", 5000000);
echo "###11.maxqueue result:\r\n";
var_dump($result);
echo "\r\n\r\n";

$result = $httpsqs->synctime("127.0.0.1", 1218, "utf-8", "your_queue_name1", 10);
echo "###12.synctime result:\r\n";
var_dump($result);
echo "\r\n\r\n";
?>
