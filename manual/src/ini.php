<?php
$title = "INI files";
require("header.php");?>

<p>You can set <a href="params.html">parameters</a> for Meson Player
using settings.ini files and/or <a href="command.html">command line</a>.</p>

<p>Upon startup Meson Player will look for <code>settings.ini</code> file in the following places:</p>
<ol>
<li><code>&lt;application's directory&gt;/settings.ini</code></li>
<li><code>&lt;user's home directory&gt;/.mesonplayer/settings.ini</code></li>
</ol>
<p>The settings in the second file overwrite settings from the first file.</p>

<p>In case you want to use settings.ini file you'll have to manually create this file
and save it to any of the above mentioned locations.</p>

<p><code>settings.ini</code> has common INI format:</p>
<div class="block example">
<code>;comment line
[config]
auto-numlock=true
balloons=true
frequency=44100
</code>
</div>

<p class="block warning">It's better to leave the first line empty or place a comment in there.<p>

<?php require("footer.php");
