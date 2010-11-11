/*
 * 不包括keep-alive的部分，get方法只能获得数据，无法获得pos
 * 作者：李博 lb13810398408@gmail.com
 */

import java.io.*;
import java.net.*;

class Httpsqs_client {
	private String server, port, charset;

	public Httpsqs_client(String server, String port, String charset) {
		this.server = server;
		this.port = port;
		this.charset = charset;
	}
	
	private String doprocess(String urlstr) {
		URL url = null;
		try {
			url = new URL(urlstr);
		} catch (MalformedURLException e) {
			return "The httpsqs server must be error";
		}
		try {
			BufferedReader instream = new BufferedReader(new InputStreamReader(url.openStream()));
			String s = null;
			StringBuffer result = new StringBuffer("");
			
			while((s = instream.readLine()) != null)
			{
				result.append(s);
			}
			instream.close();
			return result.toString();
		} catch (IOException e) {
			return "Get data error";
		}
	}
	
	public String maxqueue(String queue_name, String num) {
		String urlstr = "http://" + this.server + ":" + this.port + "/?name=" + queue_name + "&opt=maxqueue&num=" + num;
		String result = null;
		
		result = this.doprocess(urlstr);
		return result;
	}
	
	public String reset(String queue_name) {
		String urlstr = "http://" + this.server + ":" + this.port + "/?name=" + queue_name + "&opt=reset";
		String result = null;
		
		result = this.doprocess(urlstr);
		return result;
	}
	
	public String view(String queue_name, String pos) {
		String urlstr = "http://" + this.server + ":" + this.port + "/?charset=" + this.charset + "&name=" + queue_name + "&opt=view&pos=" + pos;
		String result = null;
		
		result = this.doprocess(urlstr);
		return result;
	}
	
	public String status(String queue_name) {
		String urlstr = "http://" + this.server + ":" + this.port + "/?name=" + queue_name + "&opt=status";
		String result = null;
		
		result = this.doprocess(urlstr);
		return result;
	}
	
	public String get(String queue_name) {
		String urlstr = "http://" + this.server + ":" + this.port + "/?charset=" + this.charset + "&name=" + queue_name + "&opt=get";
		String result = null;
		
		result = this.doprocess(urlstr);
		return result;
	}

	public String put(String queue_name, String data) {
		String urlstr = "http://" + this.server + ":" + this.port + "/?name=" + queue_name + "&opt=put";
		URL url = null;
		try {
			url = new URL(urlstr);
		} catch (MalformedURLException e) {
			return "The httpsqs server must be error";
		}

		URLConnection conn = null;

		try {
			conn = url.openConnection();
			conn.setDoOutput(true);
			OutputStreamWriter out = null;
			out = new OutputStreamWriter(conn.getOutputStream());
			out.write(data);
			out.flush();
			out.close();
		} catch (IOException e) {
			return "Put data error";
		}

		BufferedReader reader = null;
		
		try {
			reader = new BufferedReader(new InputStreamReader(conn.getInputStream()));
			String line = null;
			while ((line = reader.readLine()) != null) {
				return line;
			}
		} catch (IOException e) {
			return "Get return data error";
		}
		return null;
	}
}