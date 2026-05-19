<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8"/>
<meta name="viewport" content="width=device-width,initial-scale=1.0"/>
<title>Straw-Freelament Dashboard</title>

<style>
body {
    font-family: Arial;
    margin: 0;
    background: #0d1117;
    color: white;
}

.container {
    max-width: 400px;
    margin: 80px auto;
    background: rgba(255,255,255,0.05);
    padding: 30px;
    border-radius: 12px;
}

input {
    width: 100%;
    padding: 12px;
    margin: 10px 0;
}

button, input[type=submit] {
    width: 100%;
    padding: 12px;
    background: #00d1ff;
    border: none;
    cursor: pointer;
}
.hidden { display:none; }
</style>
</head>

<body>

<div class="container">

<!-- LOGIN -->
<div id="loginForm">
<h2>Login</h2>

<form id="login">
<input type="text" name="username" placeholder="Username" required>
<input type="text" name="password" placeholder="Password" required>
<input type="submit" value="Login">
</form>

<button onclick="showRegister()">Register</button>
<button onclick="showReset()">Forgot Password</button>
</div>

<!-- REGISTER -->
<div id="registerForm" class="hidden">
<h2>Register</h2>

<form id="register">
<input type="text" name="username" placeholder="Username" required>
<input type="text" name="password" placeholder="Password" required>
<input type="submit" value="Register">
</form>

<button onclick="showLogin()">Back</button>
</div>

<!-- RESET -->
<div id="resetForm" class="hidden">
<h2>Reset Password</h2>

<form id="reset">
<input type="text" name="username" placeholder="Username" required>
<input type="text" name="password" placeholder="New Password" required>
<input type="submit" value="Reset">
</form>

<button onclick="showLogin()">Back</button>
</div>

</div>

<script>
function showRegister(){
document.getElementById("loginForm").classList.add("hidden");
document.getElementById("registerForm").classList.remove("hidden");
}

function showLogin(){
document.getElementById("registerForm").classList.add("hidden");
document.getElementById("resetForm").classList.add("hidden");
document.getElementById("loginForm").classList.remove("hidden");
}

function showReset(){
document.getElementById("loginForm").classList.add("hidden");
document.getElementById("resetForm").classList.remove("hidden");
}

// LOGIN
document.getElementById("login").addEventListener("submit", e=>{
e.preventDefault();

const data = new FormData(e.target);
data.append("action","login");

fetch("auth_api.php", { method:"POST", body:data })
.then(r=>r.json())
.then(res=>{
if(res.success){
window.location.href="dashboard.php";
}else alert(res.error);
});
});

// REGISTER
document.getElementById("register").addEventListener("submit", e=>{
e.preventDefault();

const data = new FormData(e.target);
data.append("action","register");

fetch("auth_api.php", { method:"POST", body:data })
.then(r=>r.json())
.then(res=>{
if(res.success) showLogin();
else alert(res.error);
});
});

// RESET
document.getElementById("reset").addEventListener("submit", e=>{
e.preventDefault();

const data = new FormData(e.target);
data.append("action","reset");

fetch("auth_api.php", { method:"POST", body:data })
.then(r=>r.json())
.then(res=>{
if(res.success){
alert("Password reset successful");
showLogin();
}else alert(res.error);
});
});
</script>

</body>
</html>