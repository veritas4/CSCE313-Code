#include <vector> // vector, push_back, at
#include <string> // string
#include <iostream> // cin, getline
#include <fstream> // ofstream
#include <unistd.h> // getopt, exit, EXIT_FAILURE
#include <assert.h> // assert
#include <thread> // thread, join
#include <sstream> // stringstream
#include <string.h>
#include <stdio.h>

#include "BoundedBuffer.h" // BoundedBuffer class

#define MAX_MSG_LEN 256

using namespace std;

/************** Helper Function Declarations **************/

void parse_column_names(vector<string>& _colnames, const string& _opt_input);
void write_to_file(const string& _filename, const string& _text, bool _first_input=false);

/************** Thread Function Definitions **************/

// "primary thread will be a UI data entry point"
void ui_thread_function(BoundedBuffer* bb) {
    // TODO: implement UI Thread Function
    // In an infinite loop,  constinously promp the user for input
    for (;;) {
        string input;
        cout << "enter data> ";
        getline(cin,input);
        bb->push((char *) input.c_str() , input.length());

        if (input == "Exit" || input == "exit") {
            break;
        }
    }
    // if Exit break
    // Else push onto bounded buffer
}

// "second thread will be the data processing thread"
// "will open, write to, and close a csv file"
void data_thread_function(BoundedBuffer* bb, string filename, const vector<string>& colnames) {
    // TODO: implement Data Thread Function
    // (NOTE: use "write_to_file" function to write to file)
    for (auto i : colnames) {
        if (i == colnames.front()) {
            write_to_file(filename, i + ", ", true);
        }
        else if (i == colnames.back()) {
            write_to_file(filename, i + "\n", false);
        }
        else {
            write_to_file(filename, i + ", ", false);
        }
    }

    size_t counter = 0;
    for (;;) {
        char msg[MAX_MSG_LEN] = {};

        bb->pop(msg, MAX_MSG_LEN);
        if (!strcmp(msg,"Exit") || !strcmp(msg,"exit")) {
            break;
        }
        if (counter != colnames.size() - 1) {
            write_to_file(filename, (const string) msg + ", ", false);
            counter++;
        }
        else {
            write_to_file(filename, (const string) msg + "\n", false);
            counter = 0;
        }
    }
    // 
}

/************** Main Function **************/

int main(int argc, char* argv[]) {

    // variables to be used throughout the program
    vector<string> colnames; // column names
    string fname; // filename
    BoundedBuffer* bb = new BoundedBuffer(3); // BoundedBuffer with cap of 3

    // read flags from input
    int opt;
    while ((opt = getopt(argc, argv, "c:f:")) != -1) {
        switch (opt) {
            case 'c': // parse col names into vector "colnames"
                parse_column_names(colnames, optarg);
                break;
            case 'f':
                fname = optarg;
                break;
            default: // invalid input, based on https://linux.die.net/man/3/getopt
                fprintf(stderr, "Usage: %s [-c colnames] [-f filename]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }
    // Create file with column headers of column names
    
    // TODO: instantiate ui and data threads
    std::thread UI_thread(ui_thread_function, bb);
    std::thread data_thread(data_thread_function, bb , fname, colnames);
    // TODO: join ui_thread
    UI_thread.join();
    // TODO: "Once the user has entered 'Exit', the main thread will
    // "send a signal through the message queue to stop the data thread"
    
    // TODO: join data thread
    data_thread.join();

    // CLEANUP: delete members on heap
    delete bb;
}

/************** Helper Function Definitions **************/

// function to parse column names into vector
// input: _colnames (vector of column name strings), _opt_input(input from optarg for -c)
void parse_column_names(vector<string>& _colnames, const string& _opt_input) {
    stringstream sstream(_opt_input);
    string tmp;
    while (sstream >> tmp) {
        _colnames.push_back(tmp);
    }
}

// function to append "text" to end of file
// input: filename (name of file), text (text to add to file), first_input (whether or not this is the first input of the file)
void write_to_file(const string& _filename, const string& _text, bool _first_input) {
    // based on https://stackoverflow.com/questions/26084885/appending-to-a-file-with-ofstream
    // open file to either append or clear file
    ofstream ofile;
    if (_first_input)
        ofile.open(_filename);
    else
        ofile.open(_filename, ofstream::app);
    if (!ofile.is_open()) {
        perror("ofstream open");
        exit(-1);
    }

    // sleep for a random period up to 5 seconds
    usleep(rand() % 5000);

    // add data to csv
    ofile << _text;

    // close file
    ofile.close();
}