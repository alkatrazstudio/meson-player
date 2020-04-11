<?php
$printMode = isset($argv[1]) && $argv[1] == "print";
$fileExt = $printMode ? "html" : "php";
?><!DOCTYPE HTML>
<html lang="en-US">
<head>
    <meta charset="utf-8"/>
    <link rel="stylesheet" href="styles.css">
    <title><?=$title;?> - Meson Player</title>
    <link rel="shortcut icon" href="app.png">
</head>
<body>
<header>
    <h1><a href="index.<?=$fileExt;?>">
        <img src="app.png" id="app-logo" />
        <span>Meson Player</span>
    </a></h1>
</header>
<article>
    <h1><?=$title;?></h1>
