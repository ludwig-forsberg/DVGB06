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

var connection;

function setServeUI(){
    let p1_serve = true;

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
    }else{
        canvas_p1_serve.style.visibility = "Hidden";
        canvas_p2_serve.style.visibility = "Visible";
    }
}

function setUI(){
    text_p1_points.innerHTML = "" + p1_points;
    text_p1_set.innerHTML = "" + p1_set;
    text_p2_points.innerHTML = "" + p2_points;
    text_p2_set.innerHTML = "" + p2_set;
    setServeUI();

    if(original_side){
        score_container_p1.classList.remove("blue")
        score_container_p1.classList.add("red");
        score_container_p2.classList.remove("red")
        score_container_p2.classList.add("blue");
    }else{
        score_container_p1.classList.remove("red")
        score_container_p1.classList.add("blue");
        score_container_p2.classList.remove("blue")
        score_container_p2.classList.add("red");
    }
}

function reset(){
    p1_points = p1_set = p2_points = p2_set = 0;
    setUI();
}

function p1_add_point(){
    p1_points++;
    if(p1_points >= 11 && (p1_points - p2_points) >= 2){
        p1_points = p2_points = 0;
        p1_set++;
    }
    setUI();
}


function p2_add_point(){
    p2_points++;
    if(p2_points >= 11 && (p2_points - p1_points) >= 2){
        p2_points = p1_points = 0;
        p2_set++;
    }
    setUI();
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
}

function swap_serve(){
    original_serve = !original_serve;
    setServeUI();
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
    drawServeIndicator(canvas_p1_serve);
    drawServeIndicator(canvas_p2_serve);
}
