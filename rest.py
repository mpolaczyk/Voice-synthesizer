#!/usr/bin/env python
import web

from subprocess import Popen, PIPE, call

urls = (
    '/sayhello/(.*)', 'sayhello',
    '/listv', 'listv',
	'/say', 'say'
    )

app = web.application(urls, globals())

render = web.template.render('templates/')

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
        return filter(None, output.split('\n'))


class sayhello:
    def GET(self, voice):
        return Synth.Say(voice, "witaj")

class listv:
    def GET(self):
        return Synth.GetVoices()

sayForm = web.form.Form(
    web.form.Dropdown('voice', Synth.GetVoices()),
    web.form.Textbox('text'),
    web.form.Button('Say')
)

class say:
    def GET(self):
        f = sayForm()
        return render.say(content=f)

    def POST(self):
        f = sayForm()
        if not f.validates(): 
            return render.formtest(f)
        else:
            Synth.Say(f.d.voice, f.d.text)
            return f.d.voice + " just said: " + f.d.text

application = app.wsgifunc()

if __name__ == "__main__":
    app.run()
