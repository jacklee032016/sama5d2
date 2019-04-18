
from flask_restful import Resource

systemCfg = {
    'ip': '192.168.168.183',
    'mask': '255.255.255.0',
    'gateway': '192.168.168.1',
}


class System(Resource):
    def get(self):
        return systemCfg
        
    def post(self):
        return '', 201

    def delete(self, todo_id):
        # abort_if_todo_doesnt_exist(todo_id)
        # del TODOS[todo_id]
        return '', 204