<?php
session_start();
require "db.php";

header("Content-Type: application/json");

if ($_SERVER["REQUEST_METHOD"] === "POST" && isset($_POST['action'])) {

    $action = $_POST['action'];

    // ── LOGIN ─────────────────────────────
    if ($action === "login") {
        $user = $_POST['username'];
        $pass = $_POST['password'];

        $stmt = $conn->prepare("SELECT * FROM users WHERE username = ?");
        $stmt->bind_param("s", $user);
        $stmt->execute();
        $result = $stmt->get_result();

        if ($row = $result->fetch_assoc()) {
            if (password_verify($pass, $row['password'])) {
                $_SESSION['authenticated'] = true;
                $_SESSION['username'] = $user;
                echo json_encode(["success" => true]);
            } else {
                echo json_encode(["success" => false, "error" => "Invalid password"]);
            }
        } else {
            echo json_encode(["success" => false, "error" => "User not found"]);
        }
        exit();
    }

    // ── REGISTER ──────────────────────────
    if ($action === "register") {
        $user = $_POST['username'];
        $pass = password_hash($_POST['password'], PASSWORD_DEFAULT);

        $stmt = $conn->prepare("INSERT INTO users (username, password) VALUES (?, ?)");
        $stmt->bind_param("ss", $user, $pass);

        if ($stmt->execute()) {
            echo json_encode(["success" => true]);
        } else {
            echo json_encode(["success" => false, "error" => "Username already exists"]);
        }
        exit();
    }

    // ── RESET PASSWORD ────────────────────
    if ($action === "reset") {
        $user = $_POST['username'];
        $newPass = password_hash($_POST['password'], PASSWORD_DEFAULT);

        $stmt = $conn->prepare("UPDATE users SET password = ? WHERE username = ?");
        $stmt->bind_param("ss", $newPass, $user);

        if ($stmt->execute() && $stmt->affected_rows > 0) {
            echo json_encode(["success" => true]);
        } else {
            echo json_encode(["success" => false, "error" => "User not found or unchanged"]);
        }
        exit();
    }
}

// ── ESP CODE CHECK (GET) ───────────────
if ($_SERVER["REQUEST_METHOD"] === "GET" && isset($_GET['code'])) {

    $code = $_GET['code'];

    $stmt = $conn->prepare("SELECT * FROM espcodes WHERE unique_code = ?");
    $stmt->bind_param("s", $code);
    $stmt->execute();
    $result = $stmt->get_result();

    echo json_encode(["success" => $result->num_rows > 0]);
    exit();
}
?>