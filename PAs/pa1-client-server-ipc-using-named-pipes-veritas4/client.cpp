/*
	Original author of the starter code
    Tanzir Ahmed
    Department of Computer Science & Engineering
    Texas A&M University
    Date: 2/8/20
	
	Please include your Name, UIN, and the date below
	Name: Juan Carrasco
	UIN: 830005537	
	Date: 02/02/2023
*/
#include "common.h"
#include "FIFORequestChannel.h"
#include <chrono>
using namespace std;


int main (int argc, char *argv[]) {
	int opt;
	int p = -1;
	double t = -1;
	int e = -1;
	int m = MAX_MESSAGE;
	bool nc = false;
	vector<FIFORequestChannel*> myChannels;
	
	string filename = "";
	while ((opt = getopt(argc, argv, "p:t:e:f:m:c")) != -1) {
		switch (opt) {
			case 'p':
				p = atoi (optarg);
				break;
			case 't':
				t = atof (optarg);
				break;
			case 'e':
				e = atoi (optarg);
				break;
			case 'f':
				filename = optarg;
				break;
			case 'm':
				m = atoi(optarg);
				break;
			case 'c':
				nc = true;
				break;
		}
	}
	
	//Create the server process
	int child_process;
	pid_t pid = fork();
	if (pid < 0) {
		cout << "Fork failed, terminating program" << endl;
		return -1;
	}
	else if (pid == 0) {
		char* args[] = {(char*) "./server",(char*) "-m",(char*) to_string(m).c_str(),NULL};
		execvp(args[0],args);
	}
	
    FIFORequestChannel control_chan("control", FIFORequestChannel::CLIENT_SIDE);
	myChannels.push_back(&control_chan);

	if (nc) {										// if new channel request, create a new channel
		MESSAGE_TYPE new_channel = NEWCHANNEL_MSG;	// newchannel msg
		control_chan.cwrite(&new_channel,sizeof(MESSAGE_TYPE));	// write message request to the server

		char* channel_name = new char [11];		// to store the name of new channel
		control_chan.cread(channel_name,11);	// read name of channel from server

		FIFORequestChannel *newchan = new FIFORequestChannel(channel_name, FIFORequestChannel::CLIENT_SIDE); // create a new, dynamic channel
		myChannels.push_back(newchan);			// store new channel into our list of channels

		delete[] channel_name;					
	}
	FIFORequestChannel chan = *(myChannels.back());	// use the last channel created

	// example data point request	
	if (p != -1 && t != 1 && e != -1) {

		char buf[MAX_MESSAGE]; // 256
		datamsg x(p,t,e);	// construct a data message
		
		memcpy(buf, &x, sizeof(datamsg));
		chan.cwrite(buf, sizeof(datamsg)); // question
		double reply;
		chan.cread(&reply, sizeof(double)); //answer
		cout << "For person " << p << ", at time " << t << ", the value of ecg " << e << " is " << reply << endl;
	}
	else if (p != -1) {		// Copy the first 1000 lines of data 
		char buf[MAX_MESSAGE];
		string new_filename = "received/x1.csv"; 
		ofstream outputFile(new_filename);

		for (double i = 0; i < 4; i += .004) {
			datamsg msg(p,i,1);	// construct a datamsg with patient#,time, and ecgno

			memcpy(buf, &msg, sizeof(datamsg));
			chan.cwrite(buf, sizeof(datamsg)); // question
			double reply;
			chan.cread(&reply, sizeof(double)); //answer
			outputFile << i << "," << reply << ",";
			//cout << "For person " << p << ", at time " << t << ", the value of ecg " << e << " is " << reply << endl;

			msg.ecgno = 2; // set ecgno to 2;
			memcpy(buf, &msg, sizeof(datamsg));
			chan.cwrite(buf, sizeof(datamsg)); // question
			chan.cread(&reply, sizeof(double)); //answer
			(abs(i - 3.996) < 0.001? outputFile << reply : outputFile << reply << "\n");

			//cout << "For person " << p << ", at time " << t << ", the value of ecg " << e << " is " << reply << endl;
		}
	}

	//double time = 0;
	//auto start = std::chrono::high_resolution_clock::now();
	if (filename != "") {
		// sending a non-sense message, you need to change this
		filemsg fm(0, 0);
		string fname = filename;
		int len = sizeof(filemsg) + (fname.size() + 1);
		char* buf2 = new char[len];
		memcpy(buf2, &fm, sizeof(filemsg));
		strcpy(buf2 + sizeof(filemsg), fname.c_str());

		chan.cwrite(buf2, len);  					// Write the file msg request to the server

		__int64_t file_length;
		chan.cread(&file_length,sizeof(__int64_t)); // read the length of the file from channel
		//cout << "File length is: " << file_length << endl;

		// Now we proceed to request the file by sending a sequence of filemsgs until we copy entire file
		ofstream outputFile("received/" + filename);
		char* response_buff = new char[m]; 			// create response buffer with specified size m

		int remainder = file_length;
		int offset = 0;
		int length = 0;

		while(remainder > 0) {						// Until we have read all of the data
			length = min(m,remainder);
			offset = file_length - remainder;
			filemsg fmsg(offset,length); 			// construct a filemsg with offset and length
			memcpy(buf2, &fmsg, sizeof(filemsg)); 	// write file msg into the buffer
			
			
			chan.cwrite(buf2,len); 					// write the file msg to server from the buffer using the channel
			chan.cread(response_buff,length);	    // read reponse from the server into response_buffer
			
			outputFile.write(response_buff,length);	// write the data from resp buffer into output file
			remainder = remainder - m;
		}
		delete [] response_buff;					// delete dynamically allocated memory
		delete[] buf2;
	}
	//std::chrono::duration<double> diff = stop - start;
	//time += diff.count();
	//cout << "The time taken to transfer file is " << time << endl;


	// closing the channels    
    MESSAGE_TYPE msg = QUIT_MSG;
	if (nc) {
		myChannels[1]->cwrite(&msg,sizeof(MESSAGE_TYPE));	// close the channel by sending a QUIT_MSG
		delete myChannels[1];								// delete the new channel that was dynamic
	}
	myChannels[0]->cwrite(&msg,sizeof(MESSAGE_TYPE));	
	wait(&child_process);
}