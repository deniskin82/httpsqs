/**
 * 
 * @(#)HttpsqsException.java    Apr 29, 2010
 * 
 * Copyright 2010 QinYu, Inc. All rights reserved.
 * 
 */
package com.daguu.lib.httpsqs4j;

/**
 * @author Henry Young
 *
 */
public class HttpsqsException extends Exception {
	
	private static final long serialVersionUID = 1L;

	public HttpsqsException() {
		super();
	}
	
	public HttpsqsException(String message) {
		super(message);
	}
	
	public HttpsqsException(String message, Throwable cause) {
		super(message, cause);
	}
	
//	public HttpsqsException

}
