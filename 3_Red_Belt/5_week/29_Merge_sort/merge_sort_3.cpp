#include "test_runner.h"
#include <algorithm>
#include <memory>
#include <vector>

template <typename RandomIt>
void MergeSort(RandomIt range_begin, RandomIt range_end) {
    // 1. Если диапазон содержит меньше 2 элементов, выходим из функции
    int range_length = range_end - range_begin;
    if (range_length < 2) {
      return;
    }

    // 2. Создаем вектор, содержащий все элементы текущего диапазона
    std::vector<typename RandomIt::value_type> elements(std::make_move_iterator(range_begin),
                                                        std::make_move_iterator(range_end));

    // 3. Разбиваем вектор на три равные части
    auto one_third = elements.begin() + range_length / 3;
    auto two_third = elements.begin() + range_length * 2 / 3;

    // 4. Вызываем функцию MergeSort от каждой трети вектора
    MergeSort(elements.begin(), one_third);
    MergeSort(one_third, two_third);
    MergeSort(two_third, elements.end());

    // 5. С помощью алгоритма merge cливаем первые две трети во временный вектор
    std::vector<typename RandomIt::value_type> interim_result;
    merge(std::make_move_iterator(elements.begin()), std::make_move_iterator(one_third),
          std::make_move_iterator(one_third), std::make_move_iterator(two_third),
          back_inserter(interim_result));

    // 6. С помощью алгоритма merge сливаем отсортированные части
    // в исходный диапазон
    merge(std::make_move_iterator(interim_result.begin()), std::make_move_iterator(interim_result.end()),
          std::make_move_iterator(two_third), std::make_move_iterator(elements.end()),
          range_begin);
}

void TestIntVector() {
    std::vector<int> numbers = {6, 1, 3, 9, 1, 9, 8, 12, 1};
    MergeSort(begin(numbers), end(numbers));
    ASSERT(is_sorted(begin(numbers), end(numbers)));
}


int main() {
    TestRunner tr;
    RUN_TEST(tr, TestIntVector);
    return 0;
}

/* author's solution
#include <algorithm>
#include <memory>
#include <vector>

using namespace std;

template <typename RandomIt>
void MergeSort(RandomIt range_begin, RandomIt range_end) {
  // 1. Если диапазон содержит меньше 2 элементов, выходим из функции
  const int range_length = range_end - range_begin;
  if (range_length < 2) {
    return;
  }

  // 2. Создаем вектор, содержащий все элементы текущего диапазона
  vector<typename RandomIt::value_type> elements(
      make_move_iterator(range_begin), make_move_iterator(range_end));

  // 3. Разбиваем вектор на три равные части
  auto one_third = begin(elements) + range_length / 3;
  auto two_third = begin(elements) + range_length * 2 / 3;

  // 4. Вызываем функцию MergeSort от каждой трети вектора
  MergeSort(begin(elements), one_third);
  MergeSort(one_third, two_third);
  MergeSort(two_third, end(elements));

  // 5. С помощью алгоритма merge cливаем первые две трети во временный вектор
  // back_inserter -> http://ru.cppreference.com/w/cpp/iterator/back_inserter
  vector<typename RandomIt::value_type> interim_result;
  merge(make_move_iterator(begin(elements)), make_move_iterator(one_third),
        make_move_iterator(one_third), make_move_iterator(two_third),
        back_inserter(interim_result));

  // 6. С помощью алгоритма merge сливаем отсортированные части
  // в исходный диапазон
  // merge -> http://ru.cppreference.com/w/cpp/algorithm/merge
  merge(make_move_iterator(begin(interim_result)),
        make_move_iterator(end(interim_result)),
        make_move_iterator(two_third),
        make_move_iterator(end(elements)),
        range_begin);
}
 */
