<?php
$title = "Playlists";
require("header.php");?>

<p class="block warning">Some <a href="mods.html">options</a> can alter hotkeys behavior!</p>

<p class="block warning">When on OSX, keep in mind that all <code>CTRL</code>'s in this document refer to <code>Command</code> key, not to an actual <code>CTRL</code> key.</p>

<p>Main points:</p>
<ul>
<li>You can manage playlists. Each playlist is referenced by its playlist ID, which is a numeric value.</li>
<li>To save currently loaded tracks to a playlist, hold <code>ALT</code>, then type playlist ID on NumPad.</li>
<li>To load tracks from a saved playlist, hold <code>CTRL</code>, then type playlist ID on NumPad.</li>
<li>Note, that playlists ID starting with "0" are <a href="syslists.html">reserved</a> and should not be used as regular playlists.</li>
<li>Your playlists are stored in <code>&lt;your home directory&gt;/.mesonplayer/playlists</code>.</li>
</ul>

<div class="block example">
<ol>
<li>Right-click on any folder with music and select "Open in Meson Player". This will load all music files in this folder.</li>
<li>Hold <code>ALT</code>. Type <code>123</code> on your NumPad. Release <code>ALT</code>. This will save all current tracks into the playlist located at: <code>&lt;your home directory&gt;/.mesonplayer/playlists/123.m3u</code></li>
<li>You can rename this playlist to something like <code>123 - My Music.m3u</code> for convenience.</li>
<li>To load this playlist in future do the following: Hold <code>CTRL</code>, type <code>123</code> on your NumPad, release <code>CTRL</code>.</li>
</ol>
</div>

<p>
If you want to manually craft your playlists then keep in mind that you also can use folders or even other playlists as playlist entries.
Obviously, such playlists will only be supported by Meson Player.</p>

<div class="block example">
<p>The following M3U playlist is valid in Meson Player:</p>
<p><code>#EXTM3U
D:/Music/Band_1/Track_1.mp3
D:/Music/Band_2
D:/Music/Band_3/album.pls
</code>
</div>

<?php require("footer.php");
