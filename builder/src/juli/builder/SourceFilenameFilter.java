package juli.builder;

import java.io.File;
import java.io.FilenameFilter;

public class SourceFilenameFilter implements FilenameFilter {
	
	public String getExtension(String name) {
		return name.substring(name.lastIndexOf("."));
	}

	@Override
	public boolean accept(File dir, String name) {
		return getExtension(name).equals(".jl");
	}

}
