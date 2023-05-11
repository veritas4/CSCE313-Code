#include <fstream>
#include <iostream>
#include <thread>
#include <sys/time.h>
#include <sys/wait.h>

#include "BoundedBuffer.h"
#include "common.h"
#include "Histogram.h"
#include "HistogramCollection.h"
#include "FIFORequestChannel.h"

// ecgno to use for datamsgs
#define EGCNO 1

using namespace std;


void patient_thread_function (BoundedBuffer* reqBuf, int patientNo, int requestNo) {
    // functionality of the patient threads
    //cout << "Requesting " << requestNo << " data points from patient " << patientNo << endl;
    for (int i = 0; i < requestNo; i++) {
        datamsg data_msg(patientNo, i * .004, EGCNO);
        reqBuf->push(&data_msg, sizeof (datamsg));
    }
}

void file_thread_function (BoundedBuffer* reqBuf, string filename, int m, __int64_t file_len) {
    // functionality of the file thread    
    string fname = "received/" + filename;

    FILE* outputFile = fopen(fname.c_str(), "w");
    fseek(outputFile, file_len, SEEK_SET);
    fclose(outputFile);

    int remainder = file_len;
    int offset = 0;
    int length = 0;
    //cout << "thread function called" << endl;

    while (remainder > 0) {
        //cout << remainder << endl;
        length = min(m, remainder);
        offset = file_len - remainder;
        filemsg file_msg(offset,length);

        char * tempBuff = new char[sizeof(filemsg) + filename.size() + 1];
        memcpy(tempBuff, &file_msg, sizeof(filemsg));
        strcpy(tempBuff + sizeof(filemsg), filename.c_str());

        reqBuf->push(tempBuff, sizeof(filemsg) + filename.size() + 1);
        //cout << reqBuf->size() << endl;
        remainder = remainder - length;    
        delete[] tempBuff;
    }   
    //cout << reqBuf->size() << endl;
}

void worker_thread_function (BoundedBuffer* reqBuf,BoundedBuffer* respBuf, FIFORequestChannel* workerChan, int size) {
    // functionality of the worker threads
    //cout << "Worker thread" << endl;
    char requestmsg[1024];


    while (true) {
        // Retrieve the datamsg from request buffer
        reqBuf->pop(requestmsg, 1024);
        MESSAGE_TYPE m = *((MESSAGE_TYPE*) requestmsg);

        if (m == QUIT_MSG) {
            workerChan->cwrite(requestmsg, sizeof(QUIT_MSG));
            delete workerChan;
            //cout << "quitting worker" << endl;
            return;
        }
        else if (m == DATA_MSG) {
            //char requestmsg_buf[MAX_MESSAGE] = {};
            datamsg msg(0,0,0);
            //reqBuf->pop(&msg,sizeof(datamsg)); 
            memcpy(&msg, requestmsg, sizeof (datamsg));

            // Send datamsg to the server through the channel
            workerChan->cwrite(&msg, sizeof(datamsg));
            double ecg = 0;
            workerChan->cread(&ecg, sizeof (double));

            std::pair<int,double> histogramPair(msg.person,ecg);
            // Push the response to the response buffer
            respBuf->push(&histogramPair, sizeof(histogramPair));
            //cout << "Pushing: " << respBuf->size() << endl;
        }
        else if (m == FILE_MSG) {
            char *receiveBuffer = new char[size];
            filemsg * fm = reinterpret_cast<filemsg *>(requestmsg);
            string filename = (char *) (fm + 1);

            int len = filename.size() + sizeof (filemsg) + 1;

            filename = "received/" + filename;

            workerChan->cwrite(requestmsg, len); 
            workerChan->cread(receiveBuffer, size);

            FILE* filePointer = fopen(filename.c_str(), "r+");
            if (filePointer == nullptr) {
                cerr << "Error opening the file" << endl;
            }
            fseek(filePointer, fm->offset, SEEK_SET);
            fwrite(receiveBuffer, 1, fm->length, filePointer); // modified
            fclose(filePointer);
            delete[] receiveBuffer;
        }
    }   
  
}

void histogram_thread_function (BoundedBuffer* respBuf, HistogramCollection* hc) {
    // functionality of the histogram threads
    while (true) {

        std::pair <int,double> patientData;
        respBuf->pop(&patientData,sizeof(std::pair<int,double>));
        //cout << "Popping: " << respBuf->size() << endl;

        if (patientData.first == -1) {
            return;
        }
        else {
            hc->update(patientData.first,patientData.second);
            //cout << patientData.first << " " << patientData.second << endl;
        }
    }
    //cout << "Exiting histogram thread function" << endl;
}


int main (int argc, char* argv[]) {
    int n = 1000;	// default number of requests per "patient"
    int p = 10;		// number of patients [1,15]
    int w = 100;	// default number of worker threads
	int h = 20;		// default number of histogram threads
    int b = 20;		// default capacity of the request buffer (should be changed)
	int m = MAX_MESSAGE;	// default capacity of the message buffer
	string f = "";	// name of file to be transferred
    
    // read arguments
    int opt;
	while ((opt = getopt(argc, argv, "n:p:w:h:b:m:f:")) != -1) {
		switch (opt) {
			case 'n':
				n = atoi(optarg);
                break;
			case 'p':
				p = atoi(optarg);
                break;
			case 'w':
				w = atoi(optarg);
                break;
			case 'h':
				h = atoi(optarg);
				break;
			case 'b':
				b = atoi(optarg);
                break;
			case 'm':
				m = atoi(optarg);
                break;
			case 'f':
				f = optarg;
                break;
		}
	}
    
	// fork and exec the server
    int pid = fork();
    if (pid == 0) {
        execl("./server", "./server", "-m", (char*) to_string(m).c_str(), nullptr);
    }
    
	// initialize overhead (including the control channel)
	FIFORequestChannel* chan = new FIFORequestChannel("control", FIFORequestChannel::CLIENT_SIDE);
    BoundedBuffer request_buffer(b);
    BoundedBuffer response_buffer(b);
	HistogramCollection hc;

    // making histograms and adding to collection
    for (int i = 0; i < p; i++) {
        Histogram* h = new Histogram(10, -2.0, 2.0);
        hc.add(h);
    }
	
	// record start time
    struct timeval start, end;
    gettimeofday(&start, 0);

    vector<thread> file_thread;
    vector<thread> patient_threads;
    vector<thread> histogram_threads;
    /* create all threads here */
    if (f != "") {
        filemsg fm(0,0);
        int len = sizeof(filemsg) + (f.size()) + 1;
        char *buf = new char[len];

        memcpy(buf, &fm, sizeof(filemsg));
        strcpy(buf + sizeof(filemsg), f.c_str());

        chan->cwrite(buf, len);
        __int64_t file_len;
        chan->cread(&file_len, sizeof (file_len));  

        file_thread.push_back(thread(file_thread_function,&request_buffer, f, m, file_len));
        delete[] buf;
    }
    else {
        // Create the patient threads
        for (int i = 1; i <= p; i++) {
            patient_threads.push_back(thread(patient_thread_function, &request_buffer, i, n));
        }
        // Create the Histogram threads
        for (int i = 1; i <= h; i++) {
            histogram_threads.push_back(thread(histogram_thread_function,&response_buffer, &hc));
        }
    }
    // Create the worker threads

    vector<thread> worker_threads;
    //vector<FIFORequestChannel*> worker_channels;
    for (int i = 1; i <= w; i++) {
        // Create a new channel for every worker
        MESSAGE_TYPE new_channel = NEWCHANNEL_MSG;
        chan->cwrite(&new_channel, sizeof(MESSAGE_TYPE));
        char channel_name[1024];
        chan->cread(channel_name, 1024);

        // Add new worker channel to the vector of worker channels
        FIFORequestChannel *newchan = new FIFORequestChannel(channel_name, FIFORequestChannel::CLIENT_SIDE);
       // worker_channels.push_back(newchan);
        worker_threads.push_back(thread(worker_thread_function, &request_buffer, &response_buffer, newchan, m));
    }
	/* join all threads here */
    if (f == "") {
        for (int i = 0; i < p; i++) {
            patient_threads.at(i).join();
        }
    }

    if(f != ""){
        file_thread.at(0).join();
    }


    for (int i = 0; i < w; i++) {
        MESSAGE_TYPE quit_worker = QUIT_MSG;
        request_buffer.push(&quit_worker, sizeof (QUIT_MSG));
    }

    for (int i = 0; i < w; i++) {
        worker_threads.at(i).join();
    }

    if (f == "") {
        for (int i = 0; i < h; i++) {
            std::pair<int,double> quitPair(-1,0.0);
            response_buffer.push(&quitPair, sizeof (quitPair));
        }   
        for (int i = 0; i < h; i++) {
            histogram_threads.at(i).join();
        }
    }


	// record end time
    gettimeofday(&end, 0);

    // print the results
	if (f == "") {
		hc.print();
	}
    int secs = ((1e6*end.tv_sec - 1e6*start.tv_sec) + (end.tv_usec - start.tv_usec)) / ((int) 1e6);
    int usecs = (int) ((1e6*end.tv_sec - 1e6*start.tv_sec) + (end.tv_usec - start.tv_usec)) % ((int) 1e6);
    cout << "Took " << secs << " seconds and " << usecs << " micro seconds" << endl;


    MESSAGE_TYPE q = QUIT_MSG;

    chan->cwrite ((char *) &q, sizeof (MESSAGE_TYPE));
    cout << "All Done!" << endl;
    delete chan;

	// wait for server to exit
	wait(nullptr);
}
