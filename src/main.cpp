#include <iostream>
#include <vector>
#include <utility>

#include "converter.h"
#include "server.h"
#include "inverted_index.h"

using namespace std;

int main()
{
    try
    {
        ConverterJSON *converter = new ConverterJSON("config.json", "requests.json", "answer.json");
        InvertedIndex *invertedIndex = new InvertedIndex();
        SearchServer *searchServer = new SearchServer(*invertedIndex);
        searchServer->setRespLimit(converter->getResponsesLimit());

        invertedIndex->updateDocumentBase(converter->getTextDocuments());
        auto searchResults = searchServer->search(converter->getRequests());
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