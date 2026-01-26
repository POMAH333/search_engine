#include <iostream>
#include <string>
#include <vector>
#include <utility>
#include <fstream>
#include <exception>
#include <thread>
#include <mutex>
#include <sstream>

#include "nlohmann/json.hpp"
#include "gtest/gtest.h"

using namespace std;
using namespace nlohmann;

/**
 *Класс для работы с JSON-файлами
 */
class ConverterJSON
{
    string pathConfig = "config.json";
    string pathRequests = "requests.json";
    string pathAnswer = "answer.json";

    json dictConfig;
    json dictRequests;

public:
    ConverterJSON() = default;
    ConverterJSON(string config, string requests, string answer);
    /**
     * Метод получения содержимого файлов
     * @return Возвращает список с содержимым файлов перечисленных
     * в config.json
     */
    std::vector<std::string> GetTextDocuments();
    /**
     * Метод считывает поле max_responses для определения предельного
     * количества ответов на один запрос
     * @return
     */
    int GetResponsesLimit();
    /**
     * Метод получения запросов из файла requests.json
     * @return возвращает список запросов из файла requests.json
     */
    std::vector<std::string> GetRequests();
    /**
     * Положить в файл answers.json результаты поисковых запросов
     */
    void putAnswers(std::vector<std::vector<std::pair<int, float>>> answers);
};

ConverterJSON::ConverterJSON(string _config, string _requests, string _answer)
{
    pathConfig = _config;
    pathRequests = _requests;
    pathAnswer = _answer;

    ifstream fileConfig(pathConfig);
    if (fileConfig.is_open())
        fileConfig >> dictConfig;
    else
        throw runtime_error("Config file is missing!");

    fileConfig.close();
    if (dictConfig.count("config") == 0)
        throw runtime_error("Config file is empty!");

    json conf = dictConfig["config"];
    cout << endl
         << conf["name"] << endl;
    if (conf["version"] != MYPROJECT_VERSION)
        throw runtime_error("Has incorrect file version!");

    ifstream fileRequests(pathRequests);
    if (fileRequests.is_open())
        fileRequests >> dictRequests;

    fileRequests.close();

    ofstream fileAnswer(pathAnswer);
    fileAnswer.close();
}

vector<string> ConverterJSON::GetTextDocuments()
{
    vector<string> paths = dictConfig["files"];
    vector<string> textDocs;
    for (int i = 0; i < paths.size(); i++)
    {
        ifstream file(paths[i]);
        string word = "";
        if (file.is_open())
        {
            textDocs.push_back(word);
            while (file >> word)
            {
                textDocs.back() += word;
                textDocs.back() += " ";
            }
            textDocs.back().pop_back();
        }

        file.close();
    }

    return textDocs;
}

int ConverterJSON::GetResponsesLimit()
{
    json conf = dictConfig["config"];
    return conf["max_responses"];
}

vector<string> ConverterJSON::GetRequests()
{
    return dictRequests["requests"];
}

void ConverterJSON::putAnswers(vector<vector<pair<int, float>>> answers)
{
    json dictReqs;
    int reqNum = 0;
    for (auto &requests : answers)
    {
        reqNum++;
        json dictReq;
        if (!requests.empty())
        {
            dictReq["result"] = "true";
            vector<json> relevances;
            for (auto &relevance : requests)
            {
                json rel;
                rel["docid"] = relevance.first;
                rel["rank"] = relevance.second;
                relevances.push_back(rel);
            }

            dictReq["relevance"] = relevances;
        }
        else
            dictReq["result"] = "false";

        string requestID = "request" + string(3 - to_string(reqNum).length(), '0') + to_string(reqNum);
        dictReqs[requestID] = dictReq;
    }

    json dictAnswers;
    dictAnswers["answers"] = dictReqs;
    ofstream fileAnswer("answer.json");
    if (fileAnswer.is_open())
        fileAnswer << dictAnswers;
}

TEST(sample_test_case, sample_test)
{
    EXPECT_EQ(1, 1);
}

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
    void UpdateDocumentBase(std::vector<std::string> input_docs);
    /**
     * Метод определяет количество вхождений слова word в загруженной базе документов
     * @param word слово, частоту вхождений которого необходимо определить
     * @return возвращает подготовленный список с частотой слов
     */
    std::vector<Entry> GetWordCount(const std::string &word);

private:
    std::vector<std::string> docs;                             // список содержимого документов
    std::map<std::string, std::vector<Entry>> freq_dictionary; // частотный словарь
    mutex freq_dict_access;
};

void InvertedIndex::UpdateDocumentBase(vector<string> input_docs)
{
    docs = input_docs;
    vector<thread> threads;
    for (int i = 0; i < docs.size(); i++)
    {
        threads.emplace_back([this, i]()
                             {
                                string doc = this->docs[i];
                                vector<string> words;
                                istringstream iss(doc);
                                string word;
                                while(iss >> word) words.push_back(word);

                                for (auto &w : words)
                                {
                                    this->freq_dict_access.lock();
                                    if (this->freq_dictionary.count(w) > 0)
                                    {
                                        bool nFound = true;
                                        for (auto &f : this->freq_dictionary[w])
                                        {
                                            if (f.doc_id == (size_t)i)
                                            {
                                                f.count += 1;
                                                nFound = false;
                                                break;
                                            }
                                        }

                                        if(nFound) this->freq_dictionary[w].push_back({(size_t)i, 1});
                                    }
                                    else this->freq_dictionary[w] = {{(size_t)i, 1}};

                                    this->freq_dict_access.unlock();
                                } });
    }

    for (auto &t : threads)
        t.join();
}

vector<Entry> InvertedIndex::GetWordCount(const std::string &word)
{
    if (freq_dictionary.count(word) > 0)
        return freq_dictionary[word];

    return {};
}

void TestInvertedIndexFunctionality(
    const vector<string> &docs,
    const vector<string> &requests,
    const std::vector<vector<Entry>> &expected)
{

    std::vector<std::vector<Entry>> result;
    InvertedIndex idx;

    idx.UpdateDocumentBase(docs);

    for (auto &request : requests)
    {
        std::vector<Entry> word_count = idx.GetWordCount(request);
        result.push_back(word_count);
    }

    ASSERT_EQ(result, expected);
}

TEST(TestCaseInvertedIndex, TestBasic)
{
    const vector<string> docs = {
        "london is the capital of great britain",
        "big ben is the nickname for the Great bell of the striking clock"};

    const vector<string> requests = {"london", "the"};

    const vector<vector<Entry>> expected = {
        {{0, 1}},
        {{0, 1}, {1, 3}}};

    TestInvertedIndexFunctionality(docs, requests, expected);
}

TEST(TestCaseInvertedIndex, TestBasic2)
{
    const vector<string> docs = {
        "milk milk milk milk water water water",
        "milk water water",
        "milk milk milk milk milk water water water water water",
        "americano cappuccino"};

    const vector<string> requests = {"milk", "water", "cappuccino"};

    const vector<vector<Entry>> expected = {
        {{0, 4}, {1, 1}, {2, 5}},
        {{0, 3}, {1, 2}, {2, 5}},
        {{3, 1}}};

    TestInvertedIndexFunctionality(docs, requests, expected);
}

TEST(TestCaseInvertedIndex, TestInvertedIndexMissingWord)
{
    const vector<string> docs = {
        "a b c d e f g h i j k l",
        "statement"};

    const vector<string> requests = {"m", "statement"};

    const vector<vector<Entry>> expected = {
        {},
        {{1, 1}}};

    TestInvertedIndexFunctionality(docs, requests, expected);
}

int main(int argc, char **argv)
{
    // Если передан аргумент --gtest, запускаем тесты
    if (argc > 1 && std::string(argv[1]) == "--gtest")
    {
        ::testing::InitGoogleTest(&argc, argv);
        return RUN_ALL_TESTS();
    }

    std::cout << "Hello, from search_engine!\n";
}
