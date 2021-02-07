var health_score = 0;
var slider = document.getElementById("myCanvas");
var output = document.getElementById("responseVal");
output.innerHTML = slider.value; // Display the default slider value

// Update the current slider value (each time you drag the slider handle)
slider.oninput = function() {
  output.innerHTML = this.value;
}

var socket = io.connect('http://localhost:8000');
socket.on('cushion', function(message) {
    message_object = JSON.parse(message.payload);
    console.log('The server has a message for you: ' + message_object.health_score);
    health_score = message_object.health_score;   /*可能后面会更改*/
    output.innerHTML = health_score;
    
    var c=document.getElementById("myCanvas");
    var ctx=c.getContext("2d");
    ctx.clearRect(0,10,500,80);
    ctx.fillStyle="#F2C735";
    ctx.fillRect(0,10,health_score,80);  //后面可以改掉
})

