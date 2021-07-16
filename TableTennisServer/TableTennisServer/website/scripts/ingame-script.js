var p1_points = 0;
var p1_set = 0;
var p2_points = 0;
var p2_set = 0;

var original_side = true;
var original_serve = true;

var text_p1_points;
var text_p1_set;
var score_container_p1;
var input_p1_name;
var canvas_p1_serve;
var text_p2_points;
var text_p2_set;
var score_container_p2;
var input_p2_name;
var canvas_p2_serve;

var speaker_toggle;
var use_recorded_audio_toggle;
var popup;
var popup_container;
var popup_header;
var popup_text;


var small_screen_p1_score_container;
var small_screen_p1_set;
var small_screen_p1_points;
var small_screen_p1_name;
var small_screen_p1_serve;
var small_screen_p2_score_container;
var small_screen_p2_set;
var small_screen_p2_points;
var small_screen_p2_name;
var small_screen_p2_serve;

var span_console;

var connection;
var p1_serve = true;

var recordedAudio={};
var requiredAudio=[
  "player1",
  "player2",
  "serves",
  "setpoint",
  "0",
  "1",
  "2",
  "3",
  "4",
  "5",
  "6",
  "7",
  "8",
  "9",
  "10",
  "11"
];

function setServeUI(){
    p1_serve = true;

    if((original_serve && !original_side) || (!original_serve && original_side))
        p1_serve = !p1_serve;
    
    if((p1_set+p2_set)%2 == 1)
        p1_serve = !p1_serve;
    

    if(p1_points >= 10 && p2_points >= 10){
        if((p1_points+p2_points)%2 >= 1)
            p1_serve = !p1_serve;
    }
    else{
        if(((p1_points+p2_points)/2)%2 >= 1)
            p1_serve = !p1_serve;

    }
    

    if(p1_serve){
        canvas_p1_serve.style.visibility = "Visible";
        canvas_p2_serve.style.visibility = "Hidden";

        small_screen_p1_serve.style.visibility = "Visible";
        small_screen_p2_serve.style.visibility = "Hidden";
    }else{
        canvas_p1_serve.style.visibility = "Hidden";
        canvas_p2_serve.style.visibility = "Visible";

        small_screen_p1_serve.style.visibility = "Hidden";
        small_screen_p2_serve.style.visibility = "Visible";
    }
}


function setUI(){
    small_screen_p1_points.innerHTML = text_p1_points.innerHTML = "" + p1_points;
    small_screen_p1_set.innerHTML = text_p1_set.innerHTML = "" + p1_set;
    small_screen_p2_points.innerHTML = text_p2_points.innerHTML = "" + p2_points;
    small_screen_p2_set.innerHTML = text_p2_set.innerHTML = "" + p2_set;
    setServeUI();

    if(original_side){
        score_container_p1.classList.remove("blue")
        score_container_p1.classList.add("red");
        score_container_p2.classList.remove("red")
        score_container_p2.classList.add("blue");

        small_screen_p1_score_container.classList.remove("blue")
        small_screen_p1_score_container.classList.add("red");
        small_screen_p2_score_container.classList.remove("red")
        small_screen_p2_score_container.classList.add("blue");
    }else{
        score_container_p1.classList.remove("red")
        score_container_p1.classList.add("blue");
        score_container_p2.classList.remove("blue")
        score_container_p2.classList.add("red");

        small_screen_p1_score_container.classList.remove("red")
        small_screen_p1_score_container.classList.add("blue");
        small_screen_p2_score_container.classList.remove("blue")
        small_screen_p2_score_container.classList.add("red");
    }

    if(speaker_toggle.checked){
        if(use_recorded_audio_toggle.checked){
            var audioList = [];

            function addAudioToList(str){
                let audio = recordedAudio[str];
                if(audio == undefined){
                    console.log("Audio not found: ", str);
                }else{
                    audioList.push(audio);
                }
            }

            if((p1_serve && original_side) || (!p1_serve && !original_side)){
                if(original_side){
                    addAudioToList("" + p1_points);
                    addAudioToList("" + p2_points);
                }else{
                    addAudioToList("" + p2_points);
                    addAudioToList("" + p1_points);
                }
                addAudioToList("player1");
            }else{
                if(original_side){
                    addAudioToList("" + p2_points);
                    addAudioToList("" + p1_points);
                }else{
                    addAudioToList("" + p1_points);
                    addAudioToList("" + p2_points);
                }
                addAudioToList("player2");
            }
            addAudioToList("serves");
            if((p1_points >= 10 || p2_points >= 10) && p1_points != p2_points)
                addAudioToList("setpoint");

            function playAudio(){
                let audio = audioList.shift();
                audio.play();
                if(audioList.length > 0)
                {
                    audio.onended = ()=>{
                        playAudio();
                    }
                }
            }
            playAudio();
        }else{
            var audio_message_string = "";

            let name = "";
    
            if(p1_serve){
                name = (input_p1_name.value==""?input_p1_name.placeholder:input_p1_name.value);
                audio_message_string += p1_points + " " + p2_points;
            }else{
                name = (input_p2_name.value==""?input_p2_name.placeholder:input_p2_name.value);
                audio_message_string += p2_points + " " + p1_points;
    
            }
    
            audio_message_string += name + " serves";
    
            if((p1_points >= 10 || p2_points >= 10) && p1_points != p2_points)
                audio_message_string += "setpoint";
    
            var audio_message = new SpeechSynthesisUtterance(audio_message_string);
            window.speechSynthesis.speak(audio_message);
        }
    }
}

function reset(){
    p1_points = p1_set = p2_points = p2_set = 0;
    setUI();
    sendData();
}

function p1_add_point(){
    p1_points++;
    if(p1_points >= 11 && (p1_points - p2_points) >= 2){
        p1_points = p2_points = 0;
        p1_set++;
    }
    setUI();
    sendData();
}


function p2_add_point(){
    p2_points++;
    if(p2_points >= 11 && (p2_points - p1_points) >= 2){
        p2_points = p1_points = 0;
        p2_set++;
    }
    setUI();
    sendData();
}

function p1_sub_point(){
    p1_points--;
    if(p1_points < 0)
        p1_points = 0;
    if(p2_points >= 11 && (p2_points - p1_points) >= 2){
        p2_points = p1_points = 0;
        p2_set++;
    }
    setUI();
    sendData();
}


function p2_sub_point(){
    p2_points--;
    if(p2_points < 0)
        p2_points = 0;
    if(p1_points >= 11 && (p1_points - p2_points) >= 2){
        p1_points = p2_points = 0;
        p1_set++;
    }
    setUI();
    sendData();
}

function swap_serve(){
    original_serve = !original_serve;
    setServeUI();
    sendData();
}

function swap_sides(){
    original_side = !original_side;

    let temp = input_p1_name.value;
    input_p1_name.value = input_p2_name.value;
    input_p2_name.value = temp;


    temp = input_p1_name.placeholder;
    input_p1_name.placeholder = input_p2_name.placeholder;
    input_p2_name.placeholder = temp;



    temp = p1_points;
    p1_points = p2_points;
    p2_points = temp;

    temp = p1_set;
    p1_set = p2_set;
    p2_set = temp;


    
    setUI();
    sendData();
}

function drawServeIndicator(canvas){
    let ctx = canvas.getContext("2d");

    var x = 100,
    y = 50,
    innerRadius = 30,
    outerRadius = 44,
    // Radius of the entire circle.
    radius = 44;

    ctx.beginPath();
    ctx.globalAlpha = .5;
    ctx.arc(x, y+5, radius, 0, 2 * Math.PI);
    ctx.fillStyle = "black";
    ctx.fill();

    ctx.globalAlpha = 1;
    let gradient = ctx.createRadialGradient(x, y, innerRadius, x, y, outerRadius);
    gradient.addColorStop(0, 'white');
    gradient.addColorStop(1, 'gray');
    ctx.beginPath();
    ctx.arc(x, y, radius, 0, 2 * Math.PI);

    ctx.fillStyle = gradient;
    ctx.fill();

    
}
var original_side = true;
var original_serve = true;
function sendData(){
    connection.send(JSON.stringify({
        "player1":{
            "name": (input_p1_name.value==""?input_p1_name.placeholder:input_p1_name.value),
            "points": p1_points,
            "set": p1_set
        },
        "player2":{
            "name": (input_p2_name.value==""?input_p2_name.placeholder:input_p2_name.value),
            "points": p2_points,
            "set": p2_set
        },
        "original_side": original_side,
        "original_serve": original_serve,
        "screen_height": window.screen.height,
        "screen_width": window.screen.width
    }));

}

window.onload = ()=>{
    text_p1_points = document.getElementById("text_p1_points");
    text_p1_set = document.getElementById("text_p1_set");
    score_container_p1 = document.getElementById("score_container_p1");
    input_p1_name = document.getElementById("input_p1_name");
    canvas_p1_serve = document.getElementById("canvas_p1_serve");
    text_p2_points = document.getElementById("text_p2_points");
    text_p2_set = document.getElementById("text_p2_set");
    score_container_p2 = document.getElementById("score_container_p2");
    input_p2_name = document.getElementById("input_p2_name");
    canvas_p2_serve = document.getElementById("canvas_p2_serve");
    span_console = document.getElementById("span_console");
    drawServeIndicator(canvas_p1_serve);
    drawServeIndicator(canvas_p2_serve);
    
    speaker_toggle = document.getElementById("speaker_toggle");
    use_recorded_audio_toggle = document.getElementById("use_recorded_audio_toggle");
    popup = document.getElementById("popup");
    popup_container = document.getElementById("popup_container");
    popup_header = document.getElementById("popup_header");
    popup_text = document.getElementById("popup_text");

    small_screen_p1_score_container     = document.getElementById("small_screen_p1_score_container");
    small_screen_p1_set                 = document.getElementById("small_screen_p1_set");
    small_screen_p1_points              = document.getElementById("small_screen_p1_points");
    small_screen_p1_name                = document.getElementById("small_screen_p1_name");
    small_screen_p1_serve               = document.getElementById("small_screen_p1_serve");
    small_screen_p2_score_container     = document.getElementById("small_screen_p2_score_container");
    small_screen_p2_set                 = document.getElementById("small_screen_p2_set");
    small_screen_p2_points              = document.getElementById("small_screen_p2_points");
    small_screen_p2_name                = document.getElementById("small_screen_p2_name");
    small_screen_p2_serve               = document.getElementById("small_screen_p2_serve");


    var wsproto = (location.protocol === 'https:') ? 'wss:' : 'ws:';
    connection = new WebSocket(wsproto + '//' + window.location.host + '/websocket');
    //connection.send(i);

    connection.onmessage = function(e){
        if(e.data != ""){
            let data = JSON.parse(e.data);

            input_p1_name.value = data.player1.name;
            p1_points = data.player1.points;
            p1_set = data.player1.set;

            input_p2_name.value = data.player2.name;
            p2_points = data.player2.points;
            p2_set = data.player2.set;

            small_screen_p1_name.innerHTML = data.player1.name;
            small_screen_p2_name.innerHTML = data.player2.name;
    

            original_side = data.original_side;
            original_serve = data.original_serve;

            setUI();
        }
    }
    connection.onerror = function(error){
        console.error(error);
        connection.close();
    }

    input_p1_name.addEventListener('input', (event) => {
        sendData();
    });

    input_p2_name.addEventListener('input', (event) => {
        sendData();
    });
}
