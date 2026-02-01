#include "server.h"

using namespace std;

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
            auto wordDocs = _index.getWordCount(w);
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