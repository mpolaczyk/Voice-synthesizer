#!/usr/bin/env python
import web
from subprocess import Popen, PIPE, call

urls = (
    '/sayhello/(.*)', 'sayhello',
    '/listv', 'listv'
    )

app = web.application(urls, globals())


class Synth(object):
    synthPath = "./bin/synth"
    
    @staticmethod
    def Say(voice, text):
        p = Popen([Synth.synthPath, "--voice", voice, "--text", text], stdin=PIPE, stdout=PIPE, stderr=PIPE)
        output, err = p.communicate()
        return [output, p.returncode, err]
    
    @staticmethod
    def GetVoices():
        p = Popen([Synth.synthPath, '--listv'], stdin=PIPE, stdout=PIPE, stderr=PIPE)
        output, err = p.communicate()
        return [output, p.returncode, err]


class sayhello:
    def GET(self, voice):
        return Synth.Say(voice, "witaj")

class listv:
    def GET(self):
        return Synth.GetVoices()

if __name__ == "__main__":
    app.run()
