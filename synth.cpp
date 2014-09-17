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
inline void coutH() 
{ 
	cout << "Usage: " << endl <<
		"--voice <voice> --text \"<text>\"" << endl <<
		"Where <voice> is directory name with wav files and <text> is a text to process." << endl <<
		"--listv" << endl <<
		"To list available voices";
}

bool modeSay = false;
bool modeListVoices = false;

string selfPath;

string get_selfPath()
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
    string cmd = selfPath + "wavmerge ";
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
				cmd = cmd + " " + selfPath + voice + "/" + current + ".wav";
				current = "";
				next = "";
			}		
		}
		 
    }
	cout << endl << cmd << endl;
    system(cmd.c_str()); // UNSECURE: Directory traversal / Command Injection
	// merge.wav lands in working directory, move this to bin
    cmd = "mv merge.wav " + selfPath + voice + "/" + word + ".wav";
    system(cmd.c_str()); // UNSECURE: Directory traversal / Command Injection
    return true;
}

bool tryParseParams(string& voice, string& text, int& argc, char *argv[])
{
	// Check params
	if(argc == 2)
	{
		modeListVoices = (strcmp(argv[1], "--listv") == 0);
		return modeListVoices;
	}
	else if(argc == 5)
	{
		modeSay = (strcmp(argv[1], "--voice") == 0 || strcmp(argv[3], "--text") == 0);
		if (modeSay)
		{
			voice = argv[2];
			text = argv[4];
			return true;
		}
	}
  
	return false;
}

bool listVoices()
{
	// Get all voices (directories)
	string cmd = "find " + selfPath + " -type d > " + selfPath + "/voices";
	system(cmd.c_str()); // UNSECURE: Directory traversal / Command Injection
	
	ifstream voicesFile(selfPath + "/voices", ifstream::in);
	if (voicesFile.is_open())
	{
		// Each line has sound name, store it
		string line;
		int x = selfPath.length();
		while (getline(voicesFile, line))
		{
			if(line.length() > 0)
			{
				cout << line.substr(x, line.length() - x) << endl;
			}
		}
		voicesFile.close();
	}
}

bool loadVoice(string& voice, set<string>& sounds)
{
    // Get all waves from directory and save to file
    coutBegin("Looking for sounds of: '" + voice + "'");
    
	string cmd = "find " + selfPath + voice + " -type f -name \"*.wav\" -exec basename {} .wav \\; > " + selfPath + voice + "/sounds";     
	system(cmd.c_str()); // UNSECURE: Directory traversal / Command Injection
        
	ifstream soundsFile(selfPath + voice + "/sounds", ifstream::in);
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
		cmd = cmd + "aplay " + selfPath + voice + "/" + word + ".wav; "; 
	}
	system(cmd.c_str()); // UNSECURE: Directory traversal / Command Injection
	coutEnd();
}


	
int main(int argc, char *argv[])
{
	selfPath = get_selfPath();
	
	string voice;
	string text;
	set<string> sounds;
	set<string> voices;
	vector<string> words;
		
	try
	{
		if(!tryParseParams(voice, text, argc, argv)) 
		{ 
			coutH();
			return EX_USAGE;
		}

		if(modeListVoices)
		{
			listVoices();
		}
		else if(modeSay)
		{
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
  	}
	catch(exception& e)
	{
		cout << endl << endl << "Exception: " << e.what() << endl;
	}
	
	return EX_OK;           
}