#!/usr/bin/perl

print "Content-type: text/html\r\n\r\n";

print <<"END";
<html>
<head>
<title>CGI with perl</title>
<style>
  body {
    background-image: url('BlueMarble.jpg');
    background-size: cover;
    color: white;
    font-family: Arial, sans-serif;
    text-align: center;
    padding-top: 100px;
  }
  h1 {
    font-size: 3em;
  }
  h2 {
    font-size: 2em;
  }
</style>
</head>
<body>
    <h1>CGI with perl</h1>
    <h2>Hello World!</h2>
</body>
</html>
END