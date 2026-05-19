async function fetchData() {
const r = await fetch('/data');
const j = await r.json();

document.getElementById('temp').innerText = j.temp;
document.getElementById('setp').innerText = j.set;
document.getElementById('output').innerText = j.output;
document.getElementById('rpmDisplay').innerText = j.rpm;
}

async function setPoint(){
const v = document.getElementById('newSet').value;
await fetch('/set', {
method:'POST',
headers:{'Content-Type':'application/x-www-form-urlencoded'},
body:'set='+v
});
fetchData();
}

async function setRpm(){
const v = document.getElementById('newRpm').value;
await fetch(`/setMotor?rpm=${v}`);
fetchData();
}

function toggleFullScreen(){
const cam=document.getElementById('cameraBox');
if(!document.fullscreenElement) cam.requestFullscreen();
else document.exitFullscreen();
}

setInterval(fetchData, 1000);
fetchData();