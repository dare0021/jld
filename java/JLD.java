package jld;

import java.awt.Color;
import java.util.LinkedHashMap;
import java.util.Set;
import java.util.ArrayList;

/**
 * The in-code representation of a JLD
 * Because .find()
 * Values are stored using the .toString() method of each value
 */
public class JLD{
	private LinkedHashMap<String, Object> hm = null;
	public JLD(){hm = new LinkedHashMap<String, Object>();}
	public JLD(LinkedHashMap h){hm = h;}
	
	public Object get(String raw){
		return hm.get(raw);
	}public void put(String key, Object val){
		hm.put(key, val);
	}public void clear(){
		hm.clear();
	}public String toString(){
		return hm.toString();
	}public Set<String> keySet(){
		return hm.keySet();
	}
	
	public static Color getAsColor(ArrayList<String> arr, Color defaultTo){
		if(arr == null)
			return defaultTo;
		int[] rgba = new int[4];
		for(int i=0; i<4; i++){
			rgba[i] = Integer.parseInt(arr.get(i));
		}
		return new Color(rgba[0], rgba[1], rgba[2], rgba[3]);
	}
	
	public static double getAsDouble(String input, double defaultTo){
		double out = defaultTo;
		if(input != null)
			out = Double.parseDouble(input);
		return out;
	}
	
	public static int getAsInt(String input, int defaultTo){
		int out = defaultTo;
		if(input != null)
			out = Integer.parseInt(input);
		return out;
	}
	
	public static String parseControlChars(String input){
		String out = "";
		for(int i=0; i<input.length(); i++){
			char c = input.charAt(i);
			if(c != '\\'){
				out += c;
				continue;
			}//else
			i++;
			switch (input.charAt(i)){
			case 'r':
				break;
			case 'n':
				out += '\n';
				break;
			case 't':
				out += '\t';
				break;
			case '\\':
			default:
				out += '\\';
			}
		}
		return out;
	}
	
	/**
	 * Evaluates the given string to extract a map value
	 * e.g. eval(hm, "e.g") is equal to hm.get("e").get("g")
	 *      eval(hm, "e:2") is equal to ((List)(hm.get("e")))(2)
	 * Does not do error handling. 
	 * 	If the key does not exist, null is returned.
	 * 		Null return value does not always mean key DNE, it could also mean the value is actually null.
	 * 	If the index is out of bounds, an exception occurs.
	 */
	public Object find(String raw){
		Object ot = hm;
		int i = 0;
		for(String st=""; i<raw.length(); i++){ //find first key
			if(raw.charAt(i) == '.' || raw.charAt(i) == ':' || i+2>raw.length()){ //key
				if(i+2>raw.length())
					st += raw.charAt(i);
				ot = ((LinkedHashMap)ot).get(st);
				break;
			}else{
				st += raw.charAt(i);
			}
		}
		boolean isKey = false; //if false, is Vector element
		for(String st=""; i<raw.length(); i++){
			if((!st.isEmpty() && (raw.charAt(i)=='.' || raw.charAt(i)==':')) || i+2>raw.length()){
				if(i+2>raw.length())
					st += raw.charAt(i);
				if(isKey){
					ot = ((LinkedHashMap)ot).get(st);
					st = "";
				}else{
					ot = ((ArrayList)ot).get(Integer.parseInt(st));
					st = "";
				}
			}
			if(raw.charAt(i) == '.'){
				isKey = true;
			}else if(raw.charAt(i) == ':'){
				isKey = false;
			}else{
				st += raw.charAt(i);
			}
		}
		return ot; 
	}
}
