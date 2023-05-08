/*
 ===============================================================================
 Name        : merge.cpp
 Author      : Fastiraz
 Version     : 1.0
 Copyright   : null
 Description : Merge multiple wordlists into a single output file.
 Compile     : g++ merge.cpp -o merge -std=c++11 -pthread
 Usage       : ./merge secret.txt list.txt file.txt -o output.txt -t 4
 ===============================================================================
 */

#include <iostream>
#include <fstream>
#include <unordered_set>
#include <string>
#include <vector>
#include <chrono>
#include <thread>
#include <algorithm>

using namespace std;

// The function executed by each thread.
void merge_files(const vector<string>& files, unordered_set<string>& words) {
    for (const string& file : files) {
        ifstream input(file);
        if (!input) {
            cerr << "Failed to open file " << file << endl;
            exit(1);
        }
        string word;
        while (input >> word) {
            words.insert(word);
        }
        input.close();
    }
}

int main(int argc, char** argv) {
    // Start measuring time
    chrono::high_resolution_clock::time_point begin = chrono::high_resolution_clock::now();

    // Default values for output file and number of threads
    string output_file = "output.txt";
    int num_threads = 4;

    // Parse command-line arguments
    vector<string> files;
    for (int i = 1; i < argc; i++) {
        string arg = argv[i];
        if (arg == "-o" || arg == "--output") {
            i++;
            if (i >= argc) {
                cerr << "Missing output file name after -o" << endl;
                exit(1);
            }
            output_file = argv[i];
        } else if (arg == "-t" || arg == "--threads") {
            i++;
            if (i >= argc) {
                cerr << "Missing number of threads after -t" << endl;
                exit(1);
            }
            num_threads = stoi(argv[i]);
            if (num_threads <= 0) {
                cerr << "Invalid number of threads: " << num_threads << endl;
                exit(1);
            }
        } else if (arg == "-h" || arg == "--help" || arg == "-?") {
            cout << "Usage: " << argv[0] << " [-o output_file] [-t num_threads] file1 file2 ... fileN" << endl;
            cout << "Merge multiple word lists into a single output file." << endl;
            cout << "Options:" << endl;
            cout << "  -o, --output FILE       Specify output file name. Default is 'output.txt'." << endl;
            cout << "  -t, --threads NUM       Specify number of threads to use. Default is 4." << endl;
            cout << "  -h, --help, -?          Display this help message." << endl;
            return 0;
        } else {
            files.push_back(arg);
        }
    }

    if (files.empty()) {
        cerr << "Usage: " << argv[0] << " [-o <output_file>] [-t <num_threads>] <file1> <file2> ... <fileN>" << endl;
        exit(1);
    }

    // Create and start threads
    vector<thread> threads;
    vector<unordered_set<string>> word_sets(num_threads);
    for (int i = 0; i < num_threads; i++) {
        vector<string> thread_files;
        for (size_t j = i; j < files.size(); j += num_threads) {
            thread_files.push_back(files[j]);
        }
        threads.emplace_back(merge_files, thread_files, ref(word_sets[i]));
    }

    // Wait for threads to finish and merge results
    unordered_set<string> words;
    for (thread& t : threads) {
        t.join();
    }
    for (const unordered_set<string>& word_set : word_sets) {
        words.insert(word_set.begin(), word_set.end());
    }

    // Write output file
    ofstream output(output_file, ios_base::app);
    if (!output) {
        cerr << "Failed to open output file " << output_file << endl;
        exit(1);
    }
    for (const string& word : words) {
        output << word << endl;
    }
    output.close();

    cout << "Wordlists merged successfully into " << output_file << endl;

    // Stop measuring time and calculate the elapsed time
    chrono::high_resolution_clock::time_point end = chrono::high_resolution_clock::now();
    chrono::nanoseconds elapsed = chrono::duration_cast<chrono::nanoseconds>(end - begin);

    printf("Time measured: %.3f seconds.\n", elapsed.count() * 1e-9);
    printf("%d theads used.\n", num_threads);

    return 0;
}
