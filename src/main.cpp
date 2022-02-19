#include "Scanner.hpp"
#include <unordered_map>

/*
** Split string by delimeters into container containing it's tokens
** Inspired by https://stackoverflow.com/a/1493195/12792299 :)
*/
template <class ContainerT>
void split(const std::string& str, ContainerT& tokens, const std::string &delimiters)
{
	size_t pos, lastPos = 0, length = str.length();
	using value_type = typename ContainerT::value_type;
	using size_type  = typename ContainerT::size_type;

	while (lastPos < length + 1)
	{
		pos = str.find_first_of(delimiters, lastPos);
		if (pos == std::string::npos)
			pos = length;
		if (pos != lastPos)
			tokens.push_back(value_type(str.data() + lastPos, (size_type)pos - lastPos ));
		lastPos = pos + 1;
	}
}

/*
** Parse signature file
** First element of pair is hash map of extensions and their signature values one by one
** Second element of pair is just 2d array of strings, containing groups of extensions to output
**
** Example:
** Lines ".exe;.dll:CreateRemoteThread;CreateProcess" and ".cmd;.bat:rd /s /q "c:\windows"" will transform into:
**
** ret_map:
** <".exe", {"CreateRemoteThread", "CreateProcess"}>
** <".dll", {"CreateRemoteThread", "CreateProcess"}>
** <".cmd", {"rd /s /q "c:\windows""}>
** <".bat", {"rd /s /q "c:\windows""}>
** ret_extensions:
** {{".exe,.dll"}, {".cmd,.bat"}}
*/
std::pair<std::unordered_map<std::string, std::vector<std::string>>, std::vector<std::vector<std::string>>> form_signatures(std::string path)
{
	std::unordered_map<std::string, std::vector<std::string>> ret_map;
	std::vector<std::vector<std::string>> ret_extensions;
	std::ifstream sig_file;
	std::string line;

	sig_file.open(path);
	if (!sig_file.is_open())
	{
		std::cout << "Can't open signatures file!" << std::endl;
		exit(1);
	}

	while (std::getline(sig_file, line))
	{
		std::string left = line.substr(0, line.find(":"));
		std::vector<std::string> extensions;
		split(left, extensions, ";");

		std::string right = line.substr(left.length() + 1, line.length());
		std::vector<std::string> tokens;
		split(right, tokens, ";");

		ret_extensions.push_back(extensions);
		for (auto &ext : extensions)
			ret_map.emplace(ext, tokens);
	}
	sig_file.close();
	return (std::make_pair(ret_map, ret_extensions));
}

int main(int argc, char **argv)
{
	/*
	** Arguments validation
	*/
	if (argc > 4 || (argc == 2 && !strcmp(argv[1], "help")))
	{
		std::cout	<< "Usage: .\\scan_util [directory] [threads_count] [signatures_file]" << std::endl
					<< "No directory -> will scan current" << std::endl
					<< "No threads count -> will use 16 threads" << std::endl
					<< "No signature file -> will use signatures.txt" << std::endl;
		return (1);
	}

	std::string dir_path = (argc >= 2) ? argv[1] : ".";
	int threads = (argc >= 3) ? atoi(argv[2]) : 16;
	std::string sig_path = (argc == 4) ? argv[3] : "signatures.txt";
	if (!std::filesystem::is_directory(dir_path))
	{
		std::cout	<< "Wrong directory path!" << std::endl;
		return (1);
	}
	if (threads <= 0)
	{
		std::cout	<< "Wrong number of threads!" << std::endl;
		return (1);
	}
	if (!std::filesystem::is_regular_file(sig_path))
	{
		std::cout	<< "Wrong signature file path!" << std::endl;
		return (1);
	}

	/*
	** Main class creation and scanning launch
	*/
	Scanner _scanner(dir_path, threads, form_signatures(sig_path));
	_scanner.scan();
	return (0);
}
