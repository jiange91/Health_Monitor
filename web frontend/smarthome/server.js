//for mqtt
var sys = require('sys');
var net = require('net');
var mqtt    = require('mqtt');

//set up server
const express = require("express");
const app = express();
var http = require('http').createServer(app);
app.use(express.static("public"));

//start socket.io connection (websocket)
var io = require('socket.io')(http);

// create an mqtt client object and connect to the mqtt broker
var mqtt_client  = mqtt.connect('mqtt://mqtt_summer_research:unnc2020@10.160.1.47');
mqtt_client.subscribe("cushion");

// When a client connects, we note it in the console
io.on('connection', function (socket) {
    console.log('A client is connected!');
});

// listen to messages coming from the mqtt broker
mqtt_client.on('message', function (topic, payload, packet) {
    console.log(topic+'='+payload);
    io.emit('cushion',{'topic':String(topic),
                       'payload':String(payload)});
});

http.listen(8000, () => {
  console.log('listening on *:8000');
});