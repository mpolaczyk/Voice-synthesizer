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

using namespace std;

bool createSound(const string& name)
{
  // TODO
  // system(...); // UNSECURE: Directory traversal
  return false;
}

inline void coutBegin(const string& text) { cout << "=== " << text << " ===" << endl; }
inline void coutEnd() { cout << endl; }
inline void coutE(const string& text) { cout << ' ' << text; }
inline void coutH() { cout << "Format: -voice <voice> -text \"<text>\"" << endl << "Where <voice> is directory name with wav files and <text> is a text to process." << endl; }

int main(int argc, char *argv[])
{
  // Check params
  if(argc != 5 || strcmp(argv[1], "--voice") != 0 || strcmp(argv[3], "--text") != 0) { coutH(); return EX_USAGE; }
  
  // Configure
  string voice = argv[2];
  string text = argv[4];
        
  // Get all waves from directory and save to file
  coutBegin("Looking for sounds of: '" + voice + "'");
  set<string> sounds;
  {
    string cmd = "find " + voice + " -type f -name \"*.wav\" -exec basename {} .wav \\; > " + voice + "/sounds";     
    system(cmd.c_str()); // UNSECURE: Directory traversal
 
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
    if(sounds.empty()) { coutE("No sounds available..."); coutEnd(); return EX_OK; }              
  }
  coutEnd();
  
  // Split text into words
  coutBegin("Parsing text into words");
  vector<string> words;
  {
    istringstream iss(text);
    string word;
    while (iss >> word) 
    { 
      coutE(word);
      words.push_back(word);
    }
  }                                                                                                        
  coutEnd();
  
  // Create wave files for words
  coutBegin("Creating word sounds");
  for(auto word : words)
  {
    if (sounds.find(word) == sounds.end())
    {
      coutE(word);
      createSound(word);
      sounds.insert(word);  
    }
  }              
  coutEnd();
  
  // Play words
  coutBegin("Playing words");
  for(auto word : words)
  {
    coutE(word);
    string cmd = "aplay " + voice + "\\" + word + ".wav&";
    //system(cmd.c_str()); // UNSECURE: Directory traversal
  }
  coutEnd();                                                                            
  
  return EX_OK;                                            
}