<?php
$title = "Command line";
require("header.php");?>

<p>You can launch Meson Player via command line and pass optional arguments to the application.</p>

<p id="filenames">You can pass the following (in any order and/or quantity):</p>
<ul>
<li>Filenames of music files (e.g <code>C:\music\file.ogg</code>);</li>
<li>Filenames of <a href="lists.<?=$fileExt;?>">playlist</a> files (e.g <code>C:\music\album.m3u</code>);</li>
<li>Directories (e.g <code>~/Music</code>);</li>
<li>Internet streams (e.g <code>https://example.com:8777</code>).</li>
</ul>

<p>Additionally, you can pass either <a href="params.<?=$fileExt;?>">setup parameters</a> or <a href="control.<?=$fileExt;?>">control commands</a></p>

<h3>Setup Parameters</h3>
<p>Setup parameters can only be passed when launching Meson Player, i.e. when there's no running instance of the application. That means that you can't configure the application on the fly.</p>
<p>Setup parameters has a format <code>--&lt;param-name&gt;=&lt;value&gt;</code> (e.g. <code>--proxy=socks5://127.0.0.1:9050</code>)</p>
<p>Parameters passed via command line overwrite corresponding parameters from <a href="ini.<?=$fileExt;?>">settings.ini</a> files.</p>

<div class="block example">
<p>Consider the following invocation:</p>
<code>"C:\Program Files\Meson Player\mesonplayer.exe" --subdirs=false C:\Music --subdirs=true C:\Music\Band1\ C:\Music\Band2\Album\Track.flac --playback-mode=random https://example.com:12345/playlist.m3u --playback-mode=all_loop</code>
<p>This will:</p>
<ol>
<li>launch Meson Player;</li>
<li>load all music files inside C:\Music, but not inside its subdirectories;</li>
<li>add all music files inside C:\Music\Band1 and its subdirectories;</li>
<li>add C:\Music\Band2\Album\Track.flac to the playlist;</li>
<li>switch to random playback mode;</li>
<li>add Internet-stream https://example.com:12345/playlist.m3u to a current list of music files;</li>
<li>switch to "Loop Playlist" playback mode, and cancel the previous "playback-mode" parameter.</li>
</ol>
</div>

<p>See a <a href="params.<?=$fileExt;?>">full list of allowed setup parameters</a>.</p>

<h3>Control commands</h3>

<p><a href="control.<?=$fileExt;?>">Control commands</a> can be invoked when there's already a running instance of Meson Player. In that case when another instance is launched with a set of command line arguments, that instance will pass all arguments to the running instance, then return a result if neeeded.</p>
<p>Control commands has a format <code>--cmd-&lt;command-name&gt;</code> (e.g. <code>--cmd-toggle-play</code>)</p>
<p>Commands that start with <code>--cmd-get-</code> will return a result to <code>stdout</code></p>

<div class="block example">
<code>user@localhost:~/mesonplayer $ ./mesonplayer --cmd-get-volume
32
user@localhost:~/mesonplayer $
</code>
</div>

<p class="block warning">You can only pass one command at a time, i.e. you can't do the following: <code>mesonplayer --cmd-play --cmd-get-filename</code>.</p>

<p>See a <a href="control.<?=$fileExt;?>">full list of allowed control comands</a>.</p>
<p>You can also pass special parameters to running instance of Meson Player. Follow the above link to read about these parameters.</p>

<?php require("footer.php");
