<?php
$title = "Description";
require("header.php");?>

<p><b>Meson Player</b> - an (almost) invisible music player.</p>
<p>
Meson Player is a very convenient audio player if you don't need any flashy GUI to control the playback or manage playlists.
You just run it and then forget about it. The player is running in a background.
Switching tracks, volume control, loading and saving playlists, all of this is performed by using global hotkeys.
Therefore, you can control the player while working in any application.
And it lets you listen to your music without adding any clutter to your screen.
</p>
<p>
The project's homepage:
<a target="_blank" href="https://mesonplayer.alkatrazstudio.net">https://mesonplayer.alkatrazstudio.net</a>.
<br>
The project on SourceForge:
<a target="_blank" href="https://sourceforge.net/projects/mesonplayer/">https://sourceforge.net/projects/mesonplayer/</a>.
<br>
The project on GitHub:
<a target="_blank" href="https://github.com/alkatrazstudio/meson-player">https://github.com/alkatrazstudio/meson-player</a>.
<br>
Author: Alexey Parfenov (
<img title="RUS" alt="[RUS]" style="box-shadow: 0px 0px 1px 1px black;" src="rus.svg" class="flag-img" />
Алексей Парфёнов)
</p>

<h3>Features</h3>
<ul>
    <li>no GUI (except a tray popup menu, but you can hide the tray icon as well);</li>
    <li>controlled by <a href="basics.html">global hotkeys</a>;</li>
    <li>supports a lot of stream/tracker formats;</li>
    <li>gapless playback;</li>
    <li>additional formats support can be added by plugins;</li>
    <li>supports Internet radio stations;</li>
    <li>supports CUE sheets;</li>
    <li>can control system audio volume;</li>
    <li>can remove tracks to Trash;</li>
    <li>big variety of <a href="params.html">parameters</a> to change the app's behavior and appearance;</li>
    <li>Last.FM scrobbling.</li>
</ul>

<h3>Supported systems</h3>
<ul>
    <li>Ubuntu v19.10 64-bit (may not work on Wayland)</li>
    <li>Windows 10 v1909 64-bit</li>
    <li>OSX Catalina</li>
</ul>

<h3>Quick start</h3>
<ol>
    <li><a target="_blank" href="https://sourceforge.net/projects/mesonplayer/files/mesonplayer/">Install</a>;</li>
    <li>Right-click on any folder with music or a playlist;</li>
    <li>Choose "Open in Meson Player" from a context menu (on Linux this procedure may vary depending on your distro).</li>
</ol>

<h3>Supported file extensions</h3>
<p>Below is the list of all supported extensions.</p>
<p class="block warning">Meson Player may not open some files with those extensions. It depends on file format.</p>

<p>
<b>Linux and OSX</b>:
.2sf, .aac, .ac3, .ahx, .ape, .as0, .asc, .ay, .ayc, .bin, .cc3, .chi, .cop, .d, .dff, .dmm, .dsf, .dsf, .dsq, .dst, .esv, .fdi, .fla, .flac, .ftc, .gam, .gamplus, .gbs, .gsf, .gtr, .gym, .hes, .hrm, .hrp, .hvl, .kar, .kss, .lzs, .m, .m4a, .m4b, .mac, .mid, .midi, .mka, .mkv, .mod, .mp, .mp4, .mpc, .mpp, .msp, .mtc, .nsf, .nsfe, .oga, .ogg, .opus, .p, .pcd, .psc, .psf, .psf2, .psg, .psm, .pt1, .pt2, .pt3, .rmi, .rmt, .rsn, .s, .sap, .scl, .sid, .spc, .spx, .sqd, .sqt, .ssf, .st1, .st3, .stc, .stp, .str, .szx, .td0, .tf0, .tfc, .tfd, .tfe, .tlz, .tlzp, .trd, .trs, .ts, .tta, .usf, .vgm, .vgz, .vtx, .webm, .wv, .ym
<br>
<b>Windows</b>:
Same as in Linux or OSX, but also with .wma
</p>
<p><b>Supported playlist formats</b>: M3U, M3U8, ASX, PLS, XSPF, WPL, CUE.</p>

<?php require("footer.php");
