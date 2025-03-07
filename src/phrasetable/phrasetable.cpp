#include <cctype>
#include <cstdio>
#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <utility>
#include <unordered_map>
#include "phrasetable.h"

using std::cout;
using std::unordered_map;
using std::make_pair;
using std::min;
using std::pair;
using std::runtime_error;
using std::endl;

PhraseTable PhraseTableLoader::load_phrase_table(const string& path,
                                                 size_t best_trans_num) const {
  cout << "Loading phrase table" << endl;
  PhraseTable phrase_table;
  FILE* file = fopen(path.c_str(), "r");
  Phrase prev_fr_phrase;
  vector<Translation> translations;
  // skip first line with headers
  int read_symbol = fgetc(file);
  while (read_symbol != '\n') {
    read_symbol = fgetc(file);
  }
  int line_number = 0;
  while((read_symbol = fgetc(file)) != EOF) {
    ++line_number;
    if (line_number % 5000000 == 0) {
      cout << "Read " << line_number << " lines" << endl;
    }
    Phrase fr_phrase = ExtractPhrase(file, read_symbol);
    SkipDelimiter(file, read_symbol);
    Phrase en_phrase = ExtractPhrase(file, read_symbol);
    SkipDelimiter(file, read_symbol);
    double probability = ExtractProbability(file, read_symbol);
    if (prev_fr_phrase.empty()) {
      prev_fr_phrase = fr_phrase;
    }
    if (!EqualPhrases(fr_phrase, prev_fr_phrase)) {
      // sort and remain only top translations
      CompareTranslations compare_translations;
      sort(translations.begin(), translations.end(), compare_translations);
      phrase_table[prev_fr_phrase] = vector<Translation>();
      for (size_t index = 0; index < min(translations.size(), best_trans_num);
           ++index) {
        phrase_table[prev_fr_phrase].push_back(translations[index]);
      }
      translations.clear();
      prev_fr_phrase = fr_phrase;
    }
    translations.push_back(Translation(en_phrase, probability));
  }
  cout << "Phrase table loaded" << endl;
  return phrase_table;
}

Phrase PhraseTableLoader::ExtractPhrase(FILE* file, int& read_symbol) const {
  Phrase phrase;
  while (read_symbol != '|') {
    string number;
    while (isdigit(read_symbol)) {
      number.push_back(read_symbol);
      read_symbol = fgetc(file);
    }
    int phrase_element = atoi(number.data());
    phrase.push_back(phrase_element);
    read_symbol = fgetc(file);
  }
  return phrase;
}

void PhraseTableLoader::SkipDelimiter(FILE* file, int& read_symbol) const {
  while (read_symbol == '|') {
    read_symbol = fgetc(file);
  }
  read_symbol = fgetc(file);
}

double PhraseTableLoader::ExtractProbability(FILE* file,
                                             int& read_symbol) const {
  double probability;
  if (fscanf(file, "%lg", &probability) != 1) {
    throw runtime_error("Failed to read phrase table");
  }
  read_symbol = fgetc(file); // skip '\n'
  return probability;
}

bool PhraseTableLoader::EqualPhrases(const Phrase& first,
                                     const Phrase& second) const {
  if (first.size() != second.size()) {
    return false;
  }
  for (size_t index = 0; index < first.size(); ++index) {
    if (first[index] != second[index]) {
      return false;
    }
  }
  return true;
}
