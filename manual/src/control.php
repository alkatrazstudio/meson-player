<?php
$title = "Control commands";
require("header.php");?>

<p>If Meson Player is running then you can pass some commands or filenames to it. The whole process goes like this:</p>
<ol>
<li>Ensure that Meson Player is already running (let's call it instance <code>A</code>);</li>
<li>Launch another instance of Meson Player (<code>B</code>) and pass commands/filenames via command line.</li>
<li><code>B</code> will pass commands/filenames to <code>A</code>;</li>
<li><code>A</code> may return some data back to <code>B</code>,
in that case <code>B</code> will output that data to <code>stdout</code>.</li>
<li><code>B</code> will shut down itself.</li>
</ol>

<p>You can pass either <b>one</b> control command (see the list below)
or some of the following (in any order and/or quantity):</p>
<ul>
<li><a href="command.html#filenames">Filenames, directories, URLs</a> of music files and/or playlists
(e.g <code>C:\music\file.ogg</code>);</li>
<li>
Certain <a href="params.html">setup parameters</a>:
<code><a href="params.html#autoload-playlists">autoload-playlists</a></code>,
<code><a href="params.html#index">index</a></code>,
<code><a href="params.html#load-only">load-only</a></code>,
<code><a href="params.html#playback-mode">playback-mode</a></code>,
<code><a href="params.html#subdirs">subdirs</a></code> and
<code><a href="params.html#volume">volume</a></code>;
</li>
</ul>

<p>Control commands have a format <code>--cmd-&lt;command-name&gt;</code>.
Note, that if you pass a control command then it should be the only command line argument passed to Meson Player.</p>
<p>The way you execute these control commands depends on a platform:</p>
<ul>
<li>On Windows, you should use <code>mesonplayerctrl.exe</code> instead of <code>mesonplayer.exe</code>.</li>
<li>On OSX, you should run the application executable instead of the application bundle.</li>
<li>On Linux, you should use <code>mesonplayer</code> executable as usual.</li>
</ul>

<div class="block example">
<p>You want to execute <code><a href="#get-volume">get-volume</a></code> command.
Depending on OS you do something like this:</p>
<ul>
<li><b>Windows</b><br>
<code>C:\Windows\SysWOW64>"C:\Program Files\Meson Player\mesonplayerctrl.exe" --cmd-get-volume
45

C:\Windows\SysWOW64>
</code>
</li>
<li><b>OSX</b><br>
<code>users-Mac:~ user$ /Applications/Meson\ Player.app/Contents/MacOS/mesonplayer --cmd-get-volume
45
users-Mac:~ user$
</code>
</li>
<li><b>Linux</b><br>
<code>user@localhost:~/mesonplayer $ ./mesonplayer --cmd-get-volume
45
user@localhost:~/mesonplayer $
</code>
</li>
</ul>
</div>

<p>Below is a list of control commands in alphabetical order.</p>

<hr id="get-all">
<h2>get-all</h2>
<p><b>Returns</b>: all available information that can be gathered from other <code>get-*</code> commands.</p>
<div class="block example">
<code>user@localhost:~/mesonplayer $ ./mesonplayer --cmd-get-all
position: 26.37
real-position: 2927.45
duration: 273.12
full-duration: 3996.12
index: 12
count: 16
state: playing
playback-mode: all_loop
formatted-title: Machinae Supremacy - The Bigger They Are The Harder They Fall
title: The Bigger They Are The Harder They Fall
artist: Machinae Supremacy
filename: /home/user/music/Machinae Supremacy/Phantom Shadow.flac
full-filename: /home/user/music/Machinae Supremacy/Phantom Shadow.cue:12
volume: 20
master-volume: 12.00
user@localhost:~/mesonplayer $
</code>
</div>

<hr id="get-artist">
<h2>get-title</h2>
<p><b>Returns</b>: track artist based on information from tags (therefore may return nothing).</p>

<hr id="get-count">
<h2>get-count</h2>
<p><b>Returns</b>: total file count in a current playlist.</p>

<hr id="get-duration">
<h2>get-duration</h2>
<p><b>Returns</b>: current track duration in seconds as a floating point value (i.e. 652.40).</p>

<hr id="get-filename">
<h2>get-filename</h2>
<p><b>Returns</b>: current filename.</p>

<hr id="get-formatted-title">
<h2>get-formatted-title</h2>
<p><b>Returns</b>: formatted track title as it's shown on the tooltip.</p>

<hr id="get-full-duration">
<h2>get-full-duration</h2>
<p><b>Returns</b>: same as <code><a href="#get-duration">get-duration</a></code>,
but returns a duration of a current file instead of a current track
(this can make a difference for CUE-splitted files or Internet-radio).</p>

<hr id="get-full-filename">
<h2>get-full-filename</h2>
<p><b>Returns</b>: same as <code><a href="#get-filename">get-filename</a></code>,
but for CUE-splitted files returns *.cue filename with <code>:n</code> appended,
where <code>n</code> is a track's zero-based index inside a corresponding CUE file
(i.e. <code>/home/user/file.cue:3</code>).</p>

<hr id="get-index">
<h2>get-index</h2>
<p><b>Returns</b>: current track index in playlist (or -1 case no music files loaded).</p>

<hr id="get-master-volume">
<h2>get-master-volume</h2>
<p><b>Returns</b>: current system-wide sound volume level as a float value in a range of [0;100] (i.e. 13.37).</p>

<hr id="get-playback-mode">
<h2>get-playback-mode</h2>
<p><b>Returns</b>: <a href="params.html#playback-mode">playback mode</a>
(<code>track_once</code>,
<code>all_once</code>,
<code>track_loop</code>,
<code>all_loop</code> or
<code>random</code>).</p>

<hr id="get-position">
<h2>get-position</h2>
<p><b>Returns</b>: current track position in seconds as a floating point value (i.e. 23.14).</p>

<hr id="get-real-position">
<h2>get-real-position</h2>
<p><b>Returns</b>: same as <code><a href="#get-position">get-position</a></code>,
but returns actual position in stream which may not match a current track position
(this may be the case for CUE-splitted files and Internet-radio streams).</p>

<hr id="get-state">
<h2>get-state</h2>
<p><b>Returns</b>: playback state
(<code>idle</code>, <code>paused</code> or <code>playing</code>).</p>

<hr id="get-title">
<h2>get-title</h2>
<p><b>Returns</b>: track title based on information from tags (therefore may return nothing).</p>

<hr id="get-volume">
<h2>get-volume</h2>
<p><b>Returns</b>: current sound volume level as an integer value in a range of [0;100] (i.e. 42).</p>

<hr id="next">
<h2>next</h2>
<p>Play a next track.</p>
<p><b>Returns</b>: all available information.
See <code><a href="#get-all">get-all</a></code> command for details.</p>

<hr id="open-next">
<h2>open-next</h2>
<p>Open a next track but do not play.</p>
<p><b>Returns</b>: all available information.
See <code><a href="#get-all">get-all</a></code> command for details.</p>

<hr id="open-prev">
<h2>open-prev</h2>
<p>Open a previous track but do not play.</p>
<p><b>Returns</b>: all available information.
See <code><a href="#get-all">get-all</a></code> command for details.</p>

<hr id="pause">
<h2>pause</h2>
<p>Pause playback.</p>

<hr id="play">
<h2>play</h2>
<p>Play (or restart) current track.</p>

<hr id="prev">
<h2>prev</h2>
<p>Play a previous track.</p>
<p><b>Returns</b>: all available information.
See <code><a href="#get-all">get-all</a></code> command for details.</p>

<hr id="quit">
<h2>quit</h2>
<p>Quit the application.</p>

<hr id="seek-back">
<h2>seek-back</h2>
<p>Rewind by 1% of the track duration.</p>
<p><b>Returns</b>: current track position.
See <code><a href="#get-position">get-position</a></code> command for details.</p>

<hr id="seek-forward">
<h2>seek-forward</h2>
<p>Fast-forward by 1% of the track duration.</p>
<p><b>Returns</b>: current track position.
See <code><a href="#get-position">get-position</a></code> command for details.</p>

<hr id="stop">
<h2>stop</h2>
<p>Stop playback.</p>

<hr id="toggle-pause">
<h2>toggle-pause</h2>
<p>If a track is paused/stopped then continue/start playback, else pause playback.</p>
<p><b>Returns</b>: current playback state
(<code>idle</code>, <code>paused</code> or <code>playing</code>).</p>

<hr id="toggle-play">
<h2>toggle-play</h2>
<p>If a track is paused/stopped then continue/start playback, else stop playback.</p>
<p><b>Returns</b>: current playback state
(<code>idle</code>, <code>paused</code> or <code>playing</code>).</p>

<hr id="volume-down">
<h2>volume-down</h2>
<p>Lower sound volume down. Volume step can be set via
<code><a href="params.html#volume-step">volume-step</a></code> parameter.</p>
<p><b>Returns</b>: resulting sound volume in range of [0;100].</p>

<hr id="volume-up">
<h2>volume-up</h2>
<p>Raise sound volume up. Volume step can be set via
<code><a href="params.html#volume-step">volume-step</a></code> parameter.</p>
<p><b>Returns</b>: resulting sound volume in range of [0;100].</p>

<?php require("footer.php");
