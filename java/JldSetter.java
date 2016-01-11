package jld;

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStreamWriter;
import java.net.URISyntaxException;
import java.util.LinkedHashMap;
import java.util.ArrayList;

/**
 * Saves a LinkedHashMap as a JLD file
 */
public class JldSetter implements IJLDGlobalFinals {
	private final int spacePerTab = 3;
	public final String default_dir;
	public JldSetter(){
		if(USE_EXPORT_DIR)
			default_dir = EXPORT_DIR;
		else
			default_dir = System.getProperty("user.home")+"/";
	}public JldSetter(String parent_dir){
		default_dir = parent_dir;
	}
	
	/**
	 * Public setter for the class
	 * Uses the default path
	 */
	public void setValues(String url, JLD raw){
		saveFile(url, parseMap(raw, 0));
	}public void setValues(File file, JLD raw){
		file.getParentFile().mkdirs();
		saveFile(file, parseMap(raw, 0));
	}
	
	/**
	 * Parses the given map in to human-readable strings
	 */
	private ArrayList<String> parseMap(JLD raw, int layer){
		ArrayList<String> out = new ArrayList<String>();
		String st;
		out.add(indent(layer) + "{");
		layer++;
		for(String key : raw.keySet()){
			st = indent(layer);
			if(key.contains("\""))
				st += "'" + key + "' : ";
			else
				st += "\"" + key + "\" : ";
			Object val = raw.get(key);
			if(val instanceof ArrayList){
				st += parseArrayList((ArrayList)val, layer+1);
				st += ",";
			}else if(val instanceof LinkedHashMap){
				out.add(st);
				JLD jldt = new JLD((LinkedHashMap)val);
				ArrayList<String> nested = parseMap(jldt, layer+1);
				st = "";
				for(String s : nested){
					st += s;
				}
				out.add(st);
				st = out.get(out.size()-1) + ",";
				out.remove(out.size()-1);
			}else if(val.toString().contains("\"")){
				st += "'" + val + "',";
			}else{
				st += "\"" + val + "\",";
			}
			out.add(st);
		}
		out.set(out.size()-1, out.get(out.size()-1).substring(0, out.get(out.size()-1).length()-1));
		for(int i=0; i<out.size(); i++){
			out.set(i,out.get(i)+"\r\n");
		}
		layer--;
		out.add(indent(layer)+"}");
		return out;
	}
	
	private String parseArrayList(ArrayList raw, int layer){
		String out = "[";
		boolean first = true;
		for(Object val : raw){
			if(val instanceof ArrayList){
				if(!first)
					out += ", ";
				else
					first = false;
				out += parseArrayList((ArrayList)val, layer+1);
			}else if(val instanceof LinkedHashMap){
				if(!first)
					out += ",";
				else
					first = false;
				out += "\r\n";
				JLD jldt = new JLD((LinkedHashMap)val);
				ArrayList<String> nested = parseMap(jldt, layer+1);
				for(String s : nested){
					out += s;
				}
			}else{
				if(!first)
					out += ", ";
				else
					first = false;
				if(val.toString().contains("\""))
					out += "'" + val + "'";
				else
					out += "\"" + val + "\"";
			}
		}
		return out+"]";
	}
	
	private void saveFile(String url, ArrayList<String> raw){
		File f = null;
		f = new File(default_dir+url);
		try {
			while(!f.createNewFile()){ //if already present
				f.delete();
				f.mkdirs();
				System.out.println("File "+url+" already present. Attempting deletion.");
			}
		} catch (IOException e) {
			System.out.println("Failed to create file(1, IOException): "+url);
			//e.printStackTrace();
		}
		saveFile(f, raw);
	}private void saveFile(File file, ArrayList<String> raw){
		BufferedWriter bw = null;
		try {
			bw = new BufferedWriter(new OutputStreamWriter(new FileOutputStream(file)));
		} catch (FileNotFoundException e) {
			System.out.println("Failed to create file(2): "+file.getAbsolutePath());
			//e.printStackTrace();
		}
		for(String s : raw){
			try {
				bw.write(s);
			} catch (IOException e) {
				System.out.println("Failed to write: "+s);
				System.out.println("to file "+file.getAbsolutePath());
				//e.printStackTrace();
			}
		}
		try {
			bw.close();
		} catch (IOException e) {
			System.out.println("Failed to close file: "+file.getAbsolutePath());
			//e.printStackTrace();
		}
		System.out.println("SAVED TO: "+file.getAbsolutePath());
	}
	
	private String indent(int layer){
		String out = "";
		for(int i=0; i<layer; i++){
			for(int j=0; j<spacePerTab; j++){
				out += " ";
			}
		}
		return out;
	}
}
