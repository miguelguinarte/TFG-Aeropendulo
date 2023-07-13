
var on = 0;
var stopped = 1;
const select = document.getElementById("select");
const enviar = document.getElementById("enviar");
const KBox = document.getElementById("K");
const NbarBox = document.getElementById("Nbar");
const KText = document.getElementById("Ktext");
const NbarText = document.getElementById("Nbartext");
const KpBox = document.getElementById("Kp");
const KiBox = document.getElementById("Ki");
const KdBox = document.getElementById("Kd");
const KpText = document.getElementById("KpText");
const KiText = document.getElementById("KiText");
const KdText = document.getElementById("KdText");
const refPrecisa = document.getElementById("refPrecisa");


function ocultarTunnig(){
    enviar.style.display = "none";
    KBox.style.display = "none";
    NbarBox.style.display = "none";
    KText.style.display = "none";
    NbarText.style.display = "none";
    KpBox.style.display = "none";
    KiBox.style.display = "none";
    KdBox.style.display = "none";
    KpText.style.display = "none";
    KiText.style.display = "none";
    KdText.style.display = "none";
}

function mostrarTunningEE(){
    KpText.style.display = "none";
    KiText.style.display = "none";
    KdText.style.display = "none";
    KpBox.style.display = "none";
    KiBox.style.display = "none";
    KdBox.style.display = "none";
    enviar.style.display = "block";
    KBox.style.display = "block";
    NbarBox.style.display = "block";
    KText.style.display = "block";
    NbarText.style.display = "block";
}

function mostrarTunningPID(){
    enviar.style.display = "none";
    KBox.style.display = "none";
    NbarBox.style.display = "none";
    KText.style.display = "none";
    NbarText.style.display = "none";
    KpBox.style.display = "block";
    KiBox.style.display = "block";
    KdBox.style.display = "block";
    KpText.style.display = "block";
    KiText.style.display = "block";
    KdText.style.display = "block";
}


//document.onload = function(){
//    ocultarTunnig();
//    document.getElementById("StopButton").style.display = "none";
//    initialize();
//}

function initialize(){
    connectWebSocket();
    drawChart();
    select.value = "EE"
    document.getElementById("StopButton").style.display = "none";
    mostrarTunningEE();
    refPrecisa.value = 0;
}

document.addEventListener("DOMContentLoaded",function(){
    initialize();
});



var chart;
var data = [];
var options;
var counter = 0;
var socket;
var pendulo = document.getElementById("pendulo");



function drawChart() {
    var ctx = document.getElementById('grafica').getContext('2d');
    chart = new Chart(ctx, {
        type: 'line',
        data: {
            labels: [],
            datasets: [
                {
                    label: 'Ref',
                    data: [],
                    borderColor: 'blue',
                    backgroundColor: 'rgba(0, 123, 255, 0.4)',
                    tension: 0,
                    pointRadius: 0,
                    Animation: false
                },
                {
                    label: 'Yk',
                    data: [],
                    borderColor: 'green',
                    backgroundColor: 'rgba(0, 255, 0, 0.4)',
                    tension: 0,
                    pointRadius: 0,
                    Animation: false
                }
            ]
        },
        options: {
            animation: false,
            responsive: false,
            maintainAspectRatio: false,
            plugins: {
                title: {
                    display: true,
                    text: 'Respuesta en tiempo real'
                }
            },
            scales: {
                x: {
                    title: {
                        display: true,
                        text: 'Tiempo',
                    },
                    ticks:{
                        maxRotation: 0,
                        maxTicksLimit: 10
                    }
                }
            }
        }
    });
}





function connectWebSocket() {
    socket = new WebSocket('ws://192.168.4.1:81/');

    socket.onopen = function() {
        console.log('Conexión establecida');
    };

    socket.onmessage = function(event) {
        var message = event.data.split(',');
        var ref = parseFloat(message[0]);
        var yk = parseFloat(message[1]);
        counter += 0.15;
        
        if (chart.data.labels.length >= 70) {
            chart.data.labels.shift();
            chart.data.datasets.forEach(function(dataset) {
                dataset.data.shift();
            });
        }

        chart.data.labels.push(counter.toFixed(2)); //.toFixed(2)
        chart.data.datasets[0].data.push(ref);
        chart.data.datasets[1].data.push(yk);

        chart.update();

        pendulo.style.transform = "rotate(" + yk + "deg)";
    };

    socket.onclose = function() {
        console.log('Conexión cerrada');
    };
}






var slider = document.getElementById("myRange");

// Update the current slider value (each time you drag the slider handle)
slider.oninput = function() {
  //output.innerHTML = this.value;
  refPrecisa.value = this.value;
}

slider.onchange = function(){
  socket.send("ref =" + String(this.value));
  console.log(this.value);      
}





select.onchange = function(){
    if(on ==1 && stopped == 0){
        if (select.value == "EE"){
            socket.send("EE");
            mostrarTunningEE();
        }
        else if (select.value == "Lead"){
            socket.send("lead");
            ocultarTunnig();
        }
        else if (select.value == "PID"){
            socket.send("PID");
            mostrarTunningPID();
        }
    }
    else{
        if (select.value == "EE"){
            mostrarTunningEE();
        }
        else if (select.value == "Lead"){
            ocultarTunnig();
        }
        else if (select.value == "PID"){
            mostrarTunningPID();
        }
    }
}


enviar.onclick = function(){
    console.log("funciona");
    socket.send("K =" + String(KBox.value));
    socket.send("Nbar =" + String(NbarBox.value));
}

var startButton = document.getElementById("StartButton");
var stopButton = document.getElementById("StopButton");



startButton.onclick = function(){
    if(on == 0){
        on = 1;
        stopped = 0;
        startButton.style.display = "none"
        stopButton.style.display = "block"
        if (select.value == "EE"){
            socket.send("onEE");
            mostrarTunningEE();
        }
        else if (select.value == "Lead"){
            socket.send("onLead");
            ocultarTunnig();
        }
        else if (select.value == "PID"){
            socket.send("PID");
            mostrarTunningPID();
        }
    }
    if(on == 1){
        stopped = 0;
        if (select.value == "EE"){
            socket.send("initEE");
            mostrarTunningEE();
        }
        else if (select.value == "Lead"){
            socket.send("initLead");
            ocultarTunnig();
        }    
        startButton.style.display = "none"
        stopButton.style.display = "block"
        
        document.getElementById("select").style.display = "block";
        document.getElementById("ControlMode").style.display = "block";
    }
}


stopButton.onclick = function(){
    socket.send("stop");
    stopped = 1;
    stopButton.style.display = "none"
    startButton.style.display = "block"
    ocultarTunnig();
    document.getElementById("ControlMode").style.display = "none";
}


KpBox.onchange = function(){
    socket.send("Kp =" + String(KpBox.value));
}

KiBox.onchange = function(){
    socket.send("Ki =" + String(KiBox.value));
}

KdBox.onchange = function(){
    socket.send("Kd =" + String(KdBox.value));
}

refPrecisa.onchange = function(){
    console.log("Cagaste");
    slider.value = this.value;
    socket.send("ref =" + String(this.value));
}