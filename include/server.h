#pragma once

#include <string>
#include <vector>
#include <unordered_set>

#include "inverted_index.h"

struct RelativeIndex
{
    size_t doc_id;
    float rank;

    bool operator==(const RelativeIndex &other) const
    {
        return (doc_id == other.doc_id && rank == other.rank);
    }
};

class SearchServer
{
public:
    /**
     * @param idx В конструктор класса передаётся ссылка на класс InvertedIndex,
     * чтобы SearchServer мог узнать частоту слов, встречаемых в запросе.
     */
    SearchServer(InvertedIndex &idx) : _index(idx) {};

    /**
     * Метод обработки поисковых запросов.
     * @param queries_input Поисковые запросы, взятые из файла requests.json.
     * @return Возвращает отсортированный список релевантных ответов для заданных запросов.
     */
    std::vector<std::vector<RelativeIndex>> search(const std::vector<std::string> &queries_input);

private:
    InvertedIndex &_index;

    int respLimit = 5;

public:
    void setRespLimit(int _responseLimit)
    {
        if (_responseLimit > 0)
            respLimit = _responseLimit;
    }
};