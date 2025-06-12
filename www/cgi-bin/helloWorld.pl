#!/usr/bin/perl

print "Content-type: text/html\r\n\r\n";

print <<"END";
<html>
<head>
<title>CGI with perl</title>
<style>
  body {
    background-image: url('https://upload.wikimedia.org/wikipedia/commons/9/97/The_Earth_seen_from_Apollo_17.jpg');
    background-size: cover;
    color: black;
    font-family: Arial, sans-serif;
    text-align: center;
    padding-top: 100px;
  }
</style>
</head>
<body>
    <h1>CGI with perl</h1>
    <h2>Hello World!</h2>
</body>
</html>
END