package juli.builder;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Properties;

import org.apache.commons.cli.CommandLine;
import org.apache.commons.cli.CommandLineParser;
import org.apache.commons.cli.GnuParser;
import org.apache.commons.cli.HelpFormatter;
import org.apache.commons.cli.Options;
import org.apache.commons.cli.ParseException;

public class Builder {
	
	private static final String CONFIGURATION_FILE = "configuration.properties";
	private static final String KEY_COMPILER = "compiler";
	private static final String KEY_LINKER = "linker";

	static void redirect(InputStream in, OutputStream out) throws IOException {
		while (true) {
			int c = in.read();
			if (c == -1)
				break;
			out.write((char) c);
		}
	}

	private Collection<File> objects;
	private File output;
	private File buildDir;
	
	Properties configuration;

	private File getOutputFile(File filePath) throws IOException {
		String fileBaseName = filePath.getName();
		fileBaseName = fileBaseName.substring(0, fileBaseName.lastIndexOf('.'));
		return new File(buildDir, fileBaseName + ".o");
	}

	private int executeCommand(String command, File dir) throws IOException, InterruptedException {
		Process p = Runtime.getRuntime().exec(command, null, dir);
		redirect(p.getInputStream(), System.out);
		redirect(p.getErrorStream(), System.err);
		return p.waitFor();
	}

	public void compile(File file) throws IOException, InterruptedException, BuildException {
		if (file.isDirectory()) {
			System.out.println("Entering " + file);
			for (File f : file.listFiles(new SourceFilenameFilter())) {
				compile(f);
			}
		} else {
			File objectFile = getOutputFile(file);
			String command = configuration.getProperty(KEY_COMPILER) + " " + file.getCanonicalPath() + " -o "
					+ objectFile.getCanonicalPath();
			System.out.println(command);
			int res = executeCommand(command, file.getParentFile());
			if (res > 0)
				throw new BuildException("Compilation failed.");
			objects.add(objectFile);
		}
	}

	public void link() throws IOException, InterruptedException {
		StringBuilder command = new StringBuilder();
		command.append(configuration.getProperty(KEY_LINKER));
		command.append(" -o ");
		command.append(output.toString());
		for (File f : objects) {
			command.append(" ");
			command.append(f.getCanonicalPath());
		}
		System.out.println(command);
		executeCommand(command.toString(), null);
	}

	public void removeObjects() {
		for (File f : objects) {
			System.out.format("Removing %s ...\n", f);
			f.delete();
		}
	}

	public Builder(File output, File buildDir) throws FileNotFoundException, IOException {
		objects = new ArrayList<File>();
		this.output = output;
		this.buildDir = buildDir;
		
		this.buildDir.mkdirs();
		
		configuration = new Properties();
		configuration.load(new FileReader(CONFIGURATION_FILE));
	}

	public static void main(String[] args) throws IOException, InterruptedException, ParseException, BuildException {
		Options options = new Options();
		options.addOption("o", "output", true, "output file name");
		options.addOption("h", "help", false, "print this message");
		options.addOption("b", "build-dir", true, "build directory");
		
		CommandLineParser cliParser = new GnuParser();
		CommandLine commandLine = cliParser.parse(options, args);
		
		HelpFormatter formatter = new HelpFormatter();
		String helpString = "java -cp bin/:lib/commons-cli-1.2.jar juli.builder.Builder <options> <input-files>";
		
		if (commandLine.hasOption('h')) {
			formatter.printHelp(helpString , options );
			return;
		}
		
		File buildDir = new File(".");
		if (commandLine.hasOption('b')) {
			buildDir = new File(commandLine.getOptionValue('b'));
		}
		
		File output = new File(buildDir, "a.out");
		if (commandLine.hasOption('o')) {
			output = new File(buildDir, commandLine.getOptionValue('o'));
		}
		
		if (commandLine.getArgList().isEmpty()) {
			System.err.println("No input file provided.");
			formatter.printHelp(helpString, options );
			return;
		}

		Builder b = new Builder(output, buildDir);
		for (String s : commandLine.getArgs()) {
			b.compile(new File(s));
		}
		b.link();
		
		//b.removeObjects();
	}
}
