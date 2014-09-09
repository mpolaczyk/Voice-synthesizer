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

bool mergeSounds(string& word, const string& voice, set<string>& sounds)
{
    string latestKnown = string(1, word[0]);
    string cmd = "./wavmerge ";
    cout << endl;
	
    for ( int i = 0 ; i < word.length(); i++)
    {
		string current = latestKnown;
		bool currentExists = sounds.find(current) != sounds.end();
		
		string next = "";
		bool nextExists = false;
	
		if(i+1 < word.length())
		{
			next = latestKnown + string(1, word[i+1]);
			nextExists = sounds.find(next) != sounds.end();
		}
		
		if(!currentExists) { return false; }
		else if(currentExists && nextExists)
		{		
			latestKnown = next;
			continue;
		}
		else if(currentExists && !nextExists)
		{
			cmd = cmd + " " + voice + "/" + current + ".wav";
			latestKnown = string(1,word[i+1]); // at will throw exception when [] returns \0
		}
		 
    }
	cout << endl << cmd << endl;
    system(cmd.c_str()); // UNSECURE: Directory traversal / Command Injection
    cmd = "mv merge.wav " + voice + "/" + word + ".wav";
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
    
	string cmd = "find " + voice + " -type f -name \"*.wav\" -exec basename {} .wav \\; > " + voice + "/sounds";     
	system(cmd.c_str()); // UNSECURE: Directory traversal / Command Injection
        
	ifstream soundsFile(voice + "/sounds", ifstream::in);
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
		cmd = cmd + "aplay " + voice + "/" + word + ".wav; "; 
	}
	system(cmd.c_str()); // UNSECURE: Directory traversal / Command Injection
	coutEnd();
}

int main(int argc, char *argv[])
{
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