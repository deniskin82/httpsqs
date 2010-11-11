/**
 * 
 * @(#)HttpsqsStatus.java    Apr 29, 2010
 * 
 * Copyright 2010 QinYu, Inc. All rights reserved.
 * 
 */
package com.daguu.lib.httpsqs4j;

import java.util.regex.Pattern;

/**
 * @author Henry Young
 *
 */
public class HttpsqsStatus {
	
	/**
	 * HttpSQS版本
	 */
	public String version;
	
	/**
	 * 队列名称
	 */
	public String queueName;
	
	/**
	 * 队列最大数量
	 */
	public long maxNumber;
	
	/**
	 * 当前入队位置
	 */
	public long putPosition;
	
	/**
	 * 当前入队圈数
	 */
	public long putLap;
	
	/**
	 * 当前出队位置
	 */
	public long getPosition;
	
	/**
	 * 当前出队圈数
	 */
	public long getLap;
	
	/**
	 * 当前未出队数量
	 */
	public long unreadNumber;
	
	protected static Pattern pattern = Pattern.compile("HTTP Simple Queue Service v(.+?)\\s(?:.+?)\\sQueue Name: (.+?)\\sMaximum number of queues: (\\d+)\\sPut position of queue \\((\\d+)st lap\\): (\\d+)\\sGet position of queue \\((\\d+)st lap\\): (\\d+)\\sNumber of unread queue: (\\d+)");

}
