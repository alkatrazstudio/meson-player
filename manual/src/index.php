<?php
$title = "Manual";
require("header.php");?>

<p><b>Meson Player</b> - an (almost) invisible music player.</p>

<p>
    <a target="_blank" href="https://sourceforge.net/projects/mesonplayer/files/mesonplayer/">Download</a>
    &middot;
    <a target="_blank" href="https://github.com/alkatrazstudio/meson-player">Get source code</a>
</p>

<img src="shot.png" class="img-shot">

<ul>
    <li><a href="desc.html">Description</a></li>
    <?php if($isSite) { ?>
    <li><a href="screenshots.html">Screenshots</a></li>
    <?php } ?>
    <li><a href="basics.html">Basic hotkeys</a></li>
    <li><a href="lists.html">Playlists</a></li>
    <li><a href="mods.html">Hotkey modifiers</a></li>
    <li><a href="ini.html">INI files</a></li>
    <li><a href="command.html">Command line</a></li>
    <li><a href="params.html">Setup parameters</a></li>
    <li><a href="control.html">Control commands</a></li>
    <li><a href="syslists.html">System playlists</a></li>
    <li><a href="credits.html">Credits &amp; Licenses</a></li>
</ul>

<?php require("footer.php");
