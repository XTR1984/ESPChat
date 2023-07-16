# -*- coding: utf-8 -*-
import cherrypy
import io

messages =  [{"id":0, "t": "t1", "a":"guest 1", "m": "hello" },
             {"id":1, "t": "t2", "a":"guest 2", "m": "hello2 " },
             {"id":2, "t": "t3", "a":"guest 3", "m": "hello3 " },
]

messages = []
class Root:
    def __init__(self):
        self.last_id = -1
    @cherrypy.expose
    @cherrypy.tools.json_in()
    def add_msg(self):
        data = cherrypy.request.json
        self.last_id +=1
        messages.append({"id":self.last_id, "t": data["time"], "a": data["author"], "m": data["msg"] })
        return "[]";

    @cherrypy.expose
    @cherrypy.tools.json_in()
    @cherrypy.tools.json_out()
    def get_msgs(self,from_id):
#        data = cherrypy.request.json
#        print(data)
        from_id = int(from_id)
        if from_id<=self.last_id:
            return messages[from_id:]
        return "[]"

    @cherrypy.expose
    def index(self):
        page = io.open("chat.html","r",encoding="utf-8")
        return page

#cherrypy.config.update("server.conf")
cherrypy.quickstart(Root(),'/',"app.conf")