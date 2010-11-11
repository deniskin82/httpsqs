/**
 * 
 * @(#)HttpsqsConnectionInfo.java    Apr 29, 2010
 * 
 * Copyright 2010 QinYu, Inc. All rights reserved.
 * 
 */
package com.daguu.lib.httpsqs4j;

import java.io.IOException;
import java.io.UnsupportedEncodingException;
import java.net.HttpURLConnection;
import java.net.URL;

/**
 * 
 * Httpsqs4j基础类，用于设置连接信息及创建客户端对象
 * 
 * @author Henry Young
 *
 */
public class Httpsqs4j {
	
	protected static String prefix;
	
	protected static String charset;
	
	protected static boolean configured = false;
	
	/**
	 * 设置连接信息
	 * 
	 * @param ip
	 * @param port
	 * @param charset 字符集
	 * @throws HttpsqsException
	 */
	public static void setConnectionInfo(String ip, int port, String charset) throws HttpsqsException {
		try {
			"".getBytes(charset);
		} catch (UnsupportedEncodingException e) {
			throw new HttpsqsException("Unknown charset.", (Throwable) e);
		}
		URL url;
		HttpURLConnection connection = null;
		String prefix = "http://" + ip + ":" + port + "/";
		try {
			url = new URL(prefix);
			connection = (HttpURLConnection) url.openConnection();
			connection.connect();
		} catch (IOException e) {
			throw new HttpsqsException(prefix + " cannot located.", (Throwable) e);
		} finally {
			if (connection != null) {
				connection.disconnect();
			}
		}
		Httpsqs4j.prefix = prefix + "?";
		Httpsqs4j.charset = charset;
		Httpsqs4j.configured = true;
	}
	
	/**
	 * 创建新的客户端对象
	 * 
	 * @return
	 */
	public static HttpsqsClient createNewClient() {
		return new HttpsqsClient();
	}

}
