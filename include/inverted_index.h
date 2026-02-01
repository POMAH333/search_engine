#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include <thread>
#include <sstream>
#include <map>
#include <mutex>

using namespace std;

struct Entry
{
    size_t doc_id, count;
    // Данный оператор необходим для проведения тестовыхсценариев
    bool operator==(const Entry &other) const
    {
        return (doc_id == other.doc_id && count == other.count);
    }
};

class InvertedIndex
{
public:
    InvertedIndex() = default;
    /**
     * Обновить или заполнить базу документов, по которой будем совершать поиск
     * @param texts_input содержимое документов
     */
    void updateDocumentBase(std::vector<std::string> input_docs);
    /**
     * Метод определяет количество вхождений слова word в загруженной базе документов
     * @param word слово, частоту вхождений которого необходимо определить
     * @return возвращает подготовленный список с частотой слов
     */
    std::vector<Entry> getWordCount(const std::string &word);

private:
    std::vector<std::string> docs; // список содержимого документов

    std::map<std::string, std::vector<Entry>> freq_dictionary; // частотный словарь
    mutex freq_dict_access;
};
