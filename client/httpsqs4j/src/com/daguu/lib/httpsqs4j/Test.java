/**
 * 
 * @(#)Tester.java    Apr 29, 2010
 * 
 * Copyright 2010 QinYu, Inc. All rights reserved.
 * 
 */
package com.daguu.lib.httpsqs4j;

/**
 * @author Henry Young
 *
 */
public class Test {
	
	public static void main(String[] args) {
		try {
			Httpsqs4j.setConnectionInfo("192.168.1.201", 1218, "UTF-8");
			HttpsqsClient client = Httpsqs4j.createNewClient();
			HttpsqsStatus status = client.getStatus("asdf");
			System.out.println(status.version);
			System.out.println(status.queueName);
			System.out.println(status.maxNumber);;
			System.out.println(status.getLap);;
			System.out.println(status.getPosition);
			System.out.println(status.putLap);
			System.out.println(status.putPosition);
			System.out.println(status.unreadNumber);
			client.putString("asdf", "test");
			System.out.println(client.getString("asdf"));
		} catch (HttpsqsException e) {
			e.printStackTrace();
		}
	}

}
