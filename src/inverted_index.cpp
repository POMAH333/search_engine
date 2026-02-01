#include "inverted_index.h"

void InvertedIndex::updateDocumentBase(vector<string> input_docs)
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

vector<Entry> InvertedIndex::getWordCount(const std::string &word)
{
    if (freq_dictionary.count(word) > 0)
        return freq_dictionary[word];

    return {};
}
