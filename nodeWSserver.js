const WebSocket = require('ws');

const wss = new WebSocket.Server({ port: 9876 });

function broadcast(message) {
  wss.clients.forEach(function each(client) {
    if (client.readyState === WebSocket.OPEN) {
      client.send(message);
    }
  });
}



wss.on('connection', function connection(ws) {
    broadcast("### Client connected");
    ws.on('message', function sendmessage(message){
      message = JSON.parse(message);
      console.log(message);

      if (message.type == "message"){
        broadcast(message.data.username + ": " + message.data.message);
      }

    });
});
