// 
#include <iostream>
#include <cstring>
#include <vector>
#include <unordered_map>
#include <unordered_set> // Add this line
#include <sstream>
#include <stdexcept>
#include <fstream>
#include <iomanip>

std::unordered_set<std::string> stopwords;

class DSString {
private:
    char* data;
    size_t len;

public:
    // Constructors
    DSString() : data(nullptr), len(0) {}

    DSString(const char* str) {
        len = strlen(str);
        data = new char[len + 1];
        strcpy(data, str);
    }

    DSString(const char* begin, const char* end) {
        len = end - begin;
        data = new char[len + 1];
        strncpy(data, begin, len);
        data[len] = '\0'; // Null-terminate the string
    }

    DSString(const DSString& other) : len(other.len) {
        data = new char[len + 1];
        strcpy(data, other.data);
    }

    // Destructor
    ~DSString() {
        delete[] data;
    }

    // Assignment operator
    DSString& operator=(const DSString& other) {
        if (this != &other) {
            delete[] data;
            len = other.len;
            data = new char[len + 1];
            strcpy(data, other.data);
        }
        return *this;
    }

    // Concatenation operator
    DSString operator+(const DSString& other) const {
        DSString result;
        result.len = len + other.len;
        result.data = new char[result.len + 1];
        strcpy(result.data, data);
        strcat(result.data, other.data);
        return result;
    }

    // Equality operator
    bool operator==(const DSString& other) const {
        return strcmp(data, other.data) == 0;
    }

    // Length function
    size_t length() const {
        return len;
    }

    // Access character at index
    char operator[](size_t index) const {
        if (index < len) {
            return data[index];
        }
        throw std::out_of_range("Index out of range");
    }

    // Convert to C-style string
    const char* c_str() const {
        return data;
    }

    // Custom hash function for DSString
    struct Hash {
        size_t operator()(const DSString& str) const {
            // You can use a simple hash function like this
            size_t hash = 0;
            for (size_t i = 0; i < str.len; i++) {
                hash = (hash * 31) + static_cast<size_t>(str.data[i]);
            }
            return hash;
        }
    };
};

void loadStopwords(const std::string& stopwordsFile) {
    std::ifstream stopwordsInput(stopwordsFile);
    if (!stopwordsInput.is_open()) {
        std::cerr << "Failed to open stopwords file." << std::endl;
        return;
    }

    std::string word;
    while (std::getline(stopwordsInput, word)) {
        // DSString dsWord(word.c_str()); // Convert to DSString
        stopwords.insert(word);
    }

    stopwordsInput.close();
}


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



class Tweet {
public:
    Tweet(const std::vector<DSString>& textTokens, DSString sentiment) : sentiment(sentiment) {
    // Ensure the textTokens vector is not empty
    if (textTokens.empty()) {
        // Handle the case where textTokens is empty
        text = "No text available";

    } else {
        // Combine the DSString tokens into a single DSString for text
        for (const DSString& token : textTokens) {
            text = text + " " + token;
        }
    }
}
    DSString getText() const { return text; }
    DSString getSentiment() const { return sentiment; }
    void outputTextToCout() const {
        std::cout << "Text: " << text.c_str() << std::endl;
    }

private:
    DSString text;
    DSString sentiment;
};

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

    private:
    int totalPositiveWords;
    int totalNegativeWords;

};



int main(int argc, char* argv[]) {
    // Example usage

    DSString str1 = "Hello, ";
    DSString str2 = "world!";

    DSString result = str1 + str2;
    std::cout << result.c_str() << std::endl; // Output: Hello, world!

    if (str1 == str2) {
        std::cout << "Strings are equal." << std::endl;
    } else {
        std::cout << "Strings are not equal." << std::endl; // Output: Strings are not equal.
    }

    std::cout << "Length of result: " << result.length() << std::endl; // Output: Length of result: 13

    try {
        char character = result[6];
        std::cout << "Character at index 6: " << character << std::endl; // Output: Character at index 6: ,

        // Attempt to access an out-of-range index
        char outOfRangeChar = result[20]; // Throws std::out_of_range exception
    } catch (const std::out_of_range& e) {
        std::cerr << "Error: " << e.what() << std::endl; // Output: Error: Index out of range
    }

    // Tokenization test
    DSString tweetText = "This is a sample tweet! #NLP #Tokenization";
    std::vector<DSString> words = tokenizeAndRemoveStopwords(tweetText,stopwords);

    for (const DSString& word : words) {
        std::cout << word.c_str() << std::endl;
    }


    if (argc != 6) {
        std::cerr << "Usage: " << argv[0] << " <training_file> <testing_file> <sentiment_file> <results_file> <accuracy_file>" << std::endl;
        return 1;
    }

    // Extract command-line arguments
    std::string trainingFile = argv[1];
    std::string testingFile = argv[2];
    std::string sentimentFile = argv[3];
    std::string resultsFile = argv[4];
    std::string accuracyFile = argv[5];

    std::cout << trainingFile << " " << testingFile << " " << sentimentFile << " " << resultsFile << " " << accuracyFile<<std::endl;

    // Load training data and testing data
    std::vector<std::vector<DSString>> positiveTweets;
    std::vector<std::vector<DSString>> negativeTweets;
    std::vector<Tweet> testingTweets;
    std::vector<DSString> testingSentiments;

    // Create an instance of SentimentClassifier
    SentimentClassifier classifier;

    // Load training data from the training file
    classifier.loadTrainingData(trainingFile, positiveTweets, negativeTweets);

    // Load testing data and sentiments from the testing and sentiment files
    classifier.loadTestingData(testingFile, sentimentFile, positiveTweets,negativeTweets,resultsFile,accuracyFile);



    return 0;
}
