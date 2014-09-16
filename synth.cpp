// Copyright (c) 2014 Marcin Polaczyk http://mpolaczyk.pl

#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <set>
#include <string.h>
#include <vector>
#include <algorithm>
#include <iterator>
#include <sstream>
#include <sysexits.h>
#include <exception>



using namespace std;

inline void coutBegin(const string& text) { cout << "=== " << text << " ===" << endl; }
inline void coutEnd() { cout << endl; }
inline void coutE(const string& text) { cout << ' ' << text; }
inline void coutH() { cout << "Format: --voice <voice> --text \"<text>\"" << endl << "Where <voice> is directory name with wav files and <text> is a text to process." << endl; }

string selfpath;

string get_selfpath()
{
    char buff[1024];
    ssize_t len = ::readlink("/proc/self/exe", buff, sizeof(buff)-1);
    if (len != -1) {
      buff[len] = '\0';
	  string ans(buff);
      return ans.substr(0, ans.length() - 5);
    } else {
     /* handle error condition */
    }
}

void getUTF8Char(const int& i, const string& word, string& outChar, int& outLen)
{
	int num = 0;
	int position = 7;
	while ((word[i] & (1 << position)) == (1 << position)) 
	{ 
		num++; 
		position--;
	}
	
	outLen = (num == 0 ? 1 : num);
	outChar = word.substr(i, outLen);
}

bool mergeSounds(string& word, const string& voice, set<string>& sounds)
{
    string cmd = selfpath + "wavmerge ";
    cout << endl;
	
	// Current combination
	string current = "";
	// Next combination
	string next = "";
	
	int di;
	
    for ( int i = 0 ; i <= word.length(); i+=di)
    {
		di = 0;
		
		// If current combination is empty, get first not processed character
		if (current == "")
		{
			getUTF8Char(i, word, current, di);
		}
			
		// Check if current combination exists
		if(current != "" && sounds.find(current) != sounds.end())
		{
			// If we have more characters
			int din = 0;
			if(i+di < word.length())
			{
				// Build next combination
				string x;
				getUTF8Char(i+di, word, x, din);
				next = current + x;
			}
			else
			{
				next = "";
			}
		
			// Check if next combination exists
			if(din > 0 && sounds.find(next) != sounds.end())
			{
				// Save next combination to current and continue
				current = next;
				di += din;
			}
			else
			{
				// Send combination to merge and continue
				cmd = cmd + " " + selfpath + voice + "/" + current + ".wav";
				current = "";
				next = "";
			}		
		}
		 
    }
	cout << endl << cmd << endl;
    system(cmd.c_str()); // UNSECURE: Directory traversal / Command Injection
	// merge.wav lands in working directory, move this to bin
    cmd = "mv merge.wav " + selfpath + voice + "/" + word + ".wav";
    system(cmd.c_str()); // UNSECURE: Directory traversal / Command Injection
    return true;
}

bool tryParseParams(string& voice, string& text, int& argc, char *argv[])
{
	// Check params
	if(argc != 5 || strcmp(argv[1], "--voice") != 0 || strcmp(argv[3], "--text") != 0) { return false; }
  
	// Configure
	voice = argv[2];
	text = argv[4];
	return true;
}

bool loadVoice(string& voice, set<string>& sounds)
{
    // Get all waves from directory and save to file
    coutBegin("Looking for sounds of: '" + voice + "'");
    
	string cmd = "find " + selfpath + voice + " -type f -name \"*.wav\" -exec basename {} .wav \\; > " + selfpath + voice + "/sounds";     
	system(cmd.c_str()); // UNSECURE: Directory traversal / Command Injection
        
	ifstream soundsFile(selfpath + voice + "/sounds", ifstream::in);
	if (soundsFile.is_open())
	{
		// Each line has sound name, store it
		string line;
		while (getline(soundsFile, line))
		{
			coutE(line);
			sounds.insert(line);
		}
		soundsFile.close();
	}
	
	bool isEmpty = sounds.empty();
	if(isEmpty) { coutE("No sounds available..."); }
    
    coutEnd();
	
	return !isEmpty;
}

void splitTextIntoWords(string& text, vector<string>& words)
{
	// Split text into words
	coutBegin("Parsing text into words");
	
	istringstream iss(text);
	string word;
	while (iss >> word) 
	{ 
		coutE(word);
		words.push_back(word);
	}
	
	coutEnd();
}

bool createWords(string& voice, vector<string>& words, set<string>& sounds)
{
	// Create wave files for words
	coutBegin("Creating word sounds");
	for(auto word : words)
	{
		if (sounds.find(word) == sounds.end())
		{
			coutE(word);
			if (!mergeSounds(word, voice, sounds)) { return false; }
			sounds.insert(word);  
		}
	}          
	coutEnd();
	return true;
}

void playWords(string& voice, vector<string>& words)
{
	// Play words
	coutBegin("Playing words");
	string cmd;
	for(auto word : words)
	{
		coutE(word);
		cmd = cmd + "aplay " + selfpath + voice + "/" + word + ".wav; "; 
	}
	system(cmd.c_str()); // UNSECURE: Directory traversal / Command Injection
	coutEnd();
}

int main(int argc, char *argv[])
{
	selfpath = get_selfpath();
	cout << selfpath << endl;
	
	string voice;
	string text;
	set<string> sounds;
	vector<string> words;
		
	try
	{
		if(!tryParseParams(voice, text, argc, argv)) 
		{ 
			coutH();
			return EX_USAGE;
		}
  
		if(!loadVoice(voice, sounds))
		{
			return EX_OK;
		}
  
		splitTextIntoWords(text, words);
		
		if(!createWords(voice, words, sounds))
		{
			return EX_OK;
		}
  
		playWords(voice, words);
  	}
	catch(exception& e)
	{
		cout << endl << endl << "Exception: " << e.what() << endl;
	}
	
	return EX_OK;           
}