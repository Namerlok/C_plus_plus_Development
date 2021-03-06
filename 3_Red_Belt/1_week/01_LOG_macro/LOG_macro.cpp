//============================================================================
// Name        : LOG_macro
// Author      : Namerlok
// Version     :
// Copyright   : Your copyright notice
// Description : Ansi-style
//============================================================================

#include "test_runner.h"
#include <sstream>
#include <string>
using namespace std;

class Logger {
public:
	explicit Logger(ostream& output_stream) : os(output_stream) {}

	void SetLogLine(bool value) { log_line = value; }
	void SetLogFile(bool value) { log_file= value; }

	bool GiveLogLine() const { return log_line; }
	bool GiveLogFile() const { return log_file; }

	void Log(const string& message) { os << message; }

private:
	ostream& os;
	bool log_line = false;
	bool log_file = false;
};

#define LOG(logger, message) {                                      \
	if (logger.GiveLogLine() && logger.GiveLogFile()) {             \
		logger.Log(__FILE__);                                       \
		logger.Log(":");                                            \
		logger.Log(to_string(__LINE__));                            \
		logger.Log(" ");                                            \
	} else if (!logger.GiveLogLine() && logger.GiveLogFile()) {     \
		logger.Log(__FILE__);                                       \
		logger.Log(" ");                                            \
	} else if (logger.GiveLogLine() && !logger.GiveLogFile()) {     \
		logger.Log("Line ");                                        \
		logger.Log(to_string(__LINE__));                            \
		logger.Log(" ");                                            \
	}                                                               \
	logger.Log(message);                                            \
	logger.Log("\n");                                               \
}

void TestLog() {
/* Для написания юнит-тестов в этой задаче нам нужно фиксировать конкретные
 * номера строк в ожидаемом значении (см. переменную expected ниже). Если
 * мы добавляем какой-то код выше функции TestLog, то эти номера строк меняются,
 * и наш тест начинает падать. Это неудобно.
 *
 * Чтобы этого избежать, мы используем специальный макрос #line
 * (http://en.cppreference.com/w/cpp/preprocessor/line), который позволяет
 * переопределить номер строки, а также имя файла. Благодаря ему, номера
 * строк внутри функции TestLog будут фиксированы независимо от того, какой
 * код мы добавляем перед ней*/
	#line 1 "logger.cpp"

	ostringstream logs;
	Logger l(logs);
	LOG(l, "hello");

	l.SetLogFile(true);
	LOG(l, "hello");

	l.SetLogLine(true);
	LOG(l, "hello");

	l.SetLogFile(false);
	LOG(l, "hello");

	string expected = "hello\n";
	expected += "logger.cpp hello\n";
	expected += "logger.cpp:10 hello\n";
	expected += "Line 13 hello\n";
	ASSERT_EQUAL(logs.str(), expected);
}

int main() {
	TestRunner tr;
	RUN_TEST(tr, TestLog);
}

/* авторское решение
#include <string>
#include <ostream>
using namespace std;

class Logger {
public:
  explicit Logger(ostream& output_stream) : os(output_stream) {
  }

  void SetLogLine(bool value) { log_line = value; }
  void SetLogFile(bool value) { log_file= value; }

  void Log(const string& message);

  void SetFile(const string& filename) {
    file = filename;
  }

  void SetLine(int line_number) {
    line = line_number;
  }

private:
  ostream& os;
  bool log_line = false;
  bool log_file = false;
  string file;
  int line;
};

void Logger::Log(const string& message) {
  if (log_file && log_line) {
    os << file << ':' << line << ' ';
  } else if (log_file) {
    os << file << ' ';
  } else if (log_line) {
    os << "Line " << line << ' ';
  }
  os << message << '\n';
}

#define LOG(logger, message) \
  logger.SetFile(__FILE__);  \
  logger.SetLine(__LINE__);  \
  logger.Log(message);
 */

