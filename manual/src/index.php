<?php
$title = "Manual";
require("header.php");?>

<p><b>Meson Player</b> - music player without GUI.</p>
<?php if(!$printMode) { ?>
<p>Latest version: <b>0.7</b> &#10152; <a target="_blank" href="https://sourceforge.net/projects/mesonplayer/files/mesonplayer/">Download!</a></p>
<?php } ?>
<img src="shot.png" class="img-shot">

<ul>
    <li><a href="desc.<?=$fileExt;?>">Description</a></li>
    <li><a href="basics.<?=$fileExt;?>">Basic hotkeys</a></li>
    <li><a href="lists.<?=$fileExt;?>">Playlists</a></li>
    <li><a href="mods.<?=$fileExt;?>">Hotkey modifiers</a></li>
    <li><a href="ini.<?=$fileExt;?>">INI files</a></li>
    <li><a href="command.<?=$fileExt;?>">Command line</a></li>
    <li><a href="params.<?=$fileExt;?>">Setup parameters</a></li>
    <li><a href="control.<?=$fileExt;?>">Control commands</a></li>
    <li><a href="syslists.<?=$fileExt;?>">System playlists</a></li>
    <li><a href="credits.<?=$fileExt;?>">Credits &amp; Licenses</a></li>
</ul>

<?php require("footer.php");
