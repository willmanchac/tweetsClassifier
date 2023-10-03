#include "DSString.h"
#include <iostream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <sstream>
#include <stdexcept>
#include <fstream>
#include <iomanip>

std::unordered_set<std::string> stopwords;

// Tokenization function
std::vector<DSString> tokenizeAndRemoveStopwords(const DSString& line, const std::unordered_set<std::string>& stopwords) {
    std::vector<DSString> words;

    size_t textLen = line.length();
    size_t i = 0;
    size_t start = 0;

    while (i <= textLen) {
        if (i == textLen || !std::isalpha(line[i])) {
            if (i > start) {
                DSString word(line.c_str() + start, line.c_str() + i);
                words.push_back(word);
            }
            start = i + 1;
        }
        i++;
    }

    // Now, you can remove stopwords from the 'words' vector
    std::vector<DSString> wordsWithoutStopwords;
    for (const DSString& word : words) {
        if (stopwords.find(word.c_str()) == stopwords.end()) {
            // Word is not a stopword, add it to the result
            wordsWithoutStopwords.push_back(word);
        }
    }

    return wordsWithoutStopwords;
}

class SentimentClassifier {
public:
    SentimentClassifier() : totalPositiveWords(0), totalNegativeWords(0) {}

    // Specify the custom hash function for the unordered_map
    std::unordered_map<DSString, int, DSString::Hash> positiveWordCounts;
    std::unordered_map<DSString, int, DSString::Hash> negativeWordCounts;

    // Load training data from the training file
    void loadTrainingData(const std::string& trainingFile, std::vector<std::vector<DSString>>& positiveTweets, std::vector<std::vector<DSString>>& negativeTweets) {
    // Load stopwords from a CSV file
    loadStopwords("stopwords.csv");

    std::ifstream inputFile(trainingFile);
    if (!inputFile.is_open()) {
        std::cerr << "Failed to open training data file." << std::endl;
        return;
    }

    std::string line;
    while (std::getline(inputFile, line)) {

        std::istringstream iss(line);
        std::string sentimentStr, tweetId, timestamp, otherFields, username, quotedTweetText;

        if (std::getline(iss, sentimentStr, ',') &&
            std::getline(iss, tweetId, ',') &&
            std::getline(iss, timestamp, ',') &&
            std::getline(iss, otherFields, ',') &&
            std::getline(iss, username, ',') &&
            std::getline(iss, quotedTweetText, ',')) {

            // Tokenize the tweet text and remove stopwords
            DSString dsQuotedTweetText(quotedTweetText.c_str());
            std::vector<DSString> tokens = tokenizeAndRemoveStopwords(dsQuotedTweetText,stopwords);

            // Assuming "4" for positive sentiment and "0" for negative sentiment
            DSString dsSentiment(sentimentStr.c_str());

            if (dsSentiment == "4") {
                positiveTweets.push_back(tokens);
            } else if (dsSentiment == "0") {
                negativeTweets.push_back(tokens);
            }
        }else {
            std::cerr << "Failed to parse line: " << line << std::endl;
        }
    }
    inputFile.close();
}

    // Load testing data and sentiments from the testing and sentiment files
void loadTestingData(const std::string& testingFile, const std::string& sentimentFile, std::vector<std::vector<DSString>>& positiveTweets, std::vector<std::vector<DSString>>& negativeTweets, const std::string& resultFile, const std::string& accuracyFile) {
    std::ifstream testDataFile(testingFile);
    std::ifstream sentimentDataFile(sentimentFile);
    std::ofstream resultFileStream(resultFile); // Open the result file for writing
    std::ofstream accuracyFileStream(accuracyFile); // Open the accuracy file for writing

    if (!testDataFile.is_open() || !sentimentDataFile.is_open() || !resultFileStream.is_open() || !accuracyFileStream.is_open()) {
        std::cerr << "Failed to open testing, sentiment, or result data file." << std::endl;
        return;
    }

    int correctPredictions = 0;
    int totalPredictions = 0;

    std::string testDataLine, sentimentLine;

    // Calculate accuracy here
    // double accuracy = 0.00; // Default to 0 if there are no predictions

    // Write accuracy value to the accuracy file as the first line
    // accuracyFileStream << std::fixed << std::setprecision(3) << accuracy << std::endl;

    while (std::getline(testDataFile, testDataLine) && std::getline(sentimentDataFile, sentimentLine)) {
        std::istringstream iss(testDataLine);
        std::string tweetId, timestamp, otherFields, username, quotedTweetText;

        if (std::getline(iss, tweetId, ',') &&
            std::getline(iss, timestamp, ',') &&
            std::getline(iss, otherFields, ',') &&
            std::getline(iss, username, ',') &&
            std::getline(iss, quotedTweetText, ',')) {

            // Tokenize the tweet text and remove stopwords
            DSString dsQuotedTweetText(quotedTweetText.c_str());
            std::vector<DSString> testingTokens = tokenizeAndRemoveStopwords(dsQuotedTweetText, stopwords);
            std::string sentimentFinal = classifySentiment(testingTokens, positiveTweets, negativeTweets);

            DSString dsSentiment(sentimentLine.c_str());
            // Write the results to the result file
            resultFileStream << sentimentFinal << ',' << tweetId << std::endl;

            // std::cout << sentimentFinal<< " " << dsSentiment.c_str() << std::endl;
            if (std::stoi(sentimentFinal) == std::stoi(dsSentiment.c_str())) {
                correctPredictions++;
            } else {
                // Prediction was incorrect, write the misclassified tweet to the accuracy file
                // std::ofstream accuracyFileStream(accuracyFile, std::ios::app); // Open the accuracy file in append mode
                accuracyFileStream << sentimentFinal << ',' << dsSentiment.c_str() << std::endl;
            }
            totalPredictions++;
        }else {
            std::cerr << "Failed to parse line: " << testDataLine << std::endl;
        }
    }

    // std::cout << correctPredictions << " " << totalPredictions << std::endl;
    double accuracy = static_cast<double>(correctPredictions) / static_cast<double>(totalPredictions);
    // std::ofstream accuracyFileStream(accuracyFile);
    accuracyFileStream << std::fixed << std::setprecision(3) << accuracy << std::endl;

    // Close the result file and accuracy file
    resultFileStream.close();
    accuracyFileStream.close();
    testDataFile.close();
    sentimentDataFile.close();
}


std::string classifySentiment(const std::vector<DSString>& tokens, const std::vector<std::vector<DSString>>& positiveTweets, const std::vector<std::vector<DSString>>& negativeTweets) {
    int positiveCount = 0;
    int negativeCount = 0;

    for (const DSString& token : tokens) {
        // Check if the token is present in positiveTweets
        for (const std::vector<DSString>& positiveTweet : positiveTweets) {
            if (containsString(positiveTweet, token)) {
                positiveCount++;
                break; // No need to continue checking if the token is already found in positiveTweets
            }
        }

        // Check if the token is present in negativeTweets
        for (const std::vector<DSString>& negativeTweet : negativeTweets) {
            if (containsString(negativeTweet, token)) {
                negativeCount++;
                break; // No need to continue checking if the token is already found in negativeTweets
            }
        }
    }

    if (positiveCount > negativeCount) {
        return "4";
    } else
        return "0";  // Negative sentiment
}

bool containsString(const std::vector<DSString>& vec, const DSString& str) {
    for (const DSString& item : vec) {
        if (item == str) {
            return true;
        }
    }
    return false;
}


    // Write classification results to the results file
    void writeResults(const std::string& resultsFile, const std::vector<DSString>& results) {
        std::ofstream outputFile(resultsFile);
        if (!outputFile.is_open()) {
            std::cerr << "Failed to open results file for writing." << std::endl;
            return;
        }

        for (const DSString& result : results) {
            outputFile << result.c_str() << std::endl;
        }

        outputFile.close();
    }

    void loadStopwords(const std::string& stopwordsFile) {
    std::ifstream stopwordsInput(stopwordsFile);
    if (!stopwordsInput.is_open()) {
        std::cerr << "Failed to open stopwords file." << std::endl;
        return;
    }

    std::string word;
    while (std::getline(stopwordsInput, word)) {
        stopwords.insert(word);
    }

    stopwordsInput.close();
}

    private:
    int totalPositiveWords;
    int totalNegativeWords;

};
