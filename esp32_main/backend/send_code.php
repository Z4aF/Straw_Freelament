<?php
$conn = new mysqli("localhost","root","","esp32_database");
if ($conn->connect_error) die("DB error");

if (isset($_POST['unique_code'])) {
    $code = $_POST['unique_code'];

    $stmt = $conn->prepare("SELECT * FROM espcodes WHERE unique_code=?");
    $stmt->bind_param("s", $code);
    $stmt->execute();
    $res = $stmt->get_result();

    if ($res->num_rows == 0) {
        $stmt = $conn->prepare("INSERT INTO espcodes(unique_code) VALUES(?)");
        $stmt->bind_param("s", $code);
        $stmt->execute();
        echo "saved";
    } else {
        echo "exists";
    }
}
?>