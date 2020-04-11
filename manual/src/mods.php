<?php
$title = "Hotkey modifiers";
require("header.php");?>

<p class="block warning">When on OSX, keep in mind that all <code>CTRL</code>'s in this document refer to <code>Command</code> key, not to an actual <code>CTRL</code> key.</p>

<p>You may change default keyboard modifiers (<code>CTRL</code>, <code>ALT</code>, <code>SHIFT</code>) for keys.</p>
<p>By default there's a following layout:</p>
<ul>
<li>command keys (play/stop, vol up, next track) have no modifiers;</li>
<li>keys for loading playlists have <code>CTRL</code> modifier;</li>
<li>keys for saving playlists have <code>ALT</code> modifier;</li>
<li>multimedia keys without any modifiers are used for "stop/play", "next track" and "previous track" commands;</li>
<li>multimedia keys with <code>CTRL</code> modifier are used for "pause/resume", "next folder" and "previous folder" commands.</li>
</ul>

<p>Use <a href="params.<?=$fileExt;?>#mods-cmd">mods-* parameters</a> in <a href="ini.<?=$fileExt;?>">settings.ini file</a>
or pass these parameters via <a href="command.<?=$fileExt;?>">command line</a>.</p>

<p class="block example">
For example, by default there are some main hotkeys that don't require any modifiers,
therefore to enter a command (e.g. volume up) you just need to press a NumPad key.
However, in that case you won't be able to use NumPad keys for any other means until you disable all hotkeys.
If you need NumPad keys for your own purposes, then you can apply modifier(s) to command keys.
Use <a href="params.<?=$fileExt;?>#mods-cmd">mods-* parameters</a> in <a href="ini.<?=$fileExt;?>">settings.ini file</a>.
E.g. you can set <a href="params.<?=$fileExt;?>#mods-cmd">mods-cmd</a>=CTRL+ALT in settings.ini,
so that any command will be executed only if you hold <code>CTRL</code> and <code>ALT</code> keys.
That is, to jump to the next track you should press <code>CTRL+ALT+NumPad 6</code> instead of <code>NumPad 6</code>.
</p>

<p>You can completely disable a certain set of hotkeys.
Specify a value <code>OFF</code> for a corresponding <a href="params.<?=$fileExt;?>#mods-cmd">mods-* parameter</a>.</p>

<p class="block warning"><code>SHIFT</code> modifier is not supported on Windows.</p>
<p class="block warning">Key modifiers should not overlap.
For example, you should not set the same modifiers for loading playlists and for saving playlists.
If you do so, then either first or second set of hotkeys will be inaccessible.</p>

<?php require("footer.php");
