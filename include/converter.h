#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <exception>
#include <sstream>

#include "nlohmann/json.hpp"

using namespace std;
using namespace nlohmann;

/**
 *  Класс для работы с JSON-файлами
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
    std::vector<std::string> getTextDocuments();
    /**
     * Метод считывает поле max_responses для определения предельного
     * количества ответов на один запрос
     * @return
     */
    int getResponsesLimit();
    /**
     * Метод получения запросов из файла requests.json
     * @return возвращает список запросов из файла requests.json
     */
    std::vector<std::string> getRequests();
    /**
     * Положить в файл answers.json результаты поисковых запросов
     */
    void putAnswers(std::vector<std::vector<std::pair<int, float>>> answers);
};
