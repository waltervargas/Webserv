#!/usr/bin/env php-cgi

<?php
// Required header for CGI
header("Content-Type: text/html\r\n\r\n");

// Array of silly jokes
$jokes = array(
    "Why don't programmers like nature? It has too many bugs.",
    "How many programmers does it take to change a light bulb? None, that's a hardware issue.",
    "Why do Java developers wear glasses? Because they can't C#.",
    "What do you call a programmer from Finland? Nerdic.",
    "Why did the PHP developer quit his job? Because he didn't get arrays."
);

// Pick a random one
$index = rand(0, count($jokes) - 1);
$joke = $jokes[$index];
?>
<html>
<head>
    <style>
        body {
            background-color: #fcd8f7;
            font-family: Arial, sans-serif;
            padding-top: 100px;
        }
    </style>
    </head>
<body>
    <h1>Hello from PHP CGI!</h1>
    <p>Here's a joke for you:</p>
    <blockquote><?= htmlspecialchars($joke) ?></blockquote>
    <form method="GET" action="/cgi-bin/joke.php">
        <input type="submit" value="Get Another Joke">
    </form>
</body>
</html>
