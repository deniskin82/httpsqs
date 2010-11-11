<?php
include_once("httpsqs_client.php");
$httpsqs = new httpsqs;

$message = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
aaaaaaaaaaaaaaaaaaaaaa";

$number = 20000;

/* test queue put */
echo "Test Queue PUT, please waitting ...\n";
$start_time = microtime(true);
for ($i=1;$i<=$number;$i++){
    $httpsqs->pput("127.0.0.1", 1218, "utf-8", "command_line_test", $i.$message);
}
$run_time = microtime(true) - $start_time;
echo "PUT ".$number." messages. Run Time for Queue PUT: $run_time sec, ".$number/$run_time." requests/sec\n";
ob_flush();

/* test queue get */
echo "Test Queue GET, please waitting ...\n";
$start_time = microtime(true);
for ($i=1;$i<=$number;$i++){
    $result = $httpsqs->pget("127.0.0.1", 1218, "utf-8", "command_line_test");
    //echo($result."\n");
}
$run_time = microtime(true) - $start_time;
echo "GET ".$number." messages. Run Time for Queue GET: $run_time sec, ".$number/$run_time." requests/sec\n";
?>
