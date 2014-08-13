# -*- coding: windows-1250 -*-
import pyaudio
import audioop
import sys
import wave
import math
import os.path
from sys import byteorder
from array import array

# Define recording parameters
CHUNK = 1024
FORMAT = pyaudio.paInt16
RATE = 44100
RECORD_SECONDS = 2
FRAME_MAX_VALUE = 2 ** 15 - 1
NORMALIZE_MINUS_ONE_dB = 10 ** (-1.0 / 20)
averageNoiseRms = 0

def CheckNoiseLevel(RATE, CHUNK, stream):
    "Returns RMS noise level. Requires bit rate and chunk size."
    noiseRmsData = []
    for i in range(0, int(RATE / CHUNK * 2)):
        data = stream.read(CHUNK)
        noiseRmsData.append(audioop.rms(data, 2))
    return math.ceil(sum(noiseRmsData) / len(noiseRmsData)) * 2
	
def Record(RATE, CHUNK, FORMAT, stream, noiseLevel, filename):
    "Records sound to the file."
    frames = array('h')
    for i in range(0, int(RATE / CHUNK * 2)):
        data = array('h', stream.read(CHUNK))
        if byteorder == 'big':
            data.byteswap()
        if audioop.rms(data, 2) >= noiseLevel:
            frames.extend(data)
    frames = Normalize(frames)
    wf = wave.open(filename, 'wb')
    wf.setnchannels(1)
    wf.setsampwidth(p.get_sample_size(FORMAT))
    wf.setframerate(RATE)
    wf.writeframes(frames)
    wf.close()	
    return

def Normalize(data):
    "Function normalizes samples to maximum possible value below overdrive level."
    factor = float(NORMALIZE_MINUS_ONE_dB * FRAME_MAX_VALUE)/max(abs(i) for i in data)
    normalizedData = array('h')
    for i in data:
        normalizedData.append(int(i*factor))
    return normalizedData
	
# Define recording stream
p = pyaudio.PyAudio()
recStream = p.open(format=FORMAT, channels=1, rate=RATE, input=True, output=True, frames_per_buffer=CHUNK) 

# Measure noise level
print('=== Recording noise please be quiet ===') 
averageNoiseRms = CheckNoiseLevel(RATE, CHUNK, recStream)
print('Average noise RMS level is: ' + str(averageNoiseRms))

# Generate list of sounds
print('=== Genering sound names ===')
consonants = ['b', 'c', 'd', 'f', 'g', 'h', 'j', 'k', 'l', 'm', 'n', 'p', 'r', 's', 't', 'w', 'z', 'sz', 'cz', 'dz', 'rz']
vowels = ['a', 'e', 'i', 'o', 'u', 'y']
# diacriticalConsonants = ['ć', 'ł', 'ń', 'ś', 'ź', 'ż', 'dź', 'dż']
# diacriticalVowels = ['ą', 'ę', 'ó']
openSylabbles = []
for consonant in consonants:
    for vowel in vowels:
	    openSylabbles.append(consonant + vowel)
# soundNames = diacriticalConsonants + consonants + vowels + diacriticalVowels + openSylabbles
soundNames = vowels + consonants + openSylabbles
print(soundNames)

# Main loop
print('=== Main loop started ===')
for sound in soundNames:
    filename = 'out/' + sound + '.wav'
    if os.path.exists(filename): continue
    else:
        print('Say: ' + sound);
        Record(RATE, CHUNK, FORMAT, recStream, averageNoiseRms, filename)
  
print('=== Done ===')

# Clean
recStream.stop_stream()
recStream.close()
p.terminate()



