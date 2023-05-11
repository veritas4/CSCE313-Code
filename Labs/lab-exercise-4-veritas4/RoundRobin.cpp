#include "RoundRobin.h"

/*
This is a constructor for RoundRobin Scheduler, you should use the extractProcessInfo function first
to load process information to process_info and then sort process by arrival time;

Also initialize time_quantum
*/
RoundRobin::RoundRobin(string file, int time_quantum)
{
	extractProcessInfo(file);
	set_time_quantum(time_quantum);
}

// Schedule tasks based on RoundRobin Rule
// the jobs are put in the order the arrived
// Make sure you print out the information like we put in the document
void RoundRobin::schedule_tasks()
{
	int system_time = 0;
	size_t vector_index = 0;
	for (auto i : processVec) {
		if (i.get_arrival_time() == 0) {
			processQ.push(i);
			vector_index++;
		}
	}
	
	while(!processQ.empty()) {
		
		int run_time = min(time_quantum,processQ.front().get_remaining_time());
		for (int i = 0; i < run_time; i++) {
			print(system_time,processQ.front().getPid(),processQ.front().is_Completed());
			system_time++;
			while (vector_index < processVec.size() && processVec.at(vector_index).get_arrival_time() <= system_time) {
				processQ.push(processVec.at(vector_index));
				vector_index++;
			}
		}
		
		Process p = processQ.front();
		p.Run(run_time);
		processQ.pop();

		if (!p.is_Completed()) {
			processQ.push(p);
		}
		else {
			print(system_time,p.getPid(),p.is_Completed());
		}
		if (vector_index < processVec.size() && processQ.empty()) {
			while (system_time < processVec.at(vector_index).get_arrival_time()) {
				print(system_time,-1,false);
				system_time++;
			}
			processQ.push(processVec.at(vector_index));
			vector_index++;
		}
	}
}

/***************************
ALL FUNCTIONS UNDER THIS LINE ARE COMPLETED FOR YOU
You can modify them if you'd like, though :)
***************************/

// Default constructor
RoundRobin::RoundRobin()
{
	time_quantum = 0;
}

// Time quantum setter
void RoundRobin::set_time_quantum(int quantum)
{
	this->time_quantum = quantum;
}

// Time quantum getter
int RoundRobin::get_time_quantum()
{
	return time_quantum;
}

// Print function for outputting system time as part of the schedule tasks function
void RoundRobin::print(int system_time, int pid, bool isComplete)
{
	string s_pid = pid == -1 ? "NOP" : to_string(pid);
	cout << "System Time [" << system_time << "].........Process[PID=" << s_pid << "] ";
	if (isComplete)
		cout << "finished its job!" << endl;
	else
		cout << "is Running" << endl;
}

// Read a process file to extract process information
// All content goes to proces_info vector
void RoundRobin::extractProcessInfo(string file)
{
	// open file
	ifstream processFile(file);
	if (!processFile.is_open())
	{
		perror("could not open file");
		exit(1);
	}

	// read contents and populate process_info vector
	string curr_line, temp_num;
	int curr_pid, curr_arrival_time, curr_burst_time;
	while (getline(processFile, curr_line))
	{
		// use string stream to seperate by comma
		stringstream ss(curr_line);
		getline(ss, temp_num, ',');
		curr_pid = stoi(temp_num);
		getline(ss, temp_num, ',');
		curr_arrival_time = stoi(temp_num);
		getline(ss, temp_num, ',');
		curr_burst_time = stoi(temp_num);
		shared_ptr<Process> p(new Process(curr_pid, curr_arrival_time, curr_burst_time));

		processVec.push_back(*p);
	}

	// close file
	processFile.close();
}