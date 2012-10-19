/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is IETab. Modified In Coral IETab.
 *
 * The Initial Developer of the Original Code is yuoo2k <yuoo2k@gmail.com>.
 * Modified by quaful <quaful@msn.com>.
 *
 * Portions created by the Initial Developer are Copyright (C) 2006-2008
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * ***** END LICENSE BLOCK ***** */
 
const gIeTabChromeStr = "chrome://coralietab/content/container.html?url=";
const gIeTabDefaultFlag = 0x4003;		// 0x4000 means manual switching, 0x0003(0x0001|0x0002) means sync cookies and user-agent
const browserAttr = "ietabManualSwitch";

IeTab.prototype.QueryInterface = function(aIID) {
   if (aIID.equals(Components.interfaces.nsIIeTab) || aIID.equals(Components.interfaces.nsISupports))
      return gIeTab;
   throw Components.results.NS_NOINTERFACE;
}

IeTab.prototype.getIeTabURL = function(url, flags) {
   if (gIeTab.startsWith(url, gIeTabChromeStr)) return url;
   if (/^file:\/\/.*/.test(url)) try { url = decodeURI(url).replace(/\|/g,":"); }catch(e){}
   flags = ( typeof(flags) != 'undefined' ? flags : gIeTabDefaultFlag );
   return gIeTabChromeStr + flags + ',' + encodeURI(url);
}

IeTab.prototype.getIeTabTrimURL = function(url) {
   if (url && url.length>0) {
      url = url.replace(/^\s+/g,"").replace(/\s+$/g,"");
      if (/^file:\/\/.*/.test(url)) url = url.replace(/\|/g,":");
      if (url.substr(0, gIeTabChromeStr.length) == gIeTabChromeStr) {
      	var flag_pos = url.indexOf(',', gIeTabChromeStr.length);
      	if (flag_pos>0)
         url = decodeURI(url.substring(flag_pos+1));
        else
         url = decodeURI(url.substring(gIeTabChromeStr.length+1));
        
        if (!/^[\w]+:/.test(url)) {
        	url = "http://"+url;
        }
      }
   }
   return url;
}

IeTab.prototype.getIeTabElmt = function(aTab) {
   var aBrowser = (aTab ? aTab.linkedBrowser : gBrowser);
   if (aBrowser && aBrowser.currentURI && gIeTab.startsWith(aBrowser.currentURI.spec, gIeTabChromeStr)) {
      if (aBrowser.contentDocument && aBrowser.contentDocument.getElementById('IETab')){
         var obj = aBrowser.contentDocument.getElementById('IETab');
         return (obj.wrappedJSObject ? obj.wrappedJSObject : obj);		// Ref: Safely accessing content DOM from chrome
      }
   }
   return null;
}

IeTab.prototype.getIeTabElmtURL = function(aTab) {
   var aBrowser = (aTab ? aTab.linkedBrowser : gBrowser);
   var url = gIeTab.getIeTabTrimURL(aBrowser.currentURI.spec);
   var ietab = gIeTab.getIeTabElmt(aTab);
   if (ietab && ietab.url && ietab.url != "") {
      url = (/^file:\/\/.*/.test(url) ? encodeURI(gIeTab.convertToUTF8(ietab.url)) : ietab.url);
   }
   return url;
}

IeTab.prototype.isIeForceable = function(url) {
   return(url && (url.length>0) &&
             ((url=="about:blank") ||
              gIeTab.startsWith(url, 'http://') ||
              gIeTab.startsWith(url, 'https://') ||
              gIeTab.startsWith(url, 'file://') ||
              gIeTab.startsWith(url, 'ftp://')
             )
         );
}

IeTab.prototype.isIeEngine = function() {
   return gIeTab.getIeTabElmt();
}

IeTab.prototype.switchTabEngine = function(aTab, isOpenNewTab, flags) {
	if (aTab && aTab.localName == "tab") {
		var url = gIeTab.getIeTabElmtURL(aTab);
		var ietab = gIeTab.getIeTabElmt(aTab);
		if (ietab) {
			// Now it is IE engine, call me means users want to switch to Firefox engine.
			// We have to tell iewatch component that this is manual switching, do not switch back to IE engine
			if (aTab.linkedBrowser) {
				var node = aTab.linkedBrowser.QueryInterface(Components.interfaces.nsIDOMNode);
				if (node) node.setAttribute(browserAttr, true);		// Leave a mark for iewatch component
			}
		}
		else {
			if ( gIeTab.startsWith(url, "http") ) {
				var authMgr = Components.classes['@mozilla.org/network/http-auth-manager;1'].getService(Components.interfaces.nsIHttpAuthManager);
				const ioService = Components.classes["@mozilla.org/network/io-service;1"].getService(Components.interfaces.nsIIOService);
				try {
					var uri = ioService.newURI(url, null, null);
					var userDomain = {};
					var userName = {};
					var password = {};

					authMgr.getAuthIdentity(uri.scheme, uri.host, uri.port < 0 ? (uri.scheme == "https" ? 443 : 80) : uri.port, null, null, uri.path, userDomain, userName, password);
					uri.username = userName.value;
					uri.password = password.value;
					url = uri.spec;
     			}
     			catch (e) {}
     		}
			url = gIeTab.getIeTabURL(url, flags);
		}
      gBrowser.mIeTabSwitchURL = url;
      if (isOpenNewTab) {
         var newTab = gBrowser.addTab(url);
         var focustab = gIeTab.getBoolPref("coral.ietab.focustab", true);
         if (focustab) gBrowser.selectedTab = newTab;
      } else {
      	if (aTab.linkedBrowser) aTab.linkedBrowser.loadURI(url);
      }
      gBrowser.mIeTabSwitchURL = null;
   }
}

IeTab.prototype.switchEngine = function(isOpenNewTab, flags) {
   gIeTab.switchTabEngine(gBrowser.mCurrentTab, isOpenNewTab, flags);
}

IeTab.prototype.openPrefDialog = function(url) {
   if (!url) url = gIeTab.getIeTabElmtURL();
   var icon = document.getElementById('ietab-status');
   window.openDialog('chrome://coralietab/content/ietabSetting.xul', "ietabPrefDialog",
      'chrome,centerscreen,dependent', gIeTab.getUrlDomain(url), icon);
}

IeTab.prototype.loadInExtApp = function(url) {
   if (/^file:\/\/.*/.test(url)) try { url = decodeURI(url).substring(8).replace(/\//g, "\\"); }catch(e){}
   url = gIeTab.convertToASCII(url);
   var param = gIeTab.getStrPref("coral.ietab.extAppParam", "%1").replace(/%1/g, url);
   var path = gIeTab.getStrPref("coral.ietab.extAppPath", "");
   return IeTabExtApp.runApp(path, param);
}

IeTab.prototype.viewPageInExtApp = function(aTab) {
   return gIeTab.loadInExtApp(gIeTab.getIeTabElmtURL(aTab));
}

IeTab.prototype.viewLinkInExtApp = function() {
   return gIeTab.loadInExtApp(gIeTab.getContextLinkURL());
}

IeTab.prototype.clickButton = function(e) {
	if (e.button == 0) {
		if (e.ctrlKey) {
			var ctrlExtApp = gIeTab.getBoolPref("coral.ietab.ctrlclick", true);
			if (ctrlExtApp ) gIeTab.viewPageInExtApp();
			return;
		}
		gIeTab.switchEngine(e.ctrlKey || gIeTab.getBoolPref("coral.ietab.alwaysNewTab", false));
	}
	if (e.button == 1) gIeTab.switchEngine(true);
	if (e.button == 2) {
		document.getElementById("menu_context").openPopup(e.target, "before_start", 0, 0, true, false);
	}
	e.preventDefault();
}

IeTab.prototype.getContextLinkURL = function() {
   return (gContextMenu ? gContextMenu.link.toString() : null);
}

IeTab.prototype.loadIeTab = function(url, flags) {
   gBrowser.loadURI(gIeTab.getIeTabURL(url, flags));
}

IeTab.prototype.addIeTab = function(url, flags) {
   var newTab = gBrowser.addTab(gIeTab.getIeTabURL(url, flags));
   var focustab = gIeTab.getBoolPref("coral.ietab.focustab", true);
   if (focustab) {
      gBrowser.selectedTab = newTab;
      if (gURLBar && (url == 'about:blank'))
         window.setTimeout(function(){ gURLBar.focus(); }, 0);
   }
}

IeTab.prototype.ietabContextMenuPopup = function(e) {
	if (e.originalTarget != document.getElementById("contentAreaContextMenu")) return;
	if (!gContextMenu) return;

	var hide4Page = gContextMenu.isTextSelected || gContextMenu.onLink || gContextMenu.onImage || gContextMenu.onTextInput;
	var hide4Link = (!gContextMenu.onLink) || (!gIeTab.isIeForceable(gIeTab.getContextLinkURL())); //if link is javascript

	var internal = gIeTab.getBoolPref("coral.ietab.pagelink", true);
	var external = gIeTab.getBoolPref("coral.ietab.pagelink.extapp", true);
	var showicon = gIeTab.getBoolPref("coral.ietab.icon.pagelink", false);

	var menuitem = null;
	
	//click on page
	menuitem = document.getElementById("ietab-viewpage");
	menuitem.hidden = hide4Page || !internal;
	menuitem = document.getElementById("ietab-viewpage-sep");
	menuitem.hidden = hide4Page || (!internal && !external);
	
	var popupmenu = null;
	var menu_source = null;
	var menu_dest = null;
	
	popupmenu = document.getElementById('ietab-viewpage-menu');
	if (popupmenu.hasChildNodes()) {
		menuitem = document.getElementById('context_switchEngine');
		if ( menuitem ) menuitem.setAttribute("class", (showicon ? document.getElementById('menu_switchEngine').getAttribute("iconic") : ""));
		menuitem = document.getElementById('context_ietab-viewpage-extapp');
		if ( menuitem ) menuitem.setAttribute("class", (showicon ? document.getElementById('ietab-viewpage-extapp').getAttribute("iconic") : ""));
	}
	else {
		menu_source = document.getElementById('menu_switchEngine');
		if ( menu_source ) {
			menu_dest = document.createElement("menuitem");
			menu_dest.setAttribute("id", "context_switchEngine");
			menu_dest.setAttribute("label", menu_source.getAttribute("label"));
			menu_dest.setAttribute("class", (showicon ? menu_source.getAttribute("iconic") : ""));
			menu_dest.setAttribute("accesskey", menu_source.getAttribute("accesskey"));
			menu_dest.setAttribute("oncommand", menu_source.getAttribute("oncommand"));
			popupmenu.appendChild(menu_dest);
		}
		
		menu_source = document.getElementById('menu_switchEngine_NoCookie');
		if ( menu_source ) {
			menu_dest = document.createElement("menuitem");
			menu_dest.setAttribute("label", menu_source.getAttribute("label"));
			menu_dest.setAttribute("accesskey", menu_source.getAttribute("accesskey"));
			menu_dest.setAttribute("oncommand", menu_source.getAttribute("oncommand"));
			popupmenu.appendChild(menu_dest);
		}
		
		menu_source = document.getElementById('menu_switchEngine_SafeMode');
		if ( menu_source ) {
			menu_dest = document.createElement("menuitem");
			menu_dest.setAttribute("label", menu_source.getAttribute("label"));
			menu_dest.setAttribute("accesskey", menu_source.getAttribute("accesskey"));
			menu_dest.setAttribute("oncommand", menu_source.getAttribute("oncommand"));
			popupmenu.appendChild(menu_dest);
		}
		
		menu_source = document.getElementById('ietab-viewpage-extapp');
		if ( menu_source ) {
			menu_dest = document.createElement("menuitem");
			menu_dest.setAttribute("id", "context_ietab-viewpage-extapp");
			// menu_dest.setAttribute("label", menu_source.getAttribute("label"));
			menu_dest.setAttribute("class", (showicon ? menu_source.getAttribute("iconic") : ""));
			menu_dest.setAttribute("accesskey", menu_source.getAttribute("accesskey"));
			menu_dest.setAttribute("oncommand", menu_source.getAttribute("oncommand"));
			popupmenu.appendChild(menu_dest);
		}
	}

	//click on link
	menuitem = document.getElementById("ietab-viewlink");
	menuitem.hidden = hide4Link || !internal;
   
	popupmenu = document.getElementById('ietab-viewlink-menu');
	if (popupmenu.hasChildNodes()) {
		menuitem = document.getElementById('link_switchEngine');
		if ( menuitem ) menuitem.setAttribute("class", (showicon ? document.getElementById('menu_switchEngine').getAttribute("iconic") : ""));
		
		menuitem = document.getElementById('link_ietab-viewpage-extapp');
		if ( menuitem ) menuitem.setAttribute("class", (showicon ? document.getElementById('ietab-viewpage-extapp').getAttribute("iconic") : ""));
	}
	else {
		menu_source = document.getElementById('menu_switchEngine');
		if ( menu_source ) {
			menu_dest = document.createElement("menuitem");
			menu_dest.setAttribute("id", 'link_switchEngine');
			menu_dest.setAttribute("label", menu_source.getAttribute("label"));
			menu_dest.setAttribute("class", (showicon ? menu_source.getAttribute("iconic") : ""));
			menu_dest.setAttribute("accesskey", menu_source.getAttribute("accesskey"));
			menu_dest.setAttribute("oncommand", "gIeTab.viewLink(event, 0x0003);");
			popupmenu.appendChild(menu_dest);
		}
		
		menu_source = document.getElementById('menu_switchEngine_NoCookie');
		if ( menu_source ) {
			menu_dest = document.createElement("menuitem");
			menu_dest.setAttribute("label", menu_source.getAttribute("label"));
			menu_dest.setAttribute("accesskey", menu_source.getAttribute("accesskey"));
			menu_dest.setAttribute("oncommand", "gIeTab.viewLink(event, 0x4000);");
			popupmenu.appendChild(menu_dest);
		}
		
		menu_source = document.getElementById('menu_switchEngine_SafeMode');
		if ( menu_source ) {
			menu_dest = document.createElement("menuitem");
			menu_dest.setAttribute("label", menu_source.getAttribute("label"));
			menu_dest.setAttribute("accesskey", menu_source.getAttribute("accesskey"));
			menu_dest.setAttribute("oncommand", "gIeTab.viewLink(event, 0x4001);");
			popupmenu.appendChild(menu_dest);
		}
		
		menu_source = document.getElementById('ietab-viewpage-extapp');
		if ( menu_source ) {
			menu_dest = document.createElement("menuitem");
			menu_dest.setAttribute("id", 'link_ietab-viewpage-extapp');
			// menu_dest.setAttribute("label", menu_source.getAttribute("label"));
			menu_dest.setAttribute("class", (showicon ? menu_source.getAttribute("iconic") : ""));
			menu_dest.setAttribute("accesskey", menu_source.getAttribute("accesskey"));
			menu_dest.setAttribute("oncommand", "gIeTab.viewLinkInExtApp();");
			popupmenu.appendChild(menu_dest);
		}
	}
	
	gIeTab.updateContextMenu();
}

IeTab.prototype.getHandledURL = function(url, isModeIE) {
	url = gIeTab.trim(url);
	if (isModeIE) return gIeTab.getIeTabURL(url);
	if ( gIeTab.isIeEngine() &&
		(!gIeTab.startsWith(url, "about:")) && 
		(!gIeTab.startsWith(url, "view-source:"))
		) {
			if ( isValidURL(url) || isValidDomainName(url) ) {
				var isBlank = (gIeTab.getIeTabTrimURL(gBrowser.currentURI.spec)=="about:blank");
				var handleUrlBar = gIeTab.getBoolPref("coral.ietab.handleUrlBar", false);
				var isSimilar = (gIeTab.getUrlDomain(gIeTab.getIeTabElmtURL()) == gIeTab.getUrlDomain(url));
				if (isBlank || handleUrlBar || isSimilar) return gIeTab.getIeTabURL(url);
			}
		}
	
	return url;
}

IeTab.prototype.updateUrlBar = function() {
   if (!gURLBar || !gIeTab.isIeEngine()) return;
   if (gBrowser.userTypedValue) {
      if (gURLBar.selectionEnd != gURLBar.selectionStart)
         window.setTimeout(function(){ gURLBar.focus(); }, 0);
   } else {
      var url = gIeTab.getIeTabElmtURL();
      if (url == "about:blank") url = "";
      if (gURLBar.value != url) gURLBar.value = url;
   }
}

IeTab.prototype.updateToolButton = function() {
   var btn = document.getElementById("ietab-button");
   if (btn) {
      btn.setAttribute("engine", (gIeTab.isIeEngine()?"ie":"fx"));
   }
}

IeTab.prototype.updateStatusIcon = function() {
   var img = document.getElementById("ietab-status-image");
   if (img) {
      img.setAttribute("engine", (gIeTab.isIeEngine()?"ie":"fx"));

      var show = gIeTab.getBoolPref("coral.ietab.statusbar", true);
      var icon = document.getElementById('ietab-status');
      if (icon && show) {
         icon.removeAttribute("hidden");
      }else{
         icon.setAttribute("hidden", true);
      }
   }
}

IeTab.prototype.updateObjectDisabledStatus = function(objId, isEnabled) {
   var obj = ( typeof(objId)=="object" ? objId : document.getElementById(objId) );
   if (obj) {
      var d = obj.hasAttribute("disabled");
      if (d == isEnabled) {
         if (d) obj.removeAttribute("disabled");
         else obj.setAttribute("disabled", true);
      }
   }
}

IeTab.prototype.updateBackForwardButtons = function() {
   try {
      var ietab = gIeTab.getIeTabElmt();
      var canBack = (ietab ? ietab.canBack : false) || gBrowser.webNavigation.canGoBack;
      var canForward = (ietab ? ietab.canForward : false) || gBrowser.webNavigation.canGoForward;
      gIeTab.updateObjectDisabledStatus("Browser:Back", canBack);
      gIeTab.updateObjectDisabledStatus("Browser:Forward", canForward);
   }catch(e){}
}

IeTab.prototype.updateStopReloadButtons = function() {
   try {
      var ietab = gIeTab.getIeTabElmt();
      var isBlank = (gBrowser.currentURI.spec == "about:blank");
      var isLoading = gBrowser.mIsBusy;
      gIeTab.updateObjectDisabledStatus("Browser:Reload", ietab ? ietab.canRefresh : !isBlank);
      gIeTab.updateObjectDisabledStatus("Browser:Stop", ietab ? ietab.canStop : isLoading);
   }catch(e){}
}

IeTab.prototype.updateGoMenuItems = function(e) {
   var goPopup = document.getElementById("goPopup");
   if (!goPopup || (e.originalTarget != goPopup)) return;
   try {
      var ietab = gIeTab.getIeTabElmt();
      var canBack = (ietab ? ietab.canBack : false) || gBrowser.webNavigation.canGoBack;
      var canForward = (ietab ? ietab.canForward : false) || gBrowser.webNavigation.canGoForward;
      var goBack = goPopup.getElementsByAttribute("key","goBackKb");
      if (goBack) gIeTab.updateObjectDisabledStatus(goBack[0], canBack);
      var goForward = goPopup.getElementsByAttribute("key","goForwardKb");
      if (goForward) gIeTab.updateObjectDisabledStatus(goForward[0], canForward);
   }catch(e){}
}

IeTab.prototype.updateEditMenuItems = function(e) {
   if (e.originalTarget != document.getElementById("menu_Edit_Popup")) return;
   var ietab = gIeTab.getIeTabElmt();
   if (ietab) {
      gIeTab.updateObjectDisabledStatus("cmd_cut", ietab.canCut);
      gIeTab.updateObjectDisabledStatus("cmd_copy", ietab.canCopy);
      gIeTab.updateObjectDisabledStatus("cmd_paste", ietab.canPaste);
   }
}

IeTab.prototype.updateToolsMenuItem = function(e) {
   if (e.originalTarget != document.getElementById("taskPopup")) return;
   var menuitem = document.getElementById("ietab-toolsmenu");
   if (menuitem) {
      var showitem = gIeTab.getBoolPref("coral.ietab.toolsmenu", true);
      var showicon = gIeTab.getBoolPref("coral.ietab.toolsmenu.icon", false);
      menuitem.hidden = !showitem;
      menuitem.setAttribute("class", (showicon?menuitem.getAttribute("iconic"):""));
   }
}

IeTab.prototype.updateSecureLockIcon = function() {
   var ietab = gIeTab.getIeTabElmt();
   if (ietab) {
      var url = ietab.url;
      const wpl = Components.interfaces.nsIWebProgressListener;
      var state = (gIeTab.startsWith(url, "https://") ? wpl.STATE_IS_SECURE | wpl.STATE_SECURE_HIGH : wpl.STATE_IS_INSECURE);
      window.XULBrowserWindow.onSecurityChange(null, null, state);
      var securityButton = document.getElementById("security-button");
      securityButton.setAttribute("label", gIeTab.getUrlHost(ietab.url));
   }
}

IeTab.prototype.updateInterface = function() {
   gIeTab.updateStatusIcon();
   gIeTab.updateToolButton();
   gIeTab.updateBackForwardButtons();
   gIeTab.updateStopReloadButtons();
   gIeTab.updateSecureLockIcon();
   gIeTab.updateUrlBar();
}

IeTab.prototype.updateAll = function() {
   if (gIeTab.updating) return;
   try {
      gIeTab.updating = true;
      gIeTab.updateInterface();
   } finally {
      delete gIeTab.updating;
   }
}

IeTab.prototype.updateProgressStatus = function() {
   var mTabs = gBrowser.mTabContainer.childNodes;
   for(var i = 0 ; i < mTabs.length ; i++) {
      if (mTabs[i].localName == "tab") {
         var ietab = gIeTab.getIeTabElmt(mTabs[i]);
         if (ietab) {
            var aCurTotalProgress = ietab.progress;
            if (aCurTotalProgress != mTabs[i].mProgress) {
               const ios = Components.classes["@mozilla.org/network/io-service;1"].getService(Components.interfaces.nsIIOService);
               const wpl = Components.interfaces.nsIWebProgressListener;
               var aMaxTotalProgress = (aCurTotalProgress == -1 ? -1 : 100);
               var aTabListener = gBrowser.mTabListeners[mTabs[i]._tPos];
               var aWebProgress = mTabs[i].linkedBrowser.webProgress;
               var aRequest = ios.newChannelFromURI(mTabs[i].linkedBrowser.currentURI);
               var aStateFlags = (aCurTotalProgress == -1 ? wpl.STATE_STOP : wpl.STATE_START) | wpl.STATE_IS_NETWORK;
               aTabListener.onStateChange(aWebProgress, aRequest, aStateFlags, 0);
               aTabListener.onProgressChange(aWebProgress, aRequest, 0, 0, aCurTotalProgress, aMaxTotalProgress);
               mTabs[i].mProgress = aCurTotalProgress;
            }
         }
      }
   }
}

IeTab.prototype.onProgressChange = function(progress) {
   if (progress==0) gBrowser.userTypedValue = null;
   gIeTab.updateProgressStatus();
   gIeTab.updateAll();
}

IeTab.prototype.onSecurityChange = function(security) {
   gIeTab.updateSecureLockIcon();
}

IeTab.prototype.goDoCommand = function(cmd) {
	try {
		if (gBrowser.currentURI && gIeTab.startsWith(gBrowser.currentURI.spec, gIeTabChromeStr)) {
			var doc = gBrowser.contentDocument;
			if (doc) {
				var ietab = doc.getElementById('IETab');
				if (ietab && "createEvent" in doc) {
					var sendCmd = cmd;
					switch (cmd) {
						case "goBack":
							if (!ietab.wrappedJSObject.canBack) return false;
							break;
						case "goForward":
							if (!ietab.wrappedJSObject.canForward) return false;
							break;
						case "cmd_cut":
							sendCmd = "cut";
							break;
						case "cmd_copy":
							sendCmd = "copy";
							break;
						case "cmd_paste":
							sendCmd = "paste";
							break;
						case "cmd_selectAll":
							sendCmd = "selectAll";
							break;
						case "navigate":
						case "navigate2":
						case "refresh":
						case "stop":
						case "saveAs":
						case "print":
						case "printPreview":
						case "printSetup":
						case "cut":
						case "copy":
						case "paste":
						case "selectAll":
						case "find":
						case "viewSource":
						case "focus":
						case "displaySecurityInfo":    
							sendCmd = cmd;
							break;
						default:
							return false;
      				}

					ietab.setAttribute("ietabCommand", sendCmd);
					var evt = doc.createEvent("Events");
					evt.initEvent("OverlayCommand", true, false);
					ietab.dispatchEvent(evt);
					
					return true;
				}
			}
		}
	}
	finally {
		gIeTab.updateAll();
	}
	
	return false;
}

IeTab.prototype.addBookmarkMenuitem = function(e) {
   var popupMenu = e.originalTarget;
   if (popupMenu.id != "placesContext") return;

   var miInt = document.getElementById("ietab-bookmark");
   var miExt = document.getElementById("ietab-bookmark-extapp");

   var bmNode = document.popupNode.node;
   var isBookmark = bmNode && PlacesUtils.nodeIsBookmark(bmNode);
   var isShowIcon = gIeTab.getBoolPref("coral.ietab.icon.bookmark", false);

   miInt.hidden = !isBookmark || !gIeTab.getBoolPref("coral.ietab.bookmark", true);
   miExt.hidden = !isBookmark || !gIeTab.getBoolPref("coral.ietab.bookmark.extapp", true);
   if (!miInt.hidden) {
      miInt.setAttribute("oncommand", "gIeTab.addIeTab(\'"+bmNode.uri+"\');");
      miInt.setAttribute("class", (isShowIcon?miInt.getAttribute("iconic"):""));
   }
   if (!miExt.hidden) {
      miExt.setAttribute("oncommand", "gIeTab.loadInExtApp(\'"+bmNode.uri+"\');");
      miExt.setAttribute("class", (isShowIcon?miExt.getAttribute("iconic"):""));
   }
   
   gIeTab.updateContextMenu();
}

IeTab.prototype.closeIeTab = function() {
   var mTabs = gBrowser.mTabContainer.childNodes;
   for(var i = mTabs.length-1 ; i>=0 ; i--) {
      if (mTabs[i].localName == "tab") {
         var ietab = gIeTab.getIeTabElmt(mTabs[i]);
         if (ietab && (ietab.canClose)) {
         	window.setTimeout("gIeTab.closeTab("+i+");", 500);
         	break;
        }
      }
   }
}

IeTab.prototype.closeTab = function(i) {
	var mTabs = gBrowser.mTabContainer.childNodes;
	gBrowser.removeTab(mTabs[i]);
}

IeTab.prototype.getContextTab = function() {
   return  (gBrowser && gBrowser.mContextTab && (gBrowser.mContextTab.localName == "tab") ? gBrowser.mContextTab : null);
}

IeTab.prototype.viewLink = function(e, flags) {
	if (!gContextMenu) return;
	var url = gIeTab.getContextLinkURL();
	gIeTab.addIeTab(url, flags);
}

IeTab.prototype.viewPage = function(e) {
   var aTab = null;
   switch (e.originalTarget.id) {
   case "ietab-viewpage":
      aTab = gBrowser.mCurrentTab;
      break;
   case "ietab-tabbar-switch":
      aTab = gIeTab.getContextTab();
      break;
   }
   if (!aTab) return;

   switch (e.button) {
   case 0:
      if (e.ctrlKey) {
         var ctrlExtApp = gIeTab.getBoolPref("coral.ietab.ctrlclick", true);
         if (ctrlExtApp ? gIeTab.viewPageInExtApp(aTab) : false) return;
      }
      gIeTab.switchTabEngine(aTab, e.ctrlKey || gIeTab.getBoolPref("coral.ietab.alwaysNewTab", false));
      break;
   case 1:
      var menu = e.originalTarget;
      while (menu) {
         if (menu.localName == "menupopup") break;
         if (menu.localName == "popup") break;
         menu = menu.parentNode;
      }
      if (menu) menu.hidePopup();
      gIeTab.switchTabEngine(aTab, true);
      break;
   case 2:
      gIeTab.openPrefDialog(gIeTab.getIeTabElmtURL(aTab));
      break;
   }
}

IeTab.prototype.updateTabbarMenu = function(e) {
   if (e.originalTarget != gBrowser.mStrip.firstChild.nextSibling) return;

   var aTab = gIeTab.getContextTab();
   var hide = (aTab == null);

   var internal = gIeTab.getBoolPref("coral.ietab.tabsmenu", true);
   var external = gIeTab.getBoolPref("coral.ietab.tabsmenu.extapp", true);
   var showicon = gIeTab.getBoolPref("coral.ietab.icon.tabsmenu", false);

   var menuitem = null;

   //switch
   menuitem = document.getElementById("ietab-tabbar-switch");
   menuitem.hidden = hide || !internal;
   menuitem.setAttribute("class", (showicon?menuitem.getAttribute("iconic"):""));

   //extapp
   menuitem = document.getElementById("ietab-tabbar-extapp");
   menuitem.hidden = hide || !external;
   menuitem.setAttribute("class", (showicon?menuitem.getAttribute("iconic"):""));

   //sep
   menuitem = document.getElementById("ietab-tabbar-sep");
   menuitem.hidden = hide || (!internal && !external);

   if (aTab) {
      var ietab = gIeTab.getIeTabElmt(aTab);
      document.getElementById("ietab-tabbar-switch").setAttribute("engine", (ietab ? "ie" : "fx"));
   }
   
   gIeTab.updateContextMenu();
}

IeTab.prototype.statusbarContextMenuPopup = function() {
	var b = gIeTab.getIeTabElmt() != null;
	var showicon = gIeTab.getBoolPref("coral.ietab.icon.pagelink", false);	
	var menuitem = null;
	menuitem = document.getElementById("menu_switchEngine_NoCookie");
	if ( menuitem ) {
		menuitem.setAttribute("class", (showicon ? menuitem.getAttribute("iconic") : ""));
		menuitem.setAttribute("disabled", b);
	}
	menuitem = document.getElementById("menu_switchEngine_SafeMode");
	if ( menuitem ) {
		menuitem.setAttribute("class", (showicon ? menuitem.getAttribute("iconic") : ""));
		menuitem.setAttribute("disabled", b);
	}
	
	gIeTab.updateContextMenu();
}

IeTab.prototype.createTabbarMenu = function() {
   var tabbarMenu = gBrowser.mStrip.firstChild.nextSibling;
   var menuitems = tabbarMenu.childNodes;
   var separator = null;
   for(var i=0, c=0 ; i < menuitems.length-1 ; i++) {
      if (menuitems[i].localName=="menuseparator")
         if (++c==2) { separator=menuitems[i]; break; }
   }
   tabbarMenu.insertBefore(document.getElementById("ietab-tabbar-sep"), separator);
   tabbarMenu.insertBefore(document.getElementById("ietab-tabbar-switch"), separator);
   tabbarMenu.insertBefore(document.getElementById("ietab-tabbar-extapp"), separator);
   //disable toolbar menuitem tooltip
   gIeTab.hookAttr(gBrowser.mStrip.firstChild, "onpopupshowing", "if (document.tooltipNode.localName != 'tab') return false;");
}

IeTab.prototype.updateContextMenu = function() {
	var extAppName = getExtAppName(gIeTab.getStrPref("coral.ietab.extAppPath", ""));
	if ( extAppName != "" ) {
		var txt = GetLocalizedString("popup_viewpage_extapp");
		if ( txt && ( txt != "" ) ) {
			var menuitem = document.getElementById('ietab-viewpage-extapp');
			if (menuitem) menuitem.setAttribute("label", txt.replace("%s", extAppName));
			menuitem = document.getElementById('context_ietab-viewpage-extapp');
			if (menuitem) menuitem.setAttribute("label", txt.replace("%s", extAppName));
		}
		
		txt = GetLocalizedString("popup_viewlink_extapp");
		if ( txt && ( txt != "" ) ) {
			var menuitem = document.getElementById('ietab-viewpage-extapp');
			if (menuitem) menuitem.setAttribute("label", txt.replace("%s", extAppName));
			menuitem = document.getElementById('link_ietab-viewpage-extapp');
			if (menuitem) menuitem.setAttribute("label", txt.replace("%s", extAppName));
		}
		
		txt = GetLocalizedString("popup_bookmark_extapp");
		if ( txt && ( txt != "" ) ) {
			var menuitem = document.getElementById("ietab-bookmark-extapp");
			if ( menuitem ) menuitem.setAttribute("label", txt.replace("%s", extAppName));
		}
		
		txt = GetLocalizedString("popup_tabbar_extapp");
		if ( txt && ( txt != "" ) ) {
			var menuitem = document.getElementById("ietab-tabbar-extapp");
			if ( menuitem) menuitem.setAttribute("label", txt.replace("%s", extAppName));
		}
	}
}

IeTab.prototype.getTitleEnding = function(oldModifier) {
   var ietab = gIeTab.getIeTabElmt();
   if (ietab) {
      var titleEnding = gIeTab.getStrPref("coral.ietab.titleEnding", "");
      if (titleEnding != "") return titleEnding;
   }
   return oldModifier;
}

IeTab.prototype.focusIeTab = function() {
   gIeTab.goDoCommand("focus");
}

IeTab.prototype.onTabSelected = function(e) {
   if (e.originalTarget.localName == "tabs") {
      gIeTab.updateAll();
      window.setTimeout(gIeTab.focusIeTab, 0);
   }
}

IeTab.prototype.assignJSObject = function(aDoc) {
   if (aDoc instanceof HTMLDocument) {
      var aBrowser = getBrowser().getBrowserForDocument(aDoc);
      if (aBrowser && aBrowser.currentURI && aBrowser.currentURI.spec.indexOf(gIeTabChromeStr) == 0) {
         if (aDoc && aDoc.getElementById('IETab')) {
            var ietab = aDoc.getElementById('IETab');
            if (ietab.wrappedJSObject) ietab = ietab.wrappedJSObject;
            ietab.requestTarget = gIeTab;
            ietab.tabId = aBrowser.parentNode.id;		// Tell our plugin the tab id it belonging to
         }
      }
   }
}

IeTab.prototype.onPageShowOrLoad = function(e) {
   window.setTimeout(gIeTab.assignJSObject, 0, e.target);
   gIeTab.updateAll();
}

IeTab.prototype.getCurrentIeTabURI = function(aBrowser) {
   try {
      var docShell = aBrowser.boxObject.QueryInterface(Components.interfaces.nsIBrowserBoxObject).docShell;
      var wNav = docShell.QueryInterface(Components.interfaces.nsIWebNavigation);
      if (wNav.currentURI && wNav.currentURI.spec.substr(0, gIeTabChromeStr.length) == gIeTabChromeStr) {
         var ietab = wNav.document.getElementById("IETab");
         if (ietab) {
            if (ietab.wrappedJSObject) ietab = ietab.wrappedJSObject;
            var url = ietab.url;
            if (url) {
               const ios = Components.classes["@mozilla.org/network/io-service;1"].getService(Components.interfaces.nsIIOService);
               return ios.newURI(gIeTabChromeStr + ietab.flags + ',' + encodeURI(url), null, null);
            }
         }
      }
   } catch(e) {}
   return null;
}

IeTab.prototype.hookBrowserGetter = function(aBrowser) {
   if (aBrowser.localName != "browser") aBrowser = aBrowser.getElementsByTagNameNS(kXULNS, "browser")[0];
   // hook aBrowser.currentURI
   gIeTab.hookProp(aBrowser, "currentURI", function() {
      var uri = gIeTab.getCurrentIeTabURI(this);
      if (uri) return uri;
   });
   // hook aBrowser.sessionHistory
   gIeTab.hookProp(aBrowser, "sessionHistory", function() {
      var history = this.webNavigation.sessionHistory;
      var uri = gIeTab.getCurrentIeTabURI(this);
      if (uri) {
         var entry = history.getEntryAtIndex(history.index, false);
         if (entry.URI.spec != uri.spec) {
            entry.QueryInterface(Components.interfaces.nsISHEntry).setURI(uri);
            if (this.parentNode.__SS_data) delete this.parentNode.__SS_data;
         }
      }
   });
}

IeTab.prototype.hookURLBarSetter = function(aURLBar) {
	if (!aURLBar) aURLBar = document.getElementById("urlbar");
	if (!aURLBar) return;
	gIeTab.hookProp(aURLBar, "value", null, function() {
		this.isModeIE = arguments[0] && (arguments[0].substr(0, gIeTabChromeStr.length) == gIeTabChromeStr);
		if (this.isModeIE) {
			arguments[0] = gIeTab.getIeTabTrimURL(arguments[0]);
			// if (arguments[0] == "about:blank") arguments[0] = "";
		}
	});
}

IeTab.prototype.checkFilter = function(aBrowser, aRequest, aLocation) {
	var ietabwatch = Components.classes["@mozilla.org/ietabwatch;1"].getService().wrappedJSObject;
	if (ietabwatch) {
		var url = aLocation.spec;
		
		////////////////////////////////////////////////////////////////////////
		// Firefox file:// protocol strange behaviour fix
		// See nsIeTabWatchFactory.js for details
		//
		if (gIeTab.startsWith(url, "file:")) {
			var p = url.substr(6).indexOf("~CoralIETab.html?p=");
			if ( p > 0 ) {
				url = url.substr(p+25);
				
				if (!aBrowser.getAttribute(browserAttr)) {
					var r = ietabwatch.checkFilter(url);
					if (r >= 0) {
						url = gIeTab.getIeTabURL(url, r);
					}
				}
				
				if (aRequest) {
					aRequest.cancel(0x804b0002); //NS_BINDING_ABORTED
					// aBrowser.loadURI(url);
					setTimeout(function(url) { gBrowser.loadURI(url); }, 300, url);
				}
			}
		}
		
		if (aBrowser.getAttribute(browserAttr)) {
			aBrowser.removeAttribute(browserAttr);
		}
	}
}

IeTab.prototype.checkAdblock = function(contentType, url, node) {
	var contentPolicy = Components.classes["@mozilla.org/layout/content-policy;1"].getService(Components.interfaces.nsIContentPolicy);
	if (contentPolicy) {
		const ios = Components.classes["@mozilla.org/network/io-service;1"].getService(Components.interfaces.nsIIOService);
		var contentLocation = ios.newURI(url, null, null);
		return contentPolicy.shouldLoad(contentType, contentLocation, null, node, null, null) == Components.interfaces.nsIContentPolicy.ACCEPT;
	}
	else {
		return true;
	}
}

IeTab.prototype.hookCodeAll = function() {
   //hook properties
   gIeTab.hookBrowserGetter(gBrowser.mTabContainer.firstChild.linkedBrowser);
   gIeTab.hookURLBarSetter(gURLBar);

   //hook functions
   gIeTab.hookCode("gFindBar._onBrowserKeypress", "this._useTypeAheadFind &&", "$& !gIeTab.isIeEngine() &&");
   gIeTab.hookCode("PlacesCommandHook.bookmarkPage", "aBrowser.currentURI", "makeURI(gIeTab.getIeTabTrimURL($&.spec))");
   gIeTab.hookCode("PlacesStarButton.updateState", "getBrowser().currentURI", "makeURI(gIeTab.getIeTabTrimURL($&.spec))");
   gIeTab.hookCode("gBrowser.addTab", "return t;", "gIeTab.hookBrowserGetter(t.linkedBrowser); $&");
   gIeTab.hookCode("nsBrowserAccess.prototype.openURI", "var loadflags = isExternal ?", "var loadflags = false ?");
   gIeTab.hookCode("gBrowser.updateTitlebar", 'docElement.getAttribute("titlemodifier")', 'gIeTab.getTitleEnding($&)');
   gIeTab.hookCode("gBrowser.setTabTitle", "if (browser.currentURI.spec) {", "$& if (browser.currentURI.spec.indexOf(gIeTabChromeStr) == 0) return;");
   gIeTab.hookCode("URLBarSetURI", "getWebNavigation()", "getBrowser()");
   gIeTab.hookCode("getShortcutOrURI", /return (\S+);/g, "return gIeTab.getHandledURL($1);");
   if (gURLBar.handleCommand) gIeTab.hookCode("gURLBar.handleCommand", "this.value = url;", "url = gIeTab.getHandledURL(url); $&"); //fx3.1
   else gIeTab.hookCode("BrowserLoadURL", "url = gURLBar.value;", "url = gIeTab.getHandledURL(gURLBar.value);"); //fx3.0
   gIeTab.hookCode('gBrowser.mTabProgressListener', "function (aWebProgress, aRequest, aLocation) {", "$& gIeTab.checkFilter(this.mBrowser, aRequest, aLocation);");
   for(var i=0 ; i<gBrowser.mTabListeners.length ; i++)
      gIeTab.hookCode("gBrowser.mTabListeners["+i+"].onLocationChange", /{/, "$& gIeTab.checkFilter(this.mBrowser, aRequest, aLocation);");

   //hook Interface Commands
   gIeTab.hookCode("BrowserBack", /{/, "$& if(gIeTab.goDoCommand('goBack')) return;");
   gIeTab.hookCode("BrowserForward", /{/, "$& if(gIeTab.goDoCommand('goForward')) return;");
   gIeTab.hookCode("BrowserStop", /{/, "$& if(gIeTab.goDoCommand('stop')) return;");
   gIeTab.hookCode("BrowserReload", /{/, "$& if(gIeTab.goDoCommand('refresh')) return;");
   gIeTab.hookCode("BrowserReloadSkipCache", /{/, "$& if(gIeTab.goDoCommand('refresh')) return;");

   gIeTab.hookCode("saveDocument", /{/, "$& if(gIeTab.goDoCommand('saveAs')) return;");
   gIeTab.hookCode("BrowserViewSourceOfDocument", /{/, "$& if(gIeTab.goDoCommand('viewSource')) return;");
   gIeTab.hookCode("MailIntegration.sendMessage", /{/, "$& var ietab = gIeTab.getIeTabElmt(); if(ietab){ arguments[0]=ietab.url; arguments[1]=ietab.title; }");

   gIeTab.hookCode("PrintUtils.print", /{/, "$& if(gIeTab.goDoCommand('print')) return;");
   gIeTab.hookCode("PrintUtils.showPageSetup", /{/, "$& if(gIeTab.goDoCommand('printSetup')) return;");
   gIeTab.hookCode("PrintUtils.printPreview", /{/, "$& if(gIeTab.goDoCommand('printPreview')) return;");

   gIeTab.hookCode("goDoCommand", /{/, "$& if(gIeTab.goDoCommand(arguments[0])) return;");

   gIeTab.hookAttr("cmd_find", "oncommand", "if(gIeTab.goDoCommand('find')) return;");
   gIeTab.hookAttr("cmd_findAgain", "oncommand", "if(gIeTab.goDoCommand('find')) return;");
   gIeTab.hookAttr("cmd_findPrevious", "oncommand", "if(gIeTab.goDoCommand('find')) return;");

   gIeTab.hookCode("displaySecurityInfo", /{/, "$& if(gIeTab.goDoCommand('displaySecurityInfo')) return;");
}

IeTab.prototype.addEventAll = function() {
   gIeTab.addEventListener(window, "DOMContentLoaded", gIeTab.onPageShowOrLoad);
   gIeTab.addEventListener(window, "pageshow", gIeTab.onPageShowOrLoad);

   gIeTab.addEventListener(gBrowser.mStrip.firstChild.nextSibling, "popupshowing", gIeTab.updateTabbarMenu);
   gIeTab.addEventListener("appcontent", "select", gIeTab.onTabSelected);

   gIeTab.addEventListener("goPopup", "popupshowing", gIeTab.updateGoMenuItems);
   gIeTab.addEventListener("placesContext", "popupshowing", gIeTab.addBookmarkMenuitem);
   gIeTab.addEventListener("menu_EditPopup", "popupshowing", gIeTab.updateEditMenuItems);
   gIeTab.addEventListener("menu_ToolsPopup", "popupshowing", gIeTab.updateToolsMenuItem);
   gIeTab.addEventListener("contentAreaContextMenu", "popupshowing", gIeTab.ietabContextMenuPopup);
}

IeTab.prototype.removeEventAll = function() {
   gIeTab.removeEventListener(window, "DOMContentLoaded", gIeTab.onPageShowOrLoad);
   gIeTab.removeEventListener(window, "pageshow", gIeTab.onPageShowOrLoad);

   gIeTab.removeEventListener(gBrowser.mStrip.firstChild.nextSibling, "popupshowing", gIeTab.updateTabbarMenu);
   gIeTab.removeEventListener("appcontent", "select", gIeTab.onTabSelected);

   gIeTab.removeEventListener("goPopup", "popupshowing", gIeTab.updateGoMenuItems);
   gIeTab.removeEventListener("placesContext", "popupshowing", gIeTab.addBookmarkMenuitem);
   gIeTab.removeEventListener("menu_EditPopup", "popupshowing", gIeTab.updateEditMenuItems);
   gIeTab.removeEventListener("menu_ToolsPopup", "popupshowing", gIeTab.updateToolsMenuItem);
   gIeTab.removeEventListener("contentAreaContextMenu", "popupshowing", gIeTab.ietabContextMenuPopup);

   gIeTab.removeEventListener(window, "load", gIeTab.init);
   gIeTab.removeEventListener(window, "unload", gIeTab.destroy);
}

IeTab.prototype.init = function() {
   gIeTab.hookCodeAll();
   gIeTab.addEventAll();
   gIeTab.createTabbarMenu();
   gIeTab.updateContextMenu();
}

IeTab.prototype.destroy = function() {
   gIeTab.removeEventAll();
   delete gIeTab;
}

var gIeTab = new IeTab();

gIeTab.addEventListener(window, "load", gIeTab.init);
gIeTab.addEventListener(window, "unload", gIeTab.destroy);
