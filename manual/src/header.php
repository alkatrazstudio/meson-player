<?php $isSite = $argv[1] == "site";?>
<!DOCTYPE HTML>
<html lang="en-US">
<head>
    <meta charset="utf-8"/>
    <link rel="stylesheet" href="styles.css">
    <title><?=$title;?> - Meson Player</title>
    <link rel="shortcut icon" href="app.png">
</head>
<body>
<header>
    <h1><a href="index.html">
        <img src="app.png" id="app-logo" />
        <span>Meson Player</span>
    </a></h1>
</header>
<article>
    <h1><?=$title;?></h1>
