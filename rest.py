#!/usr/bin/env python
import web
from subprocess import call

urls = ('/sayhello/(.*)', 'sayhello')

app = web.application(urls, globals())


class Synth(object):
    synthPath = "./bin/synth"
    
    @staticmethod
    def Say(voice, text):
        call([Synth.synthPath, "--voice", voice, "--text", text])


class sayhello:
    def GET(self, voice):
        return Synth.Say(voice, "witaj")

if __name__ == "__main__":
    app.run()
