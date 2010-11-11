public class Test {
	 public static void main(String args[])
	 {
		 Httpsqs_client sqs = new Httpsqs_client("192.168.1.102", "1218", "utf-8");
		 String result = null;
		 result = sqs.put("testq", "dsadasd");
		 System.out.println(result);
		 System.out.println("---------------------------");
		 result = sqs.status("testq");
		 System.out.println(result);
		 System.out.println("---------------------------");
		 result = sqs.get("testq");
		 System.out.println(result);
		 System.out.println("---------------------------");
		 result = sqs.reset("testq");
		 System.out.println(result);
	 }
}
