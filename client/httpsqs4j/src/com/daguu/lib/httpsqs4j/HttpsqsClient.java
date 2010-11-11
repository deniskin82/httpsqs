/**
 * 
 * @(#)HttpsqsClient.java    Apr 29, 2010
 * 
 * Copyright 2010 QinYu, Inc. All rights reserved.
 * 
 */
package com.daguu.lib.httpsqs4j;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.net.HttpURLConnection;
import java.net.URL;
import java.util.regex.Matcher;

/**
 * 
 * 客户端类，使用Httpsqs4j类的createNewClient()方法创建，创建前请先调用Httpsqs4j的setConnectionInfo设置连接信息
 * 
 * @author Henry Young
 *
 */
public class HttpsqsClient {
	
	private long pos;
	
	protected HttpsqsClient() {
		
	}
	
	private String httpPost(String urlStr, String postData) throws HttpsqsException {
		return this.getSource(urlStr, postData);
	}
	
	private String httpGet(String urlStr) throws HttpsqsException {
		return this.getSource(urlStr, null);
	}
	
	private String getSource(String urlStr, String postData) throws HttpsqsException {
		HttpURLConnection connection = null;
		InputStream is = null;
		InputStreamReader isr = null;
		BufferedReader reader = null;
		OutputStream os = null;
		OutputStreamWriter osw = null;
		StringBuffer sb = new StringBuffer();
		try {
			URL url = new URL(Httpsqs4j.prefix + urlStr);
			connection = (HttpURLConnection) url.openConnection();
			if (postData != null) {
				try {
					connection.setDoOutput(true);
					os = connection.getOutputStream();
					osw = new OutputStreamWriter(os);
					osw.write(postData);
					osw.flush();
				} catch (IOException e) {
					throw new HttpsqsException("Send data error.", (Throwable) e);
				} finally {
					if (osw != null) {
						osw.close();
					}
					if (os != null) {
						os.close();
					}
				}
			}
			is = connection.getInputStream();
			isr = new InputStreamReader(is, Httpsqs4j.charset);
			reader = new BufferedReader(isr);
			String line = "";
			while ((line = reader.readLine()) != null) {
				sb.append(line).append('\n');
			}
			String pos = connection.getHeaderField("Pos");
			if (pos != null) {
				this.pos = Long.valueOf(pos);
			}
		} catch (IOException e) {
			throw new HttpsqsException("Cannot connect to server.", (Throwable) e);
		} finally {
			if (reader != null) {
				try {
					reader.close();
				} catch (IOException e) {
					e.printStackTrace();
				}
			}
			if (isr != null) {
				try {
					isr.close();
				} catch (IOException e) {
					e.printStackTrace();
				}
			}
			if (is != null) {
				try {
					is.close();
				} catch (IOException e) {
					e.printStackTrace();
				}
			}
			if (connection != null) {
				connection.disconnect();
			}
		}
		String sbs = sb.toString();
		if (sbs.contains("HTTPSQS_ERROR")) {
			throw new HttpsqsException("Global error.");
		}
		return sb.toString();
	}
	
	/**
	 * 获取最后一次出入队列操作的位置值
	 * 
	 * @return
	 */
	public long getLastPos() {
		return this.pos;
	}
	
	/**
	 * 获取HttpSQS状态
	 * 
	 * @param queueName 队列名称
	 * @return
	 * @throws HttpsqsException
	 */
	public HttpsqsStatus getStatus(String queueName) throws HttpsqsException {
		String urlStr = "opt=status&name=" + queueName;
		String source = this.httpGet(urlStr);
		Matcher matcher = HttpsqsStatus.pattern.matcher(source);
		if (matcher.find()) {
			HttpsqsStatus status = new HttpsqsStatus();
			status.version = matcher.group(1);
			status.queueName = matcher.group(2);
			status.maxNumber = Long.parseLong(matcher.group(3));
			status.getLap = Long.parseLong(matcher.group(4));
			status.getPosition = Long.parseLong(matcher.group(5));
			status.putLap = Long.parseLong(matcher.group(6));
			status.putPosition = Long.parseLong(matcher.group(7));
			status.unreadNumber = Long.parseLong(matcher.group(8));
			return status;
		}
		return null;
	}
	
	/**
	 * 将字符串加入队列
	 * 
	 * @param queueName 队列名称
	 * @param str 字符串
	 * @throws HttpsqsException
	 */
	public void putString(String queueName, String str) throws HttpsqsException {
		String urlStr = "opt=put&name=" + queueName;
		String source = this.httpPost(urlStr, str);
		if (source.contains("HTTPSQS_PUT_END")) {
			throw new HttpsqsException("Queue [" + queueName + "] fulled.");
		} else if (source.contains("HTTPSQS_PUT_ERROR")) {
			throw new HttpsqsException("Put data to queue [" + queueName + "] failed.");
		}
	}
	
	/**
	 * 将字符串出队列
	 * 
	 * @param queueName 队列名称
	 * @return
	 * @throws HttpsqsException
	 */
	public String getString(String queueName) throws HttpsqsException {
		String urlStr = "opt=get&charset=" + Httpsqs4j.charset + "&name=" + queueName;
		String source = this.httpGet(urlStr);
		if (source.contains("HTTPSQS_GET_END")) {
			throw new HttpsqsException("There's no data in queue [" + queueName + "].");
		}
		return source;
	}
	
	/**
	 * 获取某位置的字符串
	 * 
	 * @param queueName 队列名称
	 * @param pos 位置
	 * @return
	 * @throws HttpsqsException
	 */
	public String getStringAt(String queueName, long pos) throws HttpsqsException {
		if (pos < 1 || pos > 1000000000l) {
			throw new HttpsqsException("Pos' out of range[1 - 1000000000].");
		}
		String urlStr = "opt=view&charset=" + Httpsqs4j.charset + "&name=" + queueName + "&pos=" + pos;
		return this.httpGet(urlStr);
	}
	
	/**
	 * 重置队列
	 * 
	 * @param queueName 队列名称
	 * @return
	 * @throws HttpsqsException
	 */
	public boolean reset(String queueName) throws HttpsqsException {
		String urlStr = "opt=reset&name=" + queueName;
		String source = this.httpGet(urlStr);
		return source.contains("HTTPSQS_RESET_OK");
	}
	
	/**
	 * 设置最大队列数量
	 * 
	 * @param queueName 队列名称
	 * @param number 最大数量
	 * @return
	 * @throws HttpsqsException
	 */
	public boolean setMaxNumber(String queueName, long number) throws HttpsqsException {
		if (pos < 10 || pos > 1000000000l) {
			throw new HttpsqsException("Pos' out of range[10 - 1000000000].");
		}
		String urlStr = "opt=maxqueue&name=" + queueName + "&num=" + number;
		String source = this.httpGet(urlStr);
		return source.contains("HTTPSQS_MAXQUEUE_OK");
	}
	
}
