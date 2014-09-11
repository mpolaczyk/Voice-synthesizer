#!/usr/bin/env python
import web
from subprocess import call

urls = ('/sayhello/(.*)', 'sayhello')

app = web.application(urls, globals())

class sayhello:
    def GET(self, voice):
        return call(["./bin/synth", "--voice", "./bin/" + voice, "--text", "o as d"])

if __name__ == "__main__":
    app.run()