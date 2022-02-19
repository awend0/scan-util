#include "Scanner.hpp"

/*
** Constructor, fills up "std::multiset _files" with all files to be proceeded in directory
*/
Scanner::Scanner(std::string path, size_t threads_count, std::pair<std::unordered_map<std::string, std::vector<std::string>>, std::vector<std::vector<std::string>>> sig_pair)
	: _path(path), _threads_count(threads_count), _signatures(sig_pair.first), _extensions(sig_pair.second)
{
    this->_start_timepoint = std::chrono::high_resolution_clock::now();
    for (auto &file : std::filesystem::directory_iterator(this->_path))
	{
		auto ext = file.path().extension().generic_string();
		if (file.is_regular_file() && this->_signatures.find(ext) != this->_signatures.end())
			this->_files.insert(std::make_shared<std::filesystem::directory_entry>(file));
	}
	this->_processed = 0; this->_errors = 0;
}

/*
** Launch and join all threads
*/
void Scanner::scan(void)
{
    std::vector<std::thread> _threads(this->_threads_count);

    for (size_t i = 0; i < this->_threads_count; i++)
		_threads[i] = std::thread(&Scanner::routine, this);
	for (size_t i = 0; i < this->_threads_count; i++)
		_threads[i].join();

    print_result();
}

/*
** Threads routine. "Pops" first element of std::multiset _files, pass it to check_file method
*/
void Scanner::routine(void)
{
	while (1)
	{
		this->multiset_pop_mutex.lock();
		if (this->_files.empty())
		{
			this->multiset_pop_mutex.unlock();
			break;
		}
		auto file = std::filesystem::directory_entry(**this->_files.begin());
		this->_files.erase(this->_files.begin());
		this->multiset_pop_mutex.unlock();
		check_file(file.path().generic_wstring(), file.path().extension().generic_string());
	}
}

/*
** Just read file line-by-line, search suspicous word in these lines using std::string::find and increment corresponding counter
*/
void Scanner::check_file(std::wstring filename, std::string ext)
{
	std::vector<std::string> tofind = this->_signatures[ext];
	std::string line;
	std::ifstream file;

	this->_processed++;
	if (ext == ".exe" || ext == ".dll")
		file.open(filename.c_str(), std::ios::binary);
	else
		file.open(filename.c_str());
	if (!file.is_open())
	{
		this->_errors++;
		return;
	}

	while (std::getline(file, line))
	{
		for (size_t i = 0; i < tofind.size(); i++)
		{
			if (line.find(tofind[i], 0) != std::string::npos)
			{
				file.close();
				this->counter_inc_mutex.lock();
				this->_detects[ext]++;
				this->counter_inc_mutex.unlock();
			}
		}
	}
	file.close();
}

static inline std::string str_toupper(std::string str)
{
	std::string ret;
	for (size_t i = 0; i < str.length(); i++)
		ret += toupper(str[i]);
	return (ret);
}

/*
** Just printing the results. Calling set_timestamp before slow std::cout
*/
void Scanner::print_result(void)
{
	set_timestamp();
	std::cout	<< "====== Scan result ======" << std::endl
				<< "Processed files: " << this->_processed << std::endl;

	for (auto &it : this->_extensions)
	{
		int total_detects_in_group = 0;
		for (auto &ext : it)
			total_detects_in_group += this->_detects[ext];
		std::cout << str_toupper(it[0].substr(1, it[0].length())) << " detects: " << total_detects_in_group << std::endl;
	}

	std::cout	<< "Errors: " << this->_errors << std::endl
				<< "Execution time: " << this->_timestamp << " on " << this->_threads_count << " thread(s)" << std::endl
				<< "=========================" << std::endl;
}

/*
** Perhaps this one could be much shorter
*/
void Scanner::set_timestamp(void)
{
	std::stringstream to_set;

	auto elapsed_total = std::chrono::high_resolution_clock::now() - this->_start_timepoint;
	auto elapsed_hours = std::chrono::duration_cast<std::chrono::hours>(elapsed_total);
	auto elapsed_minutes = std::chrono::duration_cast<std::chrono::minutes>(elapsed_total - elapsed_hours);
	auto elapsed_seconds = std::chrono::duration_cast<std::chrono::seconds>(elapsed_total - elapsed_hours - elapsed_minutes);
	auto elapsed_milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed_total - elapsed_hours - elapsed_minutes - elapsed_seconds);

	to_set << IOMANIP_MOD elapsed_hours.count() << ":" << IOMANIP_MOD elapsed_minutes.count() << ":" << IOMANIP_MOD elapsed_seconds.count() << "." << IOMANIP_MOD elapsed_milliseconds.count();
	this->_timestamp = to_set.str();
}
