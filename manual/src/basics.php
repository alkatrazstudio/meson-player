<?php
$title = "Basic hotkeys";
require("header.php");?>

<p class="block warning">Some <a href="mods.<?=$fileExt;?>">parameters</a> can alter hotkeys behavior!</p>

<p class="block warning">When on OSX, keep in mind that all <code>CTRL</code>'s in this document refer to <code>Command</code> key, not to an actual <code>CTRL</code> key.</p>

<p>Meson Player is controlled mostly using your NumPad keys:</p>
<ul>
<li>NumPad 5 - stop/play</li>
<li>NumPad 0 - pause/resume</li>
<li>NumPad 4 - previous track</li>
<li>NumPad 6 - next track</li>
<li>NumPad 4 (long press) - rewind</li>
<li>NumPad 6 (long press) - fast forward</li>
<li>NumPad 7 - jump to previous folder</li>
<li>NumPad 9 - jump to next folder</li>
<li>NumPad 8 - volume up</li>
<li>NumPad 2 - volume down</li>
<li>NumPad 1 - system volume down</li>
<li>NumPad 3 - system volume up</li>
</ul>

<p>Multimedia keys are also supported:</p>
<ul>
<li>PLAY/PAUSE - stop/play</li>
<li>CTRL+PLAY/PAUSE - pause/resume</li>
<li>NEXT - next track</li>
<li>PREV - previous track</li>
<li>CTRL+NEXT - jump to next folder</li>
<li>CTRL+PREV - jump to previous folder</li>
</ul>

<?php require("footer.php");
