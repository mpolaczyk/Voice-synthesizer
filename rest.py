#!/usr/bin/env python
import web
from subprocess import call

urls = ('/sayhello/(.*)', 'sayhello')

app = web.application(urls, globals())

class sayhello:
    def GET(self, voice):
        return call(["./synth", "--voice", voice, "--text", "witaj"])

if __name__ == "__main__":
    app.run()