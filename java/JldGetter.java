package jld;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.net.HttpURLConnection;
import java.net.URL;
import java.util.LinkedHashMap;
import java.util.ArrayList;

import javax.management.BadStringOperationException;

/**
 * Opens and parses the JLD data in a file
 */
public class JldGetter implements IJLDGlobalFinals {
	private final String default_dir;
	HttpURLConnection connection;
	public JldGetter(){
		if(USE_IMPORT_DIR)
			default_dir = IMPORT_DIR;
		else
			default_dir = System.getProperty("user.home")+"/";
	}public JldGetter(String parent_dir){
		default_dir = parent_dir;
	}
	
	/**
	 * Public access function
	 */ 
	public JLD getValues(String url, int timeout){
		return finalize(download(url, timeout));
	}/**
	*	Will use the default path
	*	Use getValues(File) to not use default path
	*/
	public JLD getValues(String filepath){
		return finalize(processStream(Thread.currentThread().getContextClassLoader().getResourceAsStream(default_dir+filepath)));
	}public JLD getValues(File file){
		return finalize(processStream(getFromFile(file)));
	}
	public JLD finalize(ArrayList<String> raw){
		raw = commentStripper(raw);
		raw = macroStripper(raw, getMacros(raw));
		String input = "";
		for(String s : raw)
			input += s;
		return new JLD(parseDoc(input));
	}
	
	private LinkedHashMap<String, String> getMacros(ArrayList<String> raw){
		LinkedHashMap<String, String> out = new LinkedHashMap<String, String>();
		for(String str : raw){
			str = str.trim();
			if(str.startsWith("{"))
				return out;
			if(str.startsWith("#define ")){
				str = str.replaceFirst("#define\\s", "");
				String key = str.substring(0, str.indexOf(' '));
				String val = str.replaceFirst(key+" ", "");
				out.put(key, val);
			}
		}
		System.out.println("ERR: No document found");
		return null;
	}
	
	/**
	 * Returns the raw downloaded data
	 */
	public ArrayList<String> download(String url, int port){
		try{
			open(url, port);
			BufferedReader buffreader = new BufferedReader(new InputStreamReader(connection.getInputStream(), "utf-8"));
			String temp;
			ArrayList<String> out = new ArrayList<String>();
			while((temp=buffreader.readLine()) != null){
				out.add(temp);
			}
			//System.out.println("downloaded: "+out);
			close();
			return out;
		}catch(IOException e){
			System.out.println("ERR: download failure "+e);
			return null;
		}
	}
	
	private void open(String url, int timeout){
		try{
			connection = (HttpURLConnection) new URL(url).openConnection();
			connection.setConnectTimeout(timeout);
			connection.setRequestProperty("User-Agent", "Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.11 (KHTML, like Gecko) Chrome/23.0.1271.95 Safari/537.11");
			connection.connect();
			if(connection.getResponseCode() != 200){
				System.out.println("ERR: open failure "+connection.getResponseCode());
				close();
			}
		}catch(IOException e){
			System.out.println("ERR: open failure "+e);
		}
	}
	
	/**
	 * Returns the raw data
	 */
	public ArrayList<String> processStream(InputStream is){
		BufferedReader buffreader = null;
		try{
			buffreader = new BufferedReader(new InputStreamReader(is, "utf-8"));
			String temp;
			ArrayList<String> out = new ArrayList<String>();
			while((temp=buffreader.readLine()) != null){
				out.add(temp);
			}
			buffreader.close();
			return out;
		}catch(IOException e){
			System.out.println("ERR: download failure "+e);
			return null;
		}catch(NullPointerException e){
			System.out.println("ERR: cannot access file");
			return null;
		}
	}
	
	public FileInputStream getFromFile(File file){
		try {
			return new FileInputStream(file);
		} catch (FileNotFoundException e) {
			e.printStackTrace();
		}
		return null;
	}
	
	private void close(){
		if(connection == null)
			return;
		connection.disconnect();
	}
	
//Removed to move to use jar friendly getResourceStream instead 
//	private File openFile(String url){
//		File f = null;
//		try {
//			f = new File(getClass().getResource(url).toURI());
//		}catch (NullPointerException e){
//			System.out.println("No such file "+url);
//			e.printStackTrace();
//			return null;
//		}catch (SecurityException e){
//			System.out.println("Security violation: cannot access "+url);
//			e.printStackTrace();
//			return null;
//		} catch (URISyntaxException e) {
//			System.out.println("Invalid URI "+url);
//			e.printStackTrace();
//			return null;
//		}
//		return f;
//	}
	
	private LinkedHashMap parseDoc(String raw){
		LinkedHashMap<String, Object> out = null; 
		try{
			if(raw.charAt(0) == '{'){ //document
				boolean nextIsVal = false;
				String key = null;
				out = new LinkedHashMap<String, Object>();
				for(raw=raw.substring(1).trim(); true; raw=raw.trim()){
					if(raw.charAt(0) == ':'){ //new value
						raw = raw.substring(1).trim();
						if(nextIsVal){
							Object val = null;
							if(raw.charAt(0) == '"' || raw.charAt(0) == '\''){
								String[] arr = parseString(raw);
								val = arr[0];
								raw = arr[1];
							}else if(raw.charAt(0) == '['){ //new list value
								val = parseList(stringBetween(raw, '[', ']'));
								raw = stringAfter(raw, '[', ']');
							}else if(raw.charAt(0) == '{'){ //new doc value
								val = parseDoc(stringBetweenInclusive(raw, '{', '}'));
								raw = stringAfter(raw, '{', '}');
							}else{
								throw new BadStringOperationException("Malformed JLD: invalid value");
							}
							raw = raw.substring(raw.indexOf(',')+1);
							out.put(key, val);
							nextIsVal = false;
						}else{
							throw new BadStringOperationException("Malformed JLD: parser expected a value after :");
						}
					}else if(raw.charAt(0) == '"' || raw.charAt(0) == '\''){
						if(!nextIsVal){ //new key
							String[] arr = parseString(raw);
							key = arr[0];
							raw = arr[1];
							nextIsVal = true;
						}else{
							throw new BadStringOperationException("Malformed JLD: parser expected a key");
						}
					}else if(raw.charAt(0) == '}'){
						if(!nextIsVal){
							break;
						}else{
							throw new BadStringOperationException("Malformed JLD: parser expected a value instead of }");
						}
					}else{
						throw new BadStringOperationException("Malformed JLD: unhandled case: "+raw+"\r\nDid you forget a comma?");
					}
				}
			}else{
				throw new BadStringOperationException("Malformed JLD: no document present");
			}
		}catch(Exception e){
			e.printStackTrace();
			return null;
		}
		return out;
	}
	
	/**
	 * Returns a map of values
	 * Cannot parse fields containing objects
	 * Example input: "Id","101010100","Name"
	 */
	private ArrayList parseList(String raw) throws IndexOutOfBoundsException{
		String orig = raw;
		ArrayList<Object> out = new ArrayList<Object>();
		while(raw.indexOf('"')>=0){
			if(raw.charAt(0) == '"' || raw.charAt(0) == '\''){
				String[] arr = parseString(raw);
				out.add(arr[0]);
				raw = arr[1];
			}else if(raw.charAt(0) == '['){
				out.add(parseList(stringBetween(raw, '[', ']')));
				raw = stringAfter(raw, '[', ']');
			}else if(raw.charAt(0) == '{'){
				out.add(parseDoc(stringBetweenInclusive(raw, '{', '}')));
				raw = stringAfter(raw, '{', '}');
			}else{
				break;
			}
			raw = raw.substring(raw.indexOf(',')+1).trim();
		}
		return out;
	}
	
	/**
	 * Returns an array where [parsedString, raw]
	 */
	private String[] parseString(String raw) throws IndexOutOfBoundsException{
		String out = "";
		raw = raw.trim();
		if(raw.charAt(0) == '"'){
			out += stringBetween(raw, '"');
			raw = stringAfter(raw, '"').trim();
		}else if(raw.charAt(0) == '\''){
			out += stringBetween(raw, '\'');
			raw = stringAfter(raw, '\'');
		}else{
			return new String[]{"",raw};
		}
		if(!raw.isEmpty() && raw.charAt(0) == '+'){
			String[] tmp = parseString(raw.substring(1).trim());
			out += tmp[0];
			raw = tmp[1];
		}
		return new String[]{out, raw};
	}
	
	/**
	 * returns a populated document
	 */
	private ArrayList<String> macroStripper(ArrayList<String> in, LinkedHashMap<String, String> macros){
		ArrayList<String> out = new ArrayList<String>();
		for(String str : in){
			if(str.trim().startsWith("#define "))
				continue;
			boolean nohit = true;
			String res = "";
			for(String key : macros.keySet()){
				if(!str.contains(key)){
					continue;
				}
				boolean foundAtStart = str.trim().startsWith(key);
				String[] arr = str.split(key);
				for(int i=0; i<arr.length; i++){
					arr[i] = arr[i].trim();
				}
				String tmpstr = "";
				if(foundAtStart && tmpstr.isEmpty() && arr[1].substring(0, 1).matches("[\\[\\]:,\\+{}]")){
					tmpstr += macros.get(key); nohit = false;}
				tmpstr += arr[0];
				for(int i=1; i<arr.length; i++){
					if(!arr[i-1].isEmpty() && arr[i-1].substring(arr[i-1].length()-1).matches("[\\[\\]:,\\+{}]")
							&& arr[i].substring(0, 1).matches("[\\[\\]:,\\+{}]")){
						tmpstr += macros.get(key); nohit = false;}
					tmpstr += arr[i];
				}
				res = str = tmpstr;
			}
			if(nohit)
				res = str;
			out.add(res);
		}
		return out;
	}
	
	/**
	 * Strips comments such as // and /*
	 * Used only by JLD offline documents
	 * Actual JSON strings do not contain comments...or lines
	 */
	private ArrayList<String> commentStripper(ArrayList<String> in){
		ArrayList<String> out = new ArrayList<String>();
		boolean blockquote = false;
		for(String s : in){
			for(int i=0; i<s.length()-1; i++){
				String st = s.substring(i, i+2);
				if(blockquote){
					if(st.equals("*/")){
						s = s.substring(i+2);
						blockquote = false;
						if(s.length()>=2)
							st = s.substring(0, 2);
					}
				}else if(st.equals("//")){
					s = s.substring(0, i);
				}else if(st.equals("/*")){
					blockquote = true;
				}
			}
			s = s.trim();
			if(blockquote || s.isEmpty())
				continue;
			else
				out.add(s);
		}
		return out;
	}
	
	/**
	 * No bounds checking
	 * No checking of any kind! (sort of)
	 */
	private String stringBetween(String raw, char c) throws IndexOutOfBoundsException{
		raw = raw.substring(raw.indexOf(c)+1);
		raw = raw.substring(0,raw.indexOf(c));
		return raw;
	}
	private String stringBetween(String raw, char a, char b) throws IndexOutOfBoundsException{
		String out = raw.substring(raw.indexOf(a)+1);
		int open = 1;
		int i;
		for(i=0; open>0; i++){
			if(out.charAt(i) == a){
				open++;
			}else if(out.charAt(i) == b){
				open--;
			}else if(out.charAt(i) == '"'){
				i++;
				while(out.charAt(i) != '"')
					i++;
			}
		}
		out = out.substring(0,i-1);
		return out;
	}
	private String stringBetweenInclusive(String raw, char a, char b) throws IndexOutOfBoundsException{
		return a+stringBetween(raw, a, b)+b;
	}
	private String stringAfter(String raw, char a, char b) throws IndexOutOfBoundsException{
		String sub = stringBetweenInclusive(raw, a, b);
		return raw.substring(sub.length());
	}
	private String stringAfter(String raw, char c) throws IndexOutOfBoundsException{
		int i = raw.indexOf(c);
		for(i++; raw.charAt(i)!=c; i++)
			;
		return raw.substring(i+1);
	}
}
