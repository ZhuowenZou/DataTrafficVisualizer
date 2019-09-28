/****************************************************************************
	Zhuowen Zou
	12/28/2018
	Data Traffic Visualizer

File Name:      datParser.c
Description:    
	This program reads in .dat file containing log entries and parse it 
	into a dataframe (temp.csv, which can be opened by excel) and a stats 
	file (stats, which currently only include the adjusted start time and end 
	time of the log entries)
****************************************************************************/
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <string>
#include <fstream>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <cstddef>
#include <sstream>

using namespace std;


vector<int> sizeList;	// List of send sizes
vector<long long> tickList;	// List of ticks
vector<string> timeList;	// List of timeStrings


#define NUMTICK		20			// Number of ticks for each log entry
#define MAXPOINT	2000	// Max # of points on the graph
#define FULL_ACCESS	0000777
/*---------------------------------------------------------------------------
Function Name:
	reduceTime
Description:
	Gives a string that represents the time timeStr - (diff miliseconds)
Input:
	timeStr: the time String that represents that time to subtract from 
	diff: the amount of miliseconds to go back to 
Output:                       
	string that represents the time timeStr - (diff miliseconds)
Result:
	None
---------------------------------------------------------------------------*/
string reduceTime(string & timeStr, long long diff){
	
	//12/18/2018 06:11:20.185
	
	// Extract time info
	char *ptr = &timeStr[0];
	int month = atoi(ptr);
	
	ptr = strstr(ptr, "/");
	int day = atoi(++ptr);
	
	ptr = strstr(ptr, "/");
	int year = atoi(++ptr);
	
	ptr = strstr(ptr, " ");
	int hour = atoi(++ptr);
	
	ptr = strstr(ptr, ":");
	int min = atoi(++ptr);
	
	ptr = strstr(ptr, ":");
	double milisec = 1000*(atof(++ptr));
	
	// Subtract diff miliseconds from time
	if (milisec < diff){
		if (min == 0){
			if (hour == 0){
				if (day == 1){
					if (month == 1){
						year --;
						month = 12;
					}
					else 
						month--;
					switch (month){
						case 1:case 3:case 5:case 7:case 8:case 10: case 12:
							day = 31; break;
						case 4:case 6:case 9:case 11:
							day = 30; break;
						case 2: 
							day = (year%4)?28:29; break;
					}
				}
				else 
					day--;
				hour = 23;
			}
			else 
				hour--;
			min = 59;
			}
		else 
			min--;
		milisec = milisec + 60000 - diff;
	}
	else 
		milisec -= diff;
	
	// Parse back to String
	string yearS = to_string(year);
	string monthS = ((month < 10)?"0":"") + to_string(month); 
	string dayS   = ((day < 10)?"0":"") + to_string(day); 
	string hourS  = ((hour < 10)?"0":"") + to_string(hour); 
	string minS   = ((min < 10)?"0":"") + to_string(min); 

	milisec /= 1000;
	string secS = (milisec<10)?"0":"";
	secS = secS.append(to_string(milisec));
	secS = secS.substr(0, 6);
	
	string timeStamp = 
		"" + monthS + "/" + dayS + "/" + yearS + " " +
		hourS + ":" + minS + ":" + secS;

	return timeStamp;
}

/*---------------------------------------------------------------------------
Function Name:
	extractTime
Description:
	Extract the time Strnig from the log hearder
	 [12/18/2018 06:02:09.213 pid=15804 tid=3071653584]
Input:
	info: String of the form 
		[12/18/2018 06:02:09.213 pid=XXXXX tid=XXXXXXXXXX]
Output:                       
	time String, of the form
		12/18/2018 06:02:09.213
Result:
	None
---------------------------------------------------------------------------*/
string extractTime(string & info){
	
	char *ptr1 = &info[0];
	ptr1 = strstr(ptr1,"[");
	ptr1++;
	
	char *ptr2 = &info[0];
	ptr2 = strstr(ptr2,"pid");
	ptr2--;
	
	return info.substr(ptr1 - &info[0], ptr2 - ptr1);
	
}

/*---------------------------------------------------------------------------
Function Name:
	outputAdditionalInfo
Description:
	write the adjusted start time and finish time into output stream
Input:
	output: ptr to the output ostream
Output:                       
	None
Result:
	the adjusted start and finish time will be written into the stream
---------------------------------------------------------------------------*/
void outputAdditionalInfo(ostream *output){
	
	//the offset of the real start time from log-entry start time 
	long long diff = tickList[NUMTICK-1];
	
	(*output) << "Start,Finish" << endl;
	string start = extractTime(timeList[0]);
	(*output) << reduceTime(start, diff) <<","<< extractTime(timeList[timeList.size()-1]) <<endl; 
}

/*---------------------------------------------------------------------------
Function Name:
	loadFromDat
Description:
	Reads in log file and store (tick, send size) into two separate vectors.
	It also extract all the log headers for poosibly future use.
Input:
	input: ptr to istream
Output:                       
	None
Result:
	the vectors sizeList, tickList, timeList will be filled 
---------------------------------------------------------------------------*/
void loadFromDat( istream * input ){
	
	const char * log = "log:";
	const char * tick = "tick:";
	const int tickLength = 5;	//	length of the string tick
	const char * size = "send size:";
	const int sizeLength = 10;	//	length of the string size
	
	string startTime, endTime;
	
	const int BUFSIZE = 2560;
	char buffer[BUFSIZE] = {'\0'};
	char * endptr = buffer;
	char * ptrToTick, * ptrToSize, * ptrToLog;
	
	while (input->peek() != EOF){
		
		ptrToTick = ptrToSize = ptrToLog = buffer;
		input->read(endptr, sizeof(char) * (BUFSIZE-1 - (endptr - buffer)));
		ptrToLog = strstr(buffer, log);
		if ((*ptrToLog) != '\0'){
			
			// record log entry header 
			char * left = strstr(ptrToLog, "["), * right = strstr(ptrToLog, "]");
			timeList.push_back( string(left, right-left+1) );
			
			// record tick, size...
			for (int i = 0; i < NUMTICK; i++){
				
				ptrToTick = strstr( ptrToTick, tick );
				tickList.push_back( atoll(ptrToTick + tickLength) );
				ptrToTick ++;
				
				ptrToSize = strstr( ptrToSize, size );
				sizeList.push_back( atoi(ptrToSize + sizeLength) );
				ptrToSize ++;
				
			}
		}
		else break;
		
		ptrToLog = strstr(ptrToLog + 1, log);
		
		// if exists next entry, move the next entry to front
		if ((*ptrToLog) != '\0'){
			endptr = buffer;
			// Works like memmove + memset but I was too afraid to use them
			while ( (*ptrToLog) != '\0'){
				*endptr++ = *ptrToLog;
				*ptrToLog++ = '\0';
			}
		} 
		else 
			break;
	}
}

/*---------------------------------------------------------------------------
Function Name:
	simplifyAndOutput
Description:
	compress the send size into a fixed time interval and write 
	the dataframe (relative tick ( = tick - first tick ), total send size) 
	into stream
Input:
	output: output stream
Output:                       
	None
Result:
	the dataframe (relative tick, total send size) will be written into 
	the output stream
---------------------------------------------------------------------------*/
void simplifyAndOutput(ostream * output){

	// convert absolute tick to relative tick
	long long offset = tickList[0];
	for (int i = 0; i < (int)tickList.size(); i++)
		tickList[i] -= offset;

	// determine the time interval, compressLength, for grouping the send size
	long long totalLength = tickList[tickList.size()-1] + 100; // 100 for padding 
	long long compressLength = totalLength/MAXPOINT;
	if (compressLength == 0) compressLength = 1;

	//Header 
	(*output)<<"Time Stamp, Send Size Total"<<endl;
	
	//int index = 0; 
	int sum = 0; 
	long long threshold = compressLength;
	for (int i = 0; i < (int)tickList.size(); i++){
		if (tickList[i] < threshold){
			sum += sizeList[i];
		}
		else {
			while ( (threshold < tickList[i]) ){
				(*output) << threshold << "," << sum <<endl;
				sum = 0;
				threshold += compressLength;
			}
		}
	}
	if (sum != 0){
		(*output) << threshold <<","<< sum << endl;
	}
}

/*---------------------------------------------------------------------------
Function Name:
	main
Description:
	the driver for the whole file
Input:
	argv[1]: the path to the .dat file to parse
Output:                       
	EXIT_SUCCESS upon complete parsing
	EXIT_Failure upon failed parsing
Result:
	The .dat file will be parsed into:
	temp.csv: the dataframe containing timeStamp (relative tick) 
	 * and total send size within the time interval.
	stats.csv: the dataframe containing the adjusted start time 
	 * and end time.
---------------------------------------------------------------------------*/
int main(int argc, char **argv)
{

	// argument check
	if (argc != 2){
		// Usage for Windos system
		cerr<< "Please drag your .dat file onto the executable." << endl;
		// Usage for Linux system
		cerr<< "Please drag your .dat file on to the executable." << endl;
		return EXIT_FAILURE;
	}
	
	// Store .dat into vectors.
	cout<< "Converting .dat file..." <<endl;
	ifstream input;
	input.open(argv[1]);
	if (!input.fail()){
		loadFromDat(&input);
		input.close();
	}
	else {
		cerr << "Error: Failed to open " << argv[1] << "." <<endl;
		return EXIT_FAILURE;
	}
	
	// Simplify and output data
	cout<< "Simplifying data..." <<endl;
	ofstream output;
	output.open("temp.csv");
	if (!output.fail()){
		simplifyAndOutput(&output);
		output.close();
		chmod("temp.csv", FULL_ACCESS);
	}
	else {
		cerr << "Error: Failed to create temp.csv." <<endl;
		return EXIT_FAILURE;
	}
	
	// generate graph through R?
	cout<< "Outputing Additional Info" <<endl;
	output.open("stats.csv");
	if (!output.fail()){
		outputAdditionalInfo(&output);
		output.close();
		chmod("stats.csv", FULL_ACCESS);
	}
	else {
		cerr << "Error: Failed to create stats.csv." <<endl;
		return EXIT_FAILURE;
	}
	
	cout<< "Done." <<endl;
	return EXIT_SUCCESS;
}
