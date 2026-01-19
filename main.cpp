#include <iostream>
#include <string>
#include <vector>
#include <utility>
#include <fstream>
#include <exception>

#include "nlohmann/json.hpp"

using namespace std;
using namespace nlohmann;

/**
 *Класс для работы с JSON-файлами
 */
class ConverterJSON
{
    json dictConfig;
    json dictRequests;

public:
    ConverterJSON() = default;
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

ConverterJSON::ConverterJSON()
{
    ifstream fileConfig("config.json");
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

    ifstream fileRequests("requests.json");
    if (fileRequests.is_open())
        fileRequests >> dictRequests;

    fileRequests.close();

    ofstream fileAnswer("answer.json");
    fileAnswer.close();
}

vector<string> ConverterJSON::GetTextDocuments()
{
    return dictConfig["files"];
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

int main(int, char **)
{
    std::cout << "Hello, from search_engine!\n";
}
