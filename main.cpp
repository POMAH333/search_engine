#include <iostream>
#include <string>
#include <vector>
#include <utility>
#include <fstream>
#include <exception>
#include <thread>
#include <mutex>
#include <sstream>
#include <unordered_set>

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
        try
        {
            ifstream file(paths[i]);
            string word = "";
            if (file.is_open())
            {
                textDocs.push_back(word);
                int wordCount = 0;
                while (file >> word)
                {
                    if (word.length() > 100)
                        continue;

                    textDocs.back() += word;
                    textDocs.back() += " ";
                    wordCount++;
                    if (wordCount == 1000)
                        break;
                }
                textDocs.back().pop_back();
            }
            else
                throw invalid_argument(paths[i]);

            file.close();
        }
        catch (const invalid_argument &x)
        {
            cerr << "File " << x.what() << " not found!!!" << endl;
        }
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
    vector<string> req = dictRequests["requests"];
    if (req.size() >= 1000)
        req.clear();

    bool wordLimit = false;
    for (auto &r : req)
    {
        istringstream iss(r);
        string word;
        int wordCount;
        while (iss >> word)
        {
            wordCount++;
            if (wordCount > 10)
            {
                wordLimit = true;
            }
        }
    }
    if (wordLimit)
        req.clear();

    return req;
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

vector<vector<RelativeIndex>> SearchServer::search(const vector<string> &queries_input)
{
    vector<vector<RelativeIndex>> searchResult;
    for (auto &querie : queries_input)
    {
        unordered_set<string> words;
        istringstream iss(querie);
        string word;
        while (iss >> word)
            words.insert(word);

        float maxAbsoluteRelevance = 0.;
        map<size_t, float, std::greater<size_t>> docAbsRelevance;
        for (auto &w : words)
        {
            auto wordDocs = _index.GetWordCount(w);
            if (!wordDocs.empty())
                for (auto &doc : wordDocs)
                {
                    docAbsRelevance[doc.doc_id] += doc.count;
                    if (docAbsRelevance[doc.doc_id] > maxAbsoluteRelevance)
                        maxAbsoluteRelevance = docAbsRelevance[doc.doc_id];
                }
        }

        if (docAbsRelevance.empty())
        {
            searchResult.push_back({});
            continue;
        }

        for (auto it = docAbsRelevance.begin(); it != docAbsRelevance.end(); it++)
            it->second /= maxAbsoluteRelevance;

        int respCount = 0;
        vector<RelativeIndex> sortQueryIndex;
        while (!docAbsRelevance.empty())
        {
            auto it = docAbsRelevance.begin();
            auto maxIt = it;
            it++;
            for (it; it != docAbsRelevance.end(); it++)
                if (it->second >= maxIt->second)
                    maxIt = it;

            sortQueryIndex.push_back({maxIt->first, maxIt->second});
            docAbsRelevance.erase(maxIt);
            respCount++;
            if (respCount == respLimit)
                break;
        }

        searchResult.push_back(sortQueryIndex);
    }

    return searchResult;
}

TEST(TestCaseSearchServer, TestSimple)
{
    const vector<string> docs = {
        "milk milk milk milk water water water",
        "milk water water",
        "milk milk milk milk milk water water water water water",
        "americano cappuccino"};
    const vector<string> request = {"milk water", "sugar"};
    const std::vector<vector<RelativeIndex>> expected = {
        {{2, 1},
         {0, 0.7},
         {1, 0.3}},
        {}};

    InvertedIndex idx;
    idx.UpdateDocumentBase(docs);
    SearchServer srv(idx);
    std::vector<vector<RelativeIndex>> result = srv.search(request);
    ASSERT_EQ(result, expected);
}

TEST(TestCaseSearchServer, TestTop5)
{
    const vector<string> docs = {
        "london is the capital of great britain",
        "paris is the capital of france",
        "berlin is the capital of germany",
        "rome is the capital of italy",
        "madrid is the capital of spain",
        "lisboa is the capital of portugal",
        "bern is the capital of switzerland",
        "moscow is the capital of russia",
        "kiev is the capital of ukraine",
        "minsk is the capital of belarus",
        "astana is the capital of kazakhstan",
        "beijing is the capital of china",
        "tokyo is the capital of japan",
        "bangkok is the capital of thailand",
        "welcome to moscow the capital of russia is third rome",
        "amsterdam is the capital of netherlands",
        "helsinki is the capital of finland",
        "oslo is the capital of norway",
        "stockholm is the capital of sweden",
        "riga is the capital of latvia",
        "tallinn is the capital of estonia",
        "warsaw is the capital of poland"};
    const vector<string> request = {"moscow is the capital of russia"};
    const std::vector<vector<RelativeIndex>> expected = {
        {{7, 1},
         {14, 1},
         {0, 0.666666687},
         {1, 0.666666687},
         {2, 0.666666687}}};

    InvertedIndex idx;
    idx.UpdateDocumentBase(docs);
    SearchServer srv(idx);
    std::vector<vector<RelativeIndex>> result = srv.search(request);
    ASSERT_EQ(result, expected);
}

int main(int argc, char **argv)
{
    // Если передан аргумент --gtest, запускаем тесты
    if (argc > 1 && std::string(argv[1]) == "--gtest")
    {
        ::testing::InitGoogleTest(&argc, argv);
        return RUN_ALL_TESTS();
    }

    try
    {
        ConverterJSON *converter = new ConverterJSON("config.json", "requests.json", "answer.json");
        InvertedIndex *invertedIndex = new InvertedIndex();
        SearchServer *searchServer = new SearchServer(*invertedIndex);
        searchServer->setRespLimit(converter->GetResponsesLimit());

        invertedIndex->UpdateDocumentBase(converter->GetTextDocuments());
        auto searchResults = searchServer->search(converter->GetRequests());
        vector<vector<pair<int, float>>> answers;
        for (auto &result : searchResults)
        {
            vector<pair<int, float>> answer;
            for (auto &p : result)
                answer.push_back({(int)p.doc_id, p.rank});

            answers.push_back(answer);
        }

        converter->putAnswers(answers);

        delete searchServer;
        delete invertedIndex;
        delete converter;
    }
    catch (const runtime_error &x)
    {
        cerr << x.what() << endl;
    }

    return 0;
}
