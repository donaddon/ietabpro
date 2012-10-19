/*
 * Copyright (c) 2005 yuoo2k <yuoo2k@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

const _IETABWATCH_CID = Components.ID('{3fdaa104-5988-4050-94fc-c711d568fe64}');
const _IETABWATCH_CONTRACTID = "@mozilla.org/ietabwatch;1";
const gIeTabChromeStr = "chrome://coralietab/content/container.html?url=";
var nsJSON = Components.classes["@mozilla.org/dom/json;1"].createInstance(Components.interfaces.nsIJSON);

function _dump(aMessage) {
  var consoleService = Components.classes["@mozilla.org/consoleservice;1"]
                                 .getService(Components.interfaces.nsIConsoleService);
  consoleService.logStringMessage("[Coral IETab]" + aMessage);
}

// IeTabWatcher object
var IeTabWatcher = {
   isIeTabURL: function(url) {
      if (!url) return false;
      return (url.substr(0, gIeTabChromeStr.length) == gIeTabChromeStr);
   },
   
	isFileProtocol: function(url) {
		if (!url) return false;
		return url.substr(0,5) == "file:";
	},

   getIeTabURL: function(url, flags) {
      if (this.isIeTabURL(url)) return url;
      if (/^file:\/\/.*/.test(url)) try { url = decodeURI(url).substring(8).replace(/\//g, "\\"); }catch(e){}
      return gIeTabChromeStr + flags + "," + encodeURI(url);
   },

   getBoolPref: function(prefName, defval) {
      var result = defval;
      var prefservice = Components.classes["@mozilla.org/preferences-service;1"].getService(Components.interfaces.nsIPrefService);
      var prefs = prefservice.getBranch("");
      if (prefs.getPrefType(prefName) == prefs.PREF_BOOL) {
          try { result = prefs.getBoolPref(prefName); }catch(e){}
      }
      return(result);
   },

   getStrPref: function(prefName, defval) {
      var result = defval;
      var prefservice = Components.classes["@mozilla.org/preferences-service;1"].getService(Components.interfaces.nsIPrefService);
      var prefs = prefservice.getBranch("");
      if (prefs.getPrefType(prefName) == prefs.PREF_STRING) {
          try { result = prefs.getComplexValue(prefName, Components.interfaces.nsISupportsString).data; }catch(e){}
      }
      return(result);
   },

   setStrPref: function(prefName, value) {
      var prefservice = Components.classes["@mozilla.org/preferences-service;1"].getService(Components.interfaces.nsIPrefService);
      var prefs = prefservice.getBranch("");
      var sString = Components.classes["@mozilla.org/supports-string;1"].createInstance(Components.interfaces.nsISupportsString);
      sString.data = value;
      try { prefs.setComplexValue(prefName, Components.interfaces.nsISupportsString, sString); } catch(e){}
   },

   isFilterEnabled: function() {
      return (this.getBoolPref("coral.ietab.filter", true));
   },

   getPrefRuleList: function() {
	var s = this.getStrPref("coral.ietab.rulelist", null);
	if(s) {
		return nsJSON.decode(s);
	}
	else {
		return null;
	}
   },

   setPrefFilterList: function(list) {
      this.setStrPref("coral.ietab.filterlist", list.join(" "));
   },

   isMatchURL: function(url, pattern) {
      if ((!pattern) || (pattern.length==0)) return false;
      var retest = /^\/(.*)\/$/.exec(pattern);
      if (retest) {
         pattern = retest[1];
      } else {
         pattern = pattern.replace(/\\/g, "/");
         var m = pattern.match(/^(.+:\/\/+[^\/]+\/)?(.*)/);
         m[1] = (m[1] ? m[1].replace(/\./g, "\\.").replace(/\?/g, "[^\\/]?").replace(/\*/g, "[^\\/]*") : "");
         m[2] = (m[2] ? m[2].replace(/\./g, "\\.").replace(/\+/g, "\\+").replace(/\?/g, "\\?").replace(/\*/g, ".*") : "");
         pattern = m[1] + m[2];
         pattern = "^" + pattern.replace(/\/$/, "\/.*") + "$";
      }
      var reg = new RegExp(pattern.toLowerCase());
      return (reg.test(url.toLowerCase()));
   },

	// Return value < 0 means bypass, other value is action flags
	checkUrlAction: function(url) {
		var aList = this.getPrefRuleList();
		for (var i=0; i<aList.length; i++) {
			var pos = aList[i].indexOf(',');
			if (pos > 0) {
				var flags = parseInt(aList[i].substr(0,pos));
				var rule = aList[i].substr(pos+1);
				
				var enabled = ((flags & 0x8000) == 0);			// The highest bit == 1 means disabled
				if (enabled && this.isMatchURL(url, rule)) return flags;
			}
		}
		
		return -1;
   },

   getTopWinBrowser: function() {
      try {
         var winMgr = Components.classes['@mozilla.org/appshell/window-mediator;1'].getService();
         var topWin = winMgr.QueryInterface(Components.interfaces.nsIWindowMediator).getMostRecentWindow("navigator:browser");
         var mBrowser = topWin.document.getElementById("content");
         return mBrowser;
      } catch(e) {}
      return null;
   },
}

function file_protocol_patch(url) {
	try {
		var file = Components.classes["@mozilla.org/file/directory_service;1"].
					getService(Components.interfaces.nsIProperties).
					get("TmpD", Components.interfaces.nsIFile);
		file.append("~CoralIETab.html");
		if (!file.exists()) {
			file.create(Components.interfaces.nsIFile.NORMAL_FILE_TYPE, 0666);
		}
		
		return "file://"+file.path+"?p="+url;
		
	} catch(e) {}
		
	return null;
}

// ContentPolicy class
var IeTabWatchFactoryClass = {
  checkFilter: function(url) {
    if (IeTabWatcher.isIeTabURL(url)) return -1;
    if (!IeTabWatcher.isFilterEnabled()) return -1;
    return IeTabWatcher.checkUrlAction(url);
  },
  
	// nsIContentPolicy interface implementation
	shouldLoad: function(contentType, contentLocation, requestOrigin, requestingNode, mimeTypeGuess, extra) {
    	if (contentType == Components.interfaces.nsIContentPolicy.TYPE_DOCUMENT) {
			var node = requestingNode ? requestingNode.QueryInterface(Components.interfaces.nsIDOMNode) : null;
			if (node && (!node.getAttribute("ietabManualSwitch"))) {
				var url = contentLocation.spec;
				var r = this.checkFilter(url);
				if (r>=0) {
					if (IeTabWatcher.isFileProtocol(url)) {
						// nsIContentPolicy has strange behaviour for file: protocol. Changing contentLocation.spec
						// from file: to chrome: will cause error. So we have to stick to file: protocol, and let
						// IeTab::checkFilter (in ietabOverlay.js) deal with it
						contentLocation.spec = file_protocol_patch(url);
					}
					else {
						contentLocation.spec = IeTabWatcher.getIeTabURL(url, r);
					}
				}
			}
		}
		
		return (Components.interfaces.nsIContentPolicy.ACCEPT);
	},
	
  // this is now for urls that directly load media, and meta-refreshes (before activation)
  shouldProcess: function(contentType, contentLocation, requestOrigin, requestingNode, mimeType, extra) {
    return (Components.interfaces.nsIContentPolicy.ACCEPT);
  },

  get wrappedJSObject() {
    return this;
  }
}

// Factory object
var IeTabWatchFactoryFactory = {
  createInstance: function(outer, iid) {
    if (outer != null) throw Components.results.NS_ERROR_NO_AGGREGATION;
    return IeTabWatchFactoryClass;
  }
}

// Module object
var IeTabWatchFactoryModule = {
  registerSelf: function(compMgr, fileSpec, location, type) {
    compMgr = compMgr.QueryInterface(Components.interfaces.nsIComponentRegistrar);
    compMgr.registerFactoryLocation(_IETABWATCH_CID, "IETab content policy", _IETABWATCH_CONTRACTID, fileSpec, location, type);
    var catman = Components.classes["@mozilla.org/categorymanager;1"].getService(Components.interfaces.nsICategoryManager);
    catman.addCategoryEntry("content-policy", _IETABWATCH_CONTRACTID, _IETABWATCH_CONTRACTID, true, true);
  },

  unregisterSelf: function(compMgr, fileSpec, location) {
    compMgr = compMgr.QueryInterface(Components.interfaces.nsIComponentRegistrar);
    compMgr.unregisterFactoryLocation(_IETABWATCH_CID, fileSpec);
    var catman = Components.classes["@mozilla.org/categorymanager;1"].getService(Components.interfaces.nsICategoryManager);
    catman.deleteCategoryEntry("content-policy", _IETABWATCH_CONTRACTID, true);
  },

  getClassObject: function(compMgr, cid, iid) {
    if (!cid.equals(_IETABWATCH_CID))
      throw Components.results.NS_ERROR_NO_INTERFACE;

    if (!iid.equals(Components.interfaces.nsIFactory))
      throw Components.results.NS_ERROR_NOT_IMPLEMENTED;

    return IeTabWatchFactoryFactory;
  },

  canUnload: function(compMgr) {
    return true;
  }
};

// module initialisation
function NSGetModule(comMgr, fileSpec) {
  return IeTabWatchFactoryModule;
}
