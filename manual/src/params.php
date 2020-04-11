<?php
$title = "Setup parameters";
require("header.php");?>

<p>Setup parameters affect various aspects of playback and appearance of Meson Player.
Such parameters can be loaded from <a href="ini.html">settings.ini</a> files
or passed via <a href="command.html">command line</a>.
You can pass these parameters only if the application isn't running yet (except
<code><a href="#autoload-playlists">autoload-playlists</a></code>,
<code><a href="#index">index</a></code>,
<code><a href="#load-only">load-only</a></code>,
<code><a href="#playback-mode">playback-mode</a></code>,
<code><a href="#subdirs">subdirs</a></code> and
<code><a href="#volume">volume</a></code>
<code><a href="#position">position</a></code>).</p>

<p>Below is the full list of setup parameters in alphabetical order.</p>

<hr id="auto-numlock">
<h2>auto-numlock</h2>
<p>Set Num Lock on startup. Only for Windows.</p>
<p><b>Valid values</b>: true, false</p>
<p><b>Default</b>: true</p>

<hr id="auto-resume">
<h2>auto-resume</h2>
<p>Resume playback if no input files supplied and the app was closed via user logout while playing a music.</p>
<p><b>Valid values</b>: true, false</p>
<p><b>Default</b>: true</p>

<hr id="autoload-playlists">
<h2>autoload-playlists</h2>
<p>When loading directories, also load files in found playlists.</p>
<p><b>Valid values</b>: true, false</p>
<p><b>Default</b>: false</p>

<hr id="balloons">
<h2>balloons</h2>
<p>Show balloon-style notifications on every event (track/volume change).</p>
<p><b>Valid values</b>: true, false</p>
<p><b>Default</b>: true</p>

<hr id="buffer-length">
<h2>buffer-length</h2>
<p>The playback buffer length in milliseconds. Increasing the length, decreases the chance of the sound possibly breaking-up on slower computers.</p>
<p><b>Valid values</b>: 10..5000, but not lower than <code><a href="#update-period">update-period</a></code>
<p><b>Default</b>: 500</p>

<hr id="close-on-stop">
<h2>close-on-stop</h2>
<p>Close the file on stop?</p>
<p><b>Valid values</b>:</p>
<ul>
<li>true - If user stops a playback, the file will be closed, so that it can be deleted, for example;</li>
<li>false - The current track will remain open even after the player is stopped.</li>
</ul>
<p><b>Default</b>: true</p>

<hr id="device">
<h2>device</h2>
<p>Use a specific device for a sound output.</p>
<p><b>Valid values</b>:</p>
<ul>
<li>-1 - default device;</li>
<li>0 - no sound;</li>
<li>1 - first real output device;</li>
<li>2 - second real output device (and so on)</li>
</ul>
<p><b>Default</b>: -1</p>

<hr id="frequency">
<h2>frequency</h2>
<p>Output sample rate in Hz. Only for Linux.</p>
<p><b>Valid values</b>: 192000, 96000, 48000, 44100, 22050, 16000, 11025, 8000</p>
<p><b>Default</b>: 44100</p>

<hr id="index">
<h2>index</h2>
<p>Sets a file number to play. The first filename has the index of zero.</p>
<p class="warning">This parameter can only be specified via <a href="command.html">command line</a> and
only makes sense when some filenames were also specified.</p>
<p>If there are filenames in command line as well, then this parameter will choose what file to play. If there were no filenames, then this parameter will change the current track to a track with the specified index in a current playlist.</p>
<p><b>Valid values</b>: any positive integer</p>

<hr id="log">
<h2>log</h2>
<p>Error logs will be saved in <code>&lt;user home dir&gt;/.mesonplayer/logs/</code>.</p>
<p><b>Valid values</b>: true, false</p>
<p><b>Default</b>: false</p>

<hr id="load-only">
<h2>load-only</h2>
<p>Whether to play or not to play the music after the music file is loaded.</p>
<p class="warning">This parameter can only be specified via <a href="command.html">command line</a> and
only makes sense when some filenames were also specified.</p>
<p><b>Valid values</b>: true, false</p>
<p><b>If not specified</b>: Auto-play music. The exact music file can be specified via
<code><a href="#index">index</a></code> parameter.</p>

<hr id="min-folder-length">
<h2>min-folder-length</h2>
<p>When current folder's name length is less than this, then prepend parent directory name to it (for tooltip).</p>
<p><b>Valid values</b>: any positive integer</p>
<p><b>Default</b>: 5</p>

<hr id="mods-cmd">
<h2>mods-cmd</h2>
<p><a href="mods.html">Hotkey modifiers</a> for command keys (play, next, vol up, ...).</p>
<p><b>Valid values</b>: unspecified, <code>OFF</code> or any combination of <code>CTRL</code>, <code>SHIFT</code> or <code>ALT</code> separated by <code>+</code> sign.</p>
<p class="block warning">These values should not be identical to <code><a href="#mods-load">mods-load</a></code> or <code><a href="#mods-save">mods-save</a></code> parameters.</p>
<p><b>Default</b>: &lt;none&gt;</p>

<hr id="mods-mm">
<h2>mods-mm</h2>
<p><a href="mods.html">Hotkey modifiers</a> for multimedia keys (play/pause, next, ...).</p>
<p><b>Valid values</b>: unspecified, <code>OFF</code> or any combination of <code>CTRL</code>, <code>SHIFT</code> or <code>ALT</code> separated by <code>+</code> sign.</p>
<p class="block warning">These values should not be identical to <code><a href="#mods-mm2">mods-mm2</a></code> parameter.</p>
<p><b>Default</b>: &lt;none&gt;</p>

<hr id="mods-mm2">
<h2>mods-mm2</h2>
<p><a href="mods.html">Hotkey modifiers</a> for multimedia keys for jumping to a next/previous folder.</p>
<p><b>Valid values</b>: unspecified, <code>OFF</code> or any combination of <code>CTRL</code>, <code>SHIFT</code> or <code>ALT</code> separated by <code>+</code> sign.</p>
<p class="block warning">These values should not be identical to <code><a href="#mods-mm">mods-mm</a></code> parameter.</p>
<p><b>Default</b>: CTRL</p>

<hr id="mods-load">
<h2>mods-load</h2>
<p><a href="mods.html">Hotkey modifiers</a> for loading playlists.</p>
<p><b>Valid values</b>: unspecified, <code>OFF</code> or any combination of <code>CTRL</code>, <code>SHIFT</code> or <code>ALT</code> separated by <code>+</code> sign.</p>
<p class="block warning">These values should not be identical to <code><a href="#mods-cmd">mods-cmd</a></code> or <code><a href="#mods-save">mods-save</a></code> parameters.</p>
<p><b>Default</b>: CTRL</p>

<hr id="mods-save">
<h2>mods-save</h2>
<p><a href="mods.html">Hotkey modifiers</a> for saving playlists.</p>
<p><b>Valid values</b>: unspecified, <code>OFF</code> or any combination of <code>CTRL</code>, <code>SHIFT</code> or <code>ALT</code> separated by <code>+</code> sign.</p>
<p class="block warning">These values should not be identical to <code><a href="#mods-cmd">mods-cmd</a></code> or <code><a href="#mods-load">mods-load</a></code> parameters.</p>
<p><b>Default</b>: ALT</p>

<hr id="mpris">
<h2>mpris</h2>
<p>Propagate Meson Player via MPRIS interface. Can be used to allow other applications (e.g. KDE Connect) to control Meson Player via D-Bus. Linux only.</p>
<p><b>Valid values</b>: true, false</p>
<p><b>Default</b>: true</p>

<hr id="mvol-step">
<h2>mvol-step</h2>
<p>Change a master volume by specified step.</p>
<p><b>Valid values</b>: any value in range [0;100]</p>
<p><b>Default</b>: 5</p>

<hr id="playback-mode">
<h2>playback-mode</h2>
<p>Set playback mode.</p>
<p class="warning">This parameter can only be specified via <a href="command.html">command line</a>.</p>
<p><b>Valid values</b>:</p>
<ul>
<li>track_once - Play one track one time then stop;</li>
<li>all_once - Sequentially play all tracks from the current playlist then stop;</li>
<li>track_loop - Repeat one track in a loop;</li>
<li>all_loop - Sequentially play all tracks from the current playlist then proceed to the first track and repeat the process;</li>
<li>random - Play tracks from the current playlist in a random order without stop</li>
</ul>
<p><b>If not specified</b>: do not change playback mode.</p>

<hr id="playlists-type">
<h2>playlists-type</h2>
<p>Save <a href="lists.html">playlists</a> in this format.</p>
<p><b>Valid values</b>: M3U, ASX, XSPF, PLS, WPL</p>
<p><b>Default</b>: M3U</p>
<p class="block warning">Playlists are saved in UTF-8, even M3U.</p>

<hr id="popup-duration">
<h2>popup-duration</h2>
<p>Duration of <a href="#balloons">notification popup</a> in seconds.</p>
<p><b>Valid values</b>: any positive value</p>
<p><b>Default</b>: 5</p>

<hr id="position">
<h2>position</h2>
<p>Sets a file position in seconds. If there are filenames in command line as well, then the first file that's going to be played will start from te specified position. If there were no filenames, then this parameter will change the position of currently playing file.</p>
<p class="warning">This parameter can only be specified via <a href="command.html">command line</a> when the application is already running.</p>
<p><b>Valid values</b>: any positive floating point value (e.g. 12.5)</p>

<hr id="proxy">
<h2>proxy</h2>
<p>Use this proxy for network requests. SOCKS5 and HTTP proxies are supported.</p>
<p>The format of this value is following:
<code>&lt;scheme&gt;://[&lt;user&gt;[:&lt;pass&gt;]@]&lt;host&gt;:&lt;port&gt;</code>,
where</p>
<ul>
<li><code>&lt;scheme&gt;</code> - <code>socks5</code> or <code>http</code>;</li>
<li><code>&lt;user&gt;</code>, <code>&lt;pass&gt;</code> - user credentials (not required);</li>
<li><code>&lt;host&gt;</code>, <code>&lt;port&gt;</code> - host and port of the proxy.</li>
</ul>
<p>You may also set this value to <code>system</code> in which case the system settings will be used
(not supported on 64-bit Windows).</p>
<div class="block example">
<code>socks5://127.0.0.1:9050</code><br>
<code>https://h4x0r:myP4SS@example.com:80</code>
</div>
<p><b>Default</b>: system</p>

<hr id="sample-interpolation">
<h2>sample-interpolation</h2>
<p>Set sample interpolation mode. Only applies to the following music formats: mo3, it, xm, s3m, mtm, mod, umx, mdz, s3z, xmz, itz.</p>
<p><b>Valid values</b>:</p>
<ul>
<li>sinc - Use sinc interpolated sample mixing. This increases the sound quality, but also requires more processing;</li>
<li>none - Use non-interpolated sample mixing. This generally reduces the sound quality, but can be good for chip-tunes;</li>
<li>linear - Linear interpolation.</li>
</ul>
<p><b>Default</b>: sinc</p>

<hr id="sample-ramping">
<h2>sample-ramping</h2>
<p>Set sample ramping mode. Only applies to the following music formats: mo3, it, xm, s3m, mtm, mod, umx, mdz, s3z, xmz, itz.</p>
<p><b>Valid values</b>:</p>
<ul>
<li>normal - smoothen volume/pan changes to avoid "clicks";</li>
<li>sensitive - Same as normal, but will only ramp the start of a sample if it thinks that it would "click" otherwise.
This keeps percussive sounds sharp, whereas normal ramping can slightly deaden their impact;</li>
<li>none - no ramping is applied.</li>
</ul>
<p><b>Default</b>: sensitive</p>

<hr id="sample-type">
<h2>sample-type</h2>
<p>Set the type of samples.</p>
<p><b>Valid values</b>:</p>
<ul>
<li>float32 -  Use 32-bit floating-point sample data;</li>
<li>8bits - Use 8-bit resolution;</li>
<li>none - Use 16-bit resolution.</li>
</ul>
<p><b>Default</b>: float32</p>
<p class="block warning">If you set <code>sample-type=8bits</code>, then you must set <code><a href="#use-8bits">use-8bits</a>=true</code>.</p>

<hr id="short-captions">
<h2>short-captions</h2>
<p>Show only track title in the tray hint.</p>
<p><b>Valid values</b>: true, false</p>
<p><b>Default</b>: false</p>

<hr id="subdirs">
<h2>subdirs</h2>
<p>Load music files recursively when a folder is specified via <a href="command.html">command line</a>.</p>
<p><b>Valid values</b>: true, false</p>
<p><b>Default</b>: true</p>

<hr id="surround-mode<">
<h2>surround-mode</h2>
<p>Set surround mode. Only applies to the following music formats: mo3, it, xm, s3m, mtm, mod, umx, mdz, s3z, xmz, itz.</p>
<p>Discarded if <code>use-mono</code> is set to true.</p>
<p><b>Valid values</b>:</p>
</ul>
<li>none - do not apply surround effects;</li>
<li>mode1 - apply XMPlay's surround sound to the music;</li>
<li>mode2 - same as mode1, but ignores panning, which is useful if the music's channels are all centered.</li>
</ul>
<p><b>Default</b>: none</p>

<hr id="system-playlists">
<h2>system-playlists</h2>
<p><a href="syslists.html">Enables system playlists</a>.</p>
<p><b>Valid values</b>: true, false</p>
<p><b>Default</b>: true</p>

<hr id="tracker-emulation">
<h2>tracker-emulation</h2>
<p>Set tracker emulation mode. Only applies to the following music formats: mo3, it, xm, s3m, mtm, mod, umx, mdz, s3z, xmz, itz.</p>
<p><b>Valid values</b>:</p>
<ul>
<li>none - play .MOD file as usual;</li>
<li>fastTracker2 - play .MOD file as FastTracker 2 would;</li>
<li>proTracker1 - play .MOD file as ProTracker 1 would.</li>
</ul>
<p><b>Default</b>: none</p>

<hr id="tray-icon">
<h2>tray-icon</h2>
<p>Display a tray icon.</p>
<p>
Note, that even if the tray icon is not displayed, you're still able to close the application gracefully.
For that you should set <code>system-playlists</code> parameter to true.
Then you can close the app using <code>0011</code> <a href="control.html">command</a>.
<p>
<b>Valid values</b>: true, false</p>
<p><b>Default</b>: true</p>

<hr id="type-timeout">
<h2>type-timeout</h2>
<p>When typing a <a href="lists.html">playlist ID</a> you have only a limited time interval to enter a next digit.
This parameter sets this interval in milliseconds.</p>
<p><b>Valid values</b>: any positive integer value</p>
<p><b>Default</b>: 1000</p>

<hr id="unlimited-playlists">
<h2>unlimited-playlists</h2>
<p>When this option is set to false, then a <a href="lists.html">playlist</a> will be loaded/saved instanly
when you type the first digit of its ID.</p>
<p>Note that disabling this option will limit the number of playlists to 9.</p>
<p><b>Valid values</b>: true, false</p>
<p><b>Default</b>: true</p>

<hr id="update-period">
<h2>update-period</h2>
<p>The amount of time (in milliseconds) between updates of the playback buffers. Shorter update periods allow smaller buffers to be set with the <code><a href="#buffer-length">buffer-length</a></code>, but as the rate of updates increases, so the overhead of setting up the updates becomes a greater part of the CPU usage.</p>
<p><b>Valid values</b>: 5..100, but not greater than <code><a href="#buffer-length">buffer-length</a></code>.</p>
<p><b>Default</b>: 100</p>

<hr id="use-8bits">
<h2>use-8bits</h2>
<p>Use 8-bit resolution, else 16-bit.</p>
<p><b>Valid values</b>: true, false</p>
<p><b>Default</b>: false</p>
<p class="block warning">If you set <code>use-8bits=true</code>, then you must set <code><a href="#sample-type">sample-type</a>=8bits</code>.</p>

<hr id="use-default-device">
<h2>use-default-device</h2>
<p>Automatically switch the default output to a new default device whenever it changes. Only works on Windows.</p>
<p><b>Valid values</b>: true, false</p>
<p><b>Default</b>: true</p>

<hr id="use-mono">
<h2>use-mono</h2>
<p>Use mono output.</p>
<p><b>Valid values</b>: true, false</p>
<p><b>Default</b>: false</p>

<hr id="use-software">
<h2>use-software</h2>
<p>Use software for sound processing, else use hardware.</p>
<p><b>Valid values</b>: true, false</p>
<p><b>Default</b>: false</p>

<hr id="volume">
<h2>volume</h2>
<p>Sets a sound volume.</p>
<p class="warning">This parameter can only be specified via <a href="command.html">command line</a>.</p>
<p><b>Valid values</b>: any integer within a range [0;100]</p>
<p><b>If not specified</b>: Do not change the sound volume.</p>

<hr id="volume-step">
<h2>volume-step</h2>
<p>Change volume by specified steps.</p>
<p><b>Valid values</b>: any integer value in range [0;100]</p>
<p><b>Default</b>: 5</p>

<?php require("footer.php");
