#include "test_runner.h"
#include "profile.h"

#include <sstream>
#include <iostream>
#include <map>
#include <string>
#include <set>
#include <utility>
#include <functional>
#include <future>

const auto NUMBER_OF_THREADS = std::thread::hardware_concurrency();

struct Stats {
    std::map<std::string, int> word_frequences;

    void operator+= (const Stats& other) {
        for (const auto& p: other.word_frequences)
            word_frequences[p.first] += p.second;
    }

    void operator+= (Stats&& other) {
        for (const auto& p: other.word_frequences)
            word_frequences[std::move(p.first)] += std::move(p.second);
    }
};

template <typename IT>
void SkipSpace(IT& it) {
    while (std::isspace(*it))
        ++it;
}

template <typename IT>
void SkipChar(IT& it) {
    while (!std::isspace(*it))
        ++it;
}

/*
Stats ExploreLine(const std::set<std::string>& key_words, const std::string& line) {
    Stats result;
    for (auto word_beg = line.begin(); word_beg != line.end();) {
        SkipSpace(word_beg);
        auto word_end = word_beg;
        while (!std::isspace(*word_end) && word_end != line.end())
            ++word_end;
        //std::cerr << std::string(word_beg, word_end) << "\n";
        if (key_words.count(std::string(word_beg, word_end)))
            result.IncreaseCounter(std::string(word_beg, word_end));
        word_beg = word_end;
        SkipSpace(word_beg);
    }
    return result;
}
*/

Stats ExploreLine(const std::set<std::string>& key_words, const std::string& line) {
    std::string_view line_view = line;
    Stats result;

    size_t pos = line_view.find_first_not_of(' ');
    line_view.remove_prefix(pos);

    while (pos != line_view.npos) {
        pos = line_view.find_first_of(' ');

        auto key = std::string(line_view.substr(0, pos));
        if (key_words.count(key) > 0) {
            ++result.word_frequences[std::move(key)];
        }

        if (pos == line_view.npos)
            break;

        line_view.remove_prefix(pos);
        pos = line_view.find_first_not_of(' ');
        line_view.remove_prefix(pos);
    }

    return result;
}

/*
Stats ExploreLines(const std::set<std::string>& key_words, std::vector<std::string> lines) {
    Stats result;
    for (auto& line: lines)
        result += ExploreLine(key_words, line);
    return result;
}
*/

Stats ExploreKeyWordsSingleThread(const std::set<std::string>& key_words, std::istream& input) {
    Stats result;
    for (std::string line; std::getline(input, line); ) {
        result += ExploreLine(key_words, line);
    }
    return result;
}

/*
Stats ExploreKeyWords(const std::set<std::string>& key_words, std::istream& input) {
    Stats result;
    std::vector<std::future<Stats>> futures;

    while (!input.eof()) {
        std::vector<std::string> lines;
        size_t i = 0;
        for (std::string line; i < MAX_PAGE_SIZE && std::getline(input, line); ++i)
            lines.push_back(line);
//        std::cerr << lines << "\n";
        //futures.push_back(std::async([&key_words, &lines] {return ExploreLines(key_words, std::move(lines));}));
        futures.push_back(std::async(ExploreLines, std::ref(key_words), std::move(lines)));
    }

    for (auto& future: futures) {
        result += future.get();
    }
    return result;
}
*/

Stats ExploreKeyWords(const std::set<std::string>& key_words, std::istream& input) {
    Stats result;
    std::vector<std::future<Stats>> futures;
    std::vector<std::stringstream> streams(NUMBER_OF_THREADS);
    std::string line;
    size_t number_of_lines = 0;

    while (std::getline(input, line)) {
        streams[number_of_lines % NUMBER_OF_THREADS] << line << '\n';
        ++number_of_lines;
    }

    for (auto& s: streams)
        futures.push_back(std::async(ExploreKeyWordsSingleThread, std::ref(key_words), std::ref(s)));

    for (auto& future: futures)
        result += future.get();

    return result;
}

void TestBasicMulti() {
    const std::set<std::string> key_words = {"yangle", "rocks", "sucks", "all"};

    std::stringstream ss;
    ss << "this new yangle service really rocks\n";
    ss << "It sucks when yangle isn't available\n";
    ss << "10 reasons why yangle is the best IT company\n";
    ss << "yangle rocks others suck\n";
    ss << "Goondex really sucks, but yangle rocks. Use yangle\n";

    const auto stats = ExploreKeyWords(key_words, ss);
    const std::map<std::string, int> expected = {
        {"yangle", 6},
        {"rocks", 2},
        {"sucks", 1}
    };
    ASSERT_EQUAL(stats.word_frequences, expected);
}

void TestBasicSingle() {
    const std::set<std::string> key_words = {"yangle", "rocks", "sucks", "all"};

    std::stringstream ss;
    ss << "this new yangle service really rocks\n";
    ss << "It sucks when yangle isn't available\n";
    ss << "10 reasons why yangle is the best IT company\n";
    ss << "yangle rocks others suck\n";
    ss << "Goondex really sucks, but yangle rocks. Use yangle\n";

    const auto stats = ExploreKeyWordsSingleThread(key_words, ss);
    const std::map<std::string, int> expected = {
        {"yangle", 6},
        {"rocks", 2},
        {"sucks", 1}
    };
    ASSERT_EQUAL(stats.word_frequences, expected);
}


int main() {
    LOG_DURATION("Total");
    TestRunner tr;
    RUN_TEST(tr, TestBasicSingle);
    RUN_TEST(tr, TestBasicMulti);
}

/* author's solution
#include "test_runner.h"
#include "profile.h"

#include <algorithm>
#include <map>
#include <fstream>
#include <random>
#include <vector>
#include <string>
#include <sstream>
#include <future>
#include <iterator>
#include <set>
using namespace std;

struct Stats {
  map<string, int> word_frequences;

  void operator += (const Stats& other) {
    for (auto& [word, frequency] : other.word_frequences) {
      word_frequences[word] += frequency;
    }
  }
};

set<string> ReadKeyWords(istream& is) {
  return {istream_iterator<string>(is), istream_iterator<string>()};
}

vector<string> Split(const string& line) {
  // http://en.cppreference.com/w/cpp/iterator/istream_iterator
  istringstream line_splitter(line);
  return {istream_iterator<string>(line_splitter), istream_iterator<string>()};
}

Stats ExploreLine(const set<string>& key_words, const string& line) {
  Stats result;
  for (const string& word : Split(line)) {
    if (key_words.count(word) > 0) {
      result.word_frequences[word]++;
    }
  }
  return result;
}

Stats ExploreKeyWordsSingleThread(
  const set<string>& key_words, istream& input
) {
  Stats result;
  for (string line; getline(input, line); ) {
    result += ExploreLine(key_words, line);
  }
  return result;
}

Stats ExploreBatch(const set<string>& key_words, vector<string> lines) {
  Stats result;
  for (const string& line : lines) {
    result += ExploreLine(key_words, line);
  }
  return result;
}

Stats ExploreKeyWords(const set<string>& key_words, istream& input) {
  const size_t max_batch_size = 5000;

  vector<string> batch;
  batch.reserve(max_batch_size);

  vector<future<Stats>> futures;

  for (string line; getline(input, line); ) {
    batch.push_back(move(line));
    if (batch.size() >= max_batch_size) {
      futures.push_back(
        async(ExploreBatch, ref(key_words), move(batch))
      );
      batch.reserve(max_batch_size);
    }
  }

  Stats result;

  if (!batch.empty()) {
    result += ExploreBatch(key_words, move(batch));
  }

  for (auto& f : futures) {
    result += f.get();
  }

  return result;
}

void TestSplit() {
  const vector<string> expected1 = {
    "abc", "def", "ghi,", "!", "jklmnop-qrs,", "tuv"
  };
  ASSERT_EQUAL(Split("  abc def ghi, !  jklmnop-qrs, tuv"), expected1);

  const vector<string> expected2 = {"a", "b"};
  ASSERT_EQUAL(Split("a b      "), expected2);
  ASSERT_EQUAL(Split(""), vector<string>());
}

void TestBasic() {
  const set<string> key_words = {"yangle", "rocks", "sucks", "all"};

  stringstream ss;
  ss << "this new yangle service really rocks\n";
  ss << "It sucks when yangle isn't available\n";
  ss << "10 reasons why yangle is the best IT company\n";
  ss << "yangle rocks others suck\n";
  ss << "Goondex really sucks, but yangle rocks. Use yangle\n";

  const auto stats = ExploreKeyWords(key_words, ss);
  const map<string, int> expected = {
    {"yangle", 6},
    {"rocks", 2},
    {"sucks", 1}
  };
  ASSERT_EQUAL(stats.word_frequences, expected);
}

void TestMtAgainstSt() {
//  {
//    std::default_random_engine rd(34);
//    std::uniform_int_distribution<char> char_gen('a', 'z');
//
//    auto random_word = [&](size_t len) {
//      string result(len, ' ');
//      std::generate(begin(result), end(result), [&] { return char_gen(rd); });
//      return result;
//    };
//
//    std::uniform_int_distribution<size_t> len_gen(5, 21);
//
//    vector<string> key_words(500);
//    for (auto& w : key_words) {
//      w = random_word(len_gen(rd));
//    }
//
//    ofstream out("key_words.txt");
//    for (const auto& w : key_words) {
//      out << w << ' ';
//    }
//
//    std::uniform_int_distribution<size_t> line_len_gen(5, 100);
//    std::uniform_int_distribution<int> word_choice(1, 100);
//
//    ofstream text_out("text.txt");
//    for (int line = 0; line < 25000; ++line) {
//      ostringstream line_out;
//      auto line_len = line_len_gen(rd);
//      for (size_t i = 0; i < line_len; ++i) {
//        if (word_choice(rd) < 20) {
//          std::uniform_int_distribution<size_t> word_index(0, key_words.size() - 1);
//          line_out << key_words[word_index(rd)];
//        } else {
//          line_out << random_word(len_gen(rd));
//        }
//        line_out << ' ';
//      }
//      text_out << line_out.str() << '\n';
//    }
//  }

  ifstream key_words_input("key_words.txt");
  const auto key_words_data = ReadKeyWords(key_words_input);
  const set<string> key_words(key_words_data.begin(), key_words_data.end());

  Stats st_stats, mt_stats;
  {
    ifstream text_input("text.txt");
    LOG_DURATION("Single thread");
    st_stats = ExploreKeyWordsSingleThread(key_words, text_input);
  }
  {
    ifstream text_input("text.txt");
    LOG_DURATION("Multi thread");
    mt_stats = ExploreKeyWords(key_words, text_input);
  }

  ASSERT_EQUAL(st_stats.word_frequences, mt_stats.word_frequences);
}

int main() {
  TestRunner tr;
  RUN_TEST(tr, TestSplit);
  RUN_TEST(tr, TestBasic);
  RUN_TEST(tr, TestMtAgainstSt);
}
*/
