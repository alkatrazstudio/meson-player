<?php
$title = "Description";
require("header.php");?>

<p><b>Meson Player</b> - music player without GUI. &copy; Alkatraz Studio, 2012.</p>
<p>
Meson Player is a very convenient audio player if you don't need any flashy GUI to control a playback or manage playlists.
You just run it and then forget about it. The player is running in a background.
Switching tracks, volume control, loading and saving playlists, all of this is performed by using global hotkeys.
Therefore, you can control the player while working in any application.
</p>
<p>
The project on SourceForge.net:
<a target="_blank" href="https://mesonplayer.sourceforge.io">https://mesonplayer.sourceforge.io</a>.
<br>
Author: Alexey Parfenov (
<img title="RUS" alt="[RUS]" style="box-shadow: 0px 0px 1px 1px black;" src="rus.svg" class="flag-img" />
Алексей Парфёнов)
</p>

<h3>Features</h3>
<ul>
    <li>no GUI (except a tray popup menu, but you can hide the tray icon as well);</li>
    <li>controlled by <a href="basics.<?=$fileExt;?>">global hotkeys</a>;</li>
    <li>supports a lot of stream/tracker formats;</li>
    <li>gapless playback;</li>
    <li>additional formats support can be added by plugins;</li>
    <li>supports Internet radio stations;</li>
    <li>partial support for CUE sheets;</li>
    <li>can control system audio volume;</li>
    <li>can remove tracks to Trash;</li>
    <li>big variety of <a href="params.<?=$fileExt;?>">parameters</a> allows to change the app's behavior and appearance;</li>
    <li>Last.FM scrobbling.</li>
</ul>

<h3>Quick Start</h3>
<ol>
    <li><a target="_blank" href="https://sourceforge.net/projects/mesonplayer/files/mesonplayer/">Install</a>;</li>
    <li>Right-click on any folder with music or a playlist;</li>
    <li>Choose "Open in Meson Player" from a context menu.</li>
</ol>

<h3>Supported file extensions</h3>
<p>Below is the list of all supported extensions.</p>
<p class="block warning">Meson Player may not open some files with those extensions. It depends on file format.</p>

<p>
<b>Windows &amp; OSX</b>:
mp3, mp2, mp1, ogg, wav, aiff, mo3, it, xm, s3m, mtm, mod, umx, mdz, s3z, xmz, itz,
aac, m4a, m4b, mp4, ape, cda,
ac3, mac, mpc, mp+, mpp, spx, tta, flac, fla, oga, midi, mid, rmi, kar, wv,
$b, $m, ahx, as0, asc, ay, ayc, bin, cc3, chi, cop, d, dmm, dsq, dst, esv, fdi, ftc, gam, gamplus, gbs, gtr, gym, hes, hrm, hrp, lzs, m, msp, mtc, nsf, nsfe, p, pcd, psc, psg, psm, pt1, pt2, pt3, s, sap, scl, sid, spc, sqd, sqt, st1, st3, stc, stp, str, szx, td0, tf0, tfc, tfd, tfe, tlz, tlzp, trd, trs, ts, vgm, vtx, ym;
<br>
<b>Windows only</b>: ofr, ofs;
<br>
<b>Windows i686 only</b>: adx, aix;
<br>
<b>Linux</b>:
Visit
<a href="https://www.un4seen.com/bass.html#addons" target="_blank">https://www.un4seen.com/bass.html#addons</a>
to download plugins for Linux.
<br>
</p>
<p><b>Supported playlist formats</b>: M3U, M3U8, ASX, PLS, XSPF, WPL, CUE.</p>

<?php require("footer.php");
