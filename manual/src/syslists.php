<?php
$title = "System playlists";
require("header.php");?>

<p>
You may invoke some additional commands via hotkeys
that were not mentioned in <a href="basics.<?=$fileExt;?>">basic hotkeys</a> section.
Usually they may come in handy if you hid the app's tray icon, so that you are unable to bring up the popup menu.
To enter such a command you'll have to <a href="lists.<?=$fileExt;?>">"load" a certain playlist</a>.
These playlists ID's begin with <code>00</code>. After entering <code>00</code> you should enter a command ID:
</p>
<ul>
<li>11 - quit the application;
<li>81 - switch to "Track Once" mode;</li>
<li>82 - switch to "PLaylist Once" mode;</li>
<li>83 - switch to "Loop Track" mode;</li>
<li>84 - switch to "Loop PLaylist" mode;</li>
<li>85 - switch to "Random order" mode;</li>
<li>5 - stop and <b>close</b> the current file;</li>
<li>555 - remove the current file to Trash and proceed to the next file;</li>
<li>2 - disable all hotkeys (you will still be able to invoke app commands);</li>
<li>22 - enable all hotkeys;</li>
<li>7 - show the current file (not available on Linux);</li>
<li>77 - show the directory with the playlists</li>
</ul>

<p class="block example">For example, if you want to quit the application,
you should <a href="lists.<?=$fileExt;?>">load a playlist</a> with ID <code>0011</code>.</p>

<p>You must set <code>system-playlists=true</code> in <a href="ini.<?=$fileExt;?>">settings.ini</a>
or via <a href="command.<?=$fileExt;?>">command line</a> to enable these system playlists (they're enabled by default, though).</p>

<p class="block warning">
If system playlists are enabled, then <code>CTRL+NumPad 0</code>
(or <code>Command+NumPad 0</code> on OSX) shortcut will always be registered even if you disable all hotkeys.
Also, after entering <code>00</code> while holding <code>CTRL</code>
will enable all other <code>CTRL+NumPad X</code> hotkeys for a short amount of time,
so that you will be able to load a system playlist.
If you disabled hotkeys, then after loading a system playlist the hotkeys will be unregistered again
(of course, unless you load <code>0022</code> playlist which enables all hotkeys again).
</p>

<p class="block warning">This section uses <code>CTRL</code> as a default modifier for loading playlists,
but you should not forget, that you are able to <a href="mods.<?=$fileExt;?>">change that behavior</a>.</p>

<?php require("footer.php");
