<?php
$curver = $_GET["v"];
if (preg_match('/\d+\.\d+\.\d+/', $curver) == 0 ) {
	header("Location: http://coralietab.mozdev.org/");
	exit;
}
else {
	$client = $_GET["cli"];
	if (strcmp($client, "o") != 0) {
		header("Location: http://findicons.com/ietab?v=" . $curver);
		exit;
	}
	else {
		$latest = '2.04.20110724';
	}
}
?>
<!DOCTYPE HTML PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xml:lang="en" xmlns="http://www.w3.org/1999/xhtml" lang="en">
<head>
	<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
    <script type="text/javascript" language="javaScript" src="res/jquery.js"></script>
    <script type="text/javascript" language="javascript" src="res/install.js"></script>
    <title>IE Tab + Installed</title>
    <link rel="stylesheet" type="text/css" href="res/whatsnew.css">
</head>
<body>
	<div id="top-bar">
		<div id="top-bar-center">
			<strong>IE Tab + v<?php echo $curver; ?> Has Been Successfully Installed</strong>
		</div>
	</div>

	<div id="mainContent">
		<div id="right-info-panel">
			<div class="info-pad">
				<i>IE Tab +</i> (formerly Coral IE Tab) is an open source project, but if you find it useful, you can help support its continued development by making a small contribution.
				<p>
	  				<form action="https://www.paypal.com/cgi-bin/webscr" method="post">
					<input type="hidden" name="cmd" value="_s-xclick">
					<input type="hidden" name="hosted_button_id" value="2A9CQT2GHBAMN">
					<input type="image" src="https://www.paypal.com/en_US/i/btn/btn_donateCC_LG.gif" border="0" name="submit" alt="PayPal - The safer, easier way to pay online!">
					<img alt="" border="0" src="https://www.paypal.com/en_US/i/scr/pixel.gif" width="1" height="1">
					</form>
				</p>
			</div>

			<?php
			/*
			if ( strcmp($pending, $curver) != 0 ) {
				echo '<div class="new-tab-king-pad">';
				echo '<a href="#" onclick="install(event);"><img src="res/screencap-small5.jpg" alt="New Tab King" style="border: 0pt none ;"></a><br>';
				echo 'Make your blank new tab a useful tool with your most used sites, an easy search and more<br/>- try <a style="color: white; text-decoration: none; font-weight: bold;" href="#" onclick="install(event);">New Tab King</a> add-on!<br/>';
				echo '<a href="#" onclick="install(event);"><img title="Recommended Add-on: New Tab King" src="res/ntkinstbtn.png" style="border: 0pt none ;"></a>';
				echo '</div>';
			}
			*/
			?>
			<p></p>

			<div class="info-pad">
	  			<h3><i>IE Tab +</i> Links</h3>
	  			<ul class="links">
					<li><a href="http://coralietab.mozdev.org/">Project Home</a></li>
					<li><a href="http://coralietab.mozdev.org/source.html">Source Code</a></li>
					<li><a href="http://coralietab.mozdev.org/drupal/blog">Blog</a></li>
					<li><a href="http://coralietab.mozdev.org/bugs.html">BugZilla</a></li>
					<li><a href="http://www.facebook.com/sharer.php?u=https://addons.mozilla.org/firefox/addon/52809">Share on Facebook</a></li>
					<li><a href="http://twitter.com/home?status=IE+Tab+Plus%3A+get+rid+of+IE%21+http%3A%2F%2Faddons.mozilla.org%2Ffirefox%2Faddon%2F52809">Share on Twitter</a></li>
				</ul>
			</div>
		</div>

		<div id="top-middle">
			<?php
			if ( strcmp($latest, $curver) > 0 ) {
				echo '<div id="new-ver-excl">';
	  		echo '<img src="res/excl.jpg" alt="Warning!" border="0">';
				echo 'IE Tab + v' . $latest . ' is available.<br/>It is <b>strongly recommended</b> to update to the latest version:';
				echo '<a href="#" onclick="update(event);"><img src="res/instbtn.png" alt="Install Latest Version" title="Update to the latest version" style="border: 0pt none ; margin-top: 10px;"></a>';
				echo '</div>';
			}
			?>
			
			<div class="section-topper"><strong>What's New In v2.04.20110724:</strong></div>
			<ul>
			<li>[BUG FIX] Fixed compatible issues with Firefox 8.0a1. See bug <a href="https://www.mozdev.org/bugs/show_bug.cgi?id=24195">#24195</a>, <a href="https://www.mozdev.org/bugs/show_bug.cgi?id=24214">#24214</a>.</li>
			</ul>
			
			<div class="section-topper"><strong>What's New In v2.03.20110625:</strong></div>
			<ul>
			<li>Offical support for Firefox 5 and SeaMonkey 2.1.</li>
			</ul>
	
			<div class="section-topper"><strong>What's New In v2.02.20110525:</strong></div>
			<ul>
			<li>[BUG FIX] 'Open In New Window' opens FF Window without Toolbars. See bug <a href="https://www.mozdev.org/bugs/show_bug.cgi?id=23756">#23756</a>.</li>
			<li>[BUG FIX] Personal Bookmarks Context Menu items not visible on Firefox 4. See bug <a href="https://www.mozdev.org/bugs/show_bug.cgi?id=23674">#23674</a>.</li>
			</ul>
			
			<div class="section-topper"><strong>What's New In v2.01.20110417:</strong></div>
			<ul>
			<li>[BUG FIX] Status bar or toolbar icon doesn't reflect current engine. See bug <a href="https://www.mozdev.org/bugs/show_bug.cgi?id=23761">#23761</a>.</li>
			<li>[BUG FIX] Unnecessary authentication dialog appears. See bug <a href="https://www.mozdev.org/bugs/show_bug.cgi?id=23911">#23911</a>.</li>
			</ul>

			<div class="section-topper"><strong>What's New In v2.00.20110326:</strong></div>
			<ul>
			<li>[BUG FIX] After switching to IE the URL bar of Firefox stops working. See bug <a href="https://www.mozdev.org/bugs/show_bug.cgi?id=23763">#23763</a>.</li>
			<li>Updated Romanian localization, thanks Cãtãlin ZAMFIRESCU.</li>
			</ul>
			
			<div class="section-topper"><strong>What's New In v1.99.20110227:</strong></div>
			<ul>
			<li>[BUG FIX] Filter rules may fail if browser already opened. See bug <a href="https://www.mozdev.org/bugs/show_bug.cgi?id=23666">#23666</a>.</li>
			<li>[BUG FIX] FF opens a shortcut in a new Windows and in a new Tab. See bug <a href="https://www.mozdev.org/bugs/show_bug.cgi?id=23723">#23723</a>.</li>
			</ul>
			
			<div class="section-topper"><strong>What's New In v1.98.20110116:</strong></div>
			<ul>
			<li>As many users demanded, reverting the toolbar icon back to the original style.</li>
			<li>Support KeePass' Auto-Type feature. See bug <a href="https://www.mozdev.org/bugs/show_bug.cgi?id=23199">#23199</a>.</li>
			<li>The status bar icon now reflects the same status as the toolbar icon. See bug <a href="https://www.mozdev.org/bugs/show_bug.cgi?id=22790">#22790</a>.</li>
			<li>[BUG FIX] Input loses focus when switched out and back to the IE Tab. See bug <a href="https://www.mozdev.org/bugs/show_bug.cgi?id=23407">#23407</a>.</li>
			</ul>

			<div class="section-topper"><strong>What's New In v1.97.20101222:</strong></div>
			<ul>
			<li>[NEW] Supports Adblock Plus 1.3.x.</li>
			<li>[IMPROVEMENT] Improved compatibility of the 'Sync cookie' feature.</li>
			<li>[IMPROVEMENT] Improved compatibility with Firefox 4.0b8 and 4.0b9pre.</li>
			</ul>
			
			<div class="section-topper"><strong>What's New In v1.96.20101021:</strong></div>
			<ul>
			<li>[IMPROVEMENT] Improved reliability of auto-switching for file:// protocol.</li>
			<li>[BUG FIX] Fixed "Link works only one time" bug, see bug <a href="https://www.mozdev.org/bugs/show_bug.cgi?id=22677">#22677</a>.</li>
			</ul>
			
			<div class="section-topper"><strong>What's New In v1.95.20100930:</strong></div>
			<ul>
			<li>[BUG FIX] 'Sync Cookie' features stops working since Firefox 4.0b5.</li>
			<li>[BUG FIX] IE Tab + does not properly handle redirects. See bug <a href="https://www.mozdev.org/bugs/show_bug.cgi?id=23146">#23146</a>.</li>
			<li>[BUG FIX] IE Tab + fails on Minefield 4.0b7pre. See bug <a href="https://www.mozdev.org/bugs/show_bug.cgi?id=23268">#23268</a>.</li>
			<li>[BUG FIX] External links open in a new window instead of a new tab. See bug <a href="https://www.mozdev.org/bugs/show_bug.cgi?id=23286">#23286.</a></li>
			</ul>
			
			<div class="section-topper"><strong>What's New In v1.94.20100904:</strong></div>
			<ul>
				<li><b>No Adware, No Spyware, </b>this version is as clean as the old school.</li>
				<li>Improved compatibility with Firefox 4.0 Beta. See bug <a href="https://www.mozdev.org/bugs/show_bug.cgi?id=23063">#23063</a>, <a href="https://www.mozdev.org/bugs/show_bug.cgi?id=23062">#23062</a> and <a href="https://www.mozdev.org/bugs/show_bug.cgi?id=23106">#23106</a>.</li>
				<li>[BUG FIX] Hang of FF after entering an URL in a window that already is running with IE engine. See bug <a href="https://www.mozdev.org/bugs/show_bug.cgi?id=23069">#23069</a></li>
			</ul>
			
			<div class="section-topper"><strong>What's New In v1.93.20100725:</strong></div>
			<ul>
				<li>[BUG FIX] Fixed OS version detection bug. See bug <a href="https://www.mozdev.org/bugs/show_bug.cgi?id=22986">#22986</a></li>
				<li>[BUG FIX] Adblock Plus may block some request inproperly. See bug <a href="https://www.mozdev.org/bugs/show_bug.cgi?id=23023">#23023</a></li>
				<li>[BUG FIX] Back doesn't work across Firefox to IE transition. See bug <a href="https://www.mozdev.org/bugs/show_bug.cgi?id=22963">#22963</a></li>
				<li>[NEW] Add a built-in Window Shopper online utility (See "ExternalApplications" in options)</li>
				<li>Updated Spanish locale, thanks J.I. Plaza.</li>
			</ul>
			
			<div class="section-topper"><strong>What's New In v1.92.20100607:</strong></div>
			<ul>
				<li>[BUG FIX] Copy shortcut may contain login details. See bug <a href="https://www.mozdev.org/bugs/show_bug.cgi?id=22864">#22864</a></li>
				<li>[BUG FIX] Previous version stops working on some language versions of Windows XP.</li>
				<li>[BUG FIX] Back/Forward button is not working in SeaMonkey.</li>
				<li>Updated Czech locale, thanks xenophex.</li>
			</ul>
			
			<div class="section-topper"><strong>What's New In v1.91.20100528:</strong></div>
			<ul>
				<li>Improved stability in Firefox 3.6.4 when OOPP is enabled.</li>
				<li>Improved compatibility with SeaMonkey 2.0.4+.</li>
				<li>[BUG FIX] IE Tab is not displayed correctly if Firefox resizes. Reported by DaveM.</li>
				<li>[BUG FIX] Alt+Left Arrow/Right Arrow will let IE Tab go back/forward twice.</li>
				<li>[BUG FIX] Some Latvian language special characters via AltGr can not be typed. See bug <a href="https://www.mozdev.org/bugs/show_bug.cgi?id=22839">#22839</a></li>
				<li>[MINOR] Refined description and icon picture in SeaMonkey. See bug <a href="https://www.mozdev.org/bugs/show_bug.cgi?id=22799">#22799</a></li>
			</ul>
			
			<div class="section-topper"><strong>What's New In v1.90.20100517:</strong></div>
			<ul>
				<li>Completely rewritten Adblock Plus support, better performance, much more stable.</li>
				<li>[IMPROVEMENT] Private browsing warning in IE Tab can be turned off.</li>
				<li>[BUG FIX] In private browsing mode, IE Tab will generate an XML error in German version of Firefox. See bug <a href="https://www.mozdev.org/bugs/show_bug.cgi?id=22820">#22820</a></li>
				<li>[BUG FIX] Previous version stops working in SeaMonkey. See bug <a href="https://www.mozdev.org/bugs/show_bug.cgi?id=22798">#22798</a></li>
				<li>[BUG FIX] Crashes in Firefox v3.6.4 if OOPP is enabled. See bug <a href="https://www.mozdev.org/bugs/show_bug.cgi?id=22720">#22720</a></li>
				<li>[BUG FIX] Coral IE Tab opens two tabs when changing browser. See bug <a href="https://www.mozdev.org/bugs/show_bug.cgi?id=22818">#22818</a></li>
			</ul>
			
			<div class="section-topper"><strong>What's New In v1.85.20100407:</strong></div>
			<ul>
				<li>[NEW] Detect "Private Browsing" mode of Firefox and remind users for privacy.</li>
				<li>[BUG FIX] 'Switch back to Firefox' feature does not working properly since v1.80.</li>
				<li>[BUG FIX] UNC path such as \\fileserver\folder will be properly displayed in address bar.</li>
				<li>[BUG FIX] Switching to IE will only result a black page in Windows 2000. See bug <a href="https://www.mozdev.org/bugs/show_bug.cgi?id=22581">#22581</a>.</li>
				<li>Added Czech locale, thanks tomdlouhan.</li>
			</ul>
			
			<div class="section-topper"><strong>What's New In v1.83.20100316:</strong></div>
			<ul>
				<li>[BUG FIX] Viewing an link in the Ext. App. will fail in the 2nd time. Reported by David Masterson.</li>
				<li>[BUG FIX] Sometimes shortcut keys may not function properly. See bug <a href="https://www.mozdev.org/bugs/show_bug.cgi?id=22586">#22586</a> and <a href="https://www.mozdev.org/bugs/show_bug.cgi?id=22589">#22589</a>.</li>
				<li>[BUG FIX] Cursor stopped on selection box in HTML form. See bug <a href="https://www.mozdev.org/bugs/show_bug.cgi?id=22580">#22580</a>.</li>
				<li>Updated Korean locale, thanks 베르짱.</li>
			</ul>
			
			<div class="section-topper"><strong>What's New In v1.82.20100312:</strong></div>
			<ul>
				<li>[IMPROVEMENT] As many people suggested, popup window will be opened in tabs defaultly.</li>
				<li>[IMPROVEMENT] Added a menu entry for filter editor.</li>
				<li>Added Hungarian locale, thanks Kozák Csaba.</li>
				<li>Added Portuguese (Portugal) locale, thanks mrfyda.</li>
				<li>Updated Japanese locale, thanks papino0319.</li>
			</ul>

			<div class="section-topper"><strong>What's New In v1.81.20100304:</strong></div>
			<ul>
				<li>[IMPROVEMENT] As many people suggested, New popup window in IE engine will be shown in Firefox window.</li>
	  			<li>[NEW] Support window.external, which is often used in IE detection. See bug <a href="https://www.mozdev.org/bugs/show_bug.cgi?id=22507">#22507</a>.</li>
	  			<li>Added Hebrew locale, thanks SiiiE.</li>
				<li>Added Vietnamese locale, thanks longnh.</li>
				<li>Added Brazilian Portuguese locale, thanks pcbossa and edgard.magalhaes.</li>
			</ul>
			
			<div class="section-topper"><strong>What's New In v1.80.20100224:</strong></div>
			<ul>
				<li>[NEW] Users can now select "Classical Mode" and "Advanced Mode". IE Tab + will behave like the old IE Tab if users select "Classical Mode".</li>
				<li>[BUG FIX] In some situations popup window will not work properly. See bug <a href="https://www.mozdev.org/bugs/show_bug.cgi?id=22379">#22379</a>.</li>
				<li>Fixed Firefox 3.7a2pre support.</li>
				<li>Added Japanese locale, thanks Kazuhiro Hiratsuka and Aozora&Genei.</li>
				<li>Added Italian locale, thanks AlexV.</li>
				<li>Added Polish locale, thanks Leszek(teo)?yczkowski.</li>
				<li>Added Slovak locale, thanks sklado.</li>
				<li>Added Dutch locale, thanks Alf.</li>
			</ul>
		</div>
	</div>
<script type="text/javascript">
var gaJsHost = (("https:" == document.location.protocol) ? "https://ssl." : "http://www.");
document.write(unescape("%3Cscript src='" + gaJsHost + "google-analytics.com/ga.js' type='text/javascript'%3E%3C/script%3E"));
</script>
<script type="text/javascript">
try {
var pageTracker = _gat._getTracker("UA-15769165-1");
pageTracker._trackPageview();
} catch(err) {}</script>
</body>
</html>
