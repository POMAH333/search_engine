#include "converter.h"

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

vector<string> ConverterJSON::getTextDocuments()
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

int ConverterJSON::getResponsesLimit()
{
    json conf = dictConfig["config"];
    return conf["max_responses"];
}

vector<string> ConverterJSON::getRequests()
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