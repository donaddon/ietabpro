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
 
function IeTab() {}

var IETAB_SYNC_COOKIES = 1;
var IETAB_SYNC_USER_AGENT = 2;
var IETAB_AUTO_SWITCH_BACK = 4;

IeTab.prototype = {
   IExploreExePath: ""
}

IeTab.prototype.getPrefRuleList = function() {
	var s = this.getStrPref("coral.ietab.rulelist", null);
	if(s) {
		return nsJSON.decode(s);
	}
	else {
		return null;
	}
}

IeTab.prototype.getPrefFilterListFromOldIETab = function() {
   var s = this.getStrPref("ietab.filterlist", null);
   return (s ? s.split(" ") : "");
}

IeTab.prototype.addFilterRule = function(rule) {
	var pos = rule.indexOf(',');
	if ( -1 == pos ) return 0;
	var flags = parseInt(rule.substr(0,pos));
	var ruleStr = rule.substr(pos+1);
	var rulesListChilds = document.getElementById('rulesListChilds');
	
	// Checking whether the rule already exists
	var bFound = false;
	for ( var i = 0; i < rulesListChilds.childNodes.length; i++ ) {
		var child = rulesListChilds.childNodes[i];
		if ( child.firstChild.firstChild.getAttribute('label') == ruleStr )
		{
			bFound = true;
			break;
		}
	}
	
	if (!bFound) {
		var item = document.createElement('treeitem');
		var row = document.createElement('treerow');
		var c1 = document.createElement('treecell');
		c1.setAttribute('label', ruleStr);
		var c2 = document.createElement('treecell');
		c2.setAttribute('value', (flags & 0x8000) == 0);	// The highest bit == 1 means disabled
		var c3 = document.createElement('treecell');
		c3.setAttribute('value', (flags & IETAB_SYNC_COOKIES) != 0);
		var c4 = document.createElement('treecell');
		c4.setAttribute('value', (flags & IETAB_SYNC_USER_AGENT) != 0);
		var c5 = document.createElement('treecell');
		c5.setAttribute('value', (flags & IETAB_AUTO_SWITCH_BACK) != 0);
		row.appendChild(c1);
		row.appendChild(c2);
		row.appendChild(c3);
		row.appendChild(c4);
		row.appendChild(c5);
		item.appendChild(row);
		rulesListChilds.appendChild(item);
	}
	
	return (rulesListChilds.childNodes.length-1);
}

IeTab.prototype.initDialog = function() {
   // get iexplore.exe path
   this.IExploreExePath = IeTabExtApp.getIExploreExePath();

	////////////////////////////////////////////////////////////////
	// rules
	// Firstly we'll clear the existing contents 
	var rules = document.getElementById('rulesListChilds');
	while (rules.hasChildNodes()) rules.removeChild(rules.firstChild);
	// Then we'll fill the contents with rules from preference
	var ruleList = this.getPrefRuleList();
	if ( ruleList ) {
		for (var i = 0; i < ruleList.length; i++) {
			if (ruleList[i]) {
				this.addFilterRule(ruleList[i]);
			}
		}
	}
	////////////////////////////////////////////////////////////////
   
   //general
   document.getElementById('toolsmenu').checked = this.getBoolPref("coral.ietab.toolsmenu", true);
   document.getElementById('toolsmenu.icon').checked = this.getBoolPref("coral.ietab.toolsmenu.icon", false);
   document.getElementById('statusbar').checked = this.getBoolPref("coral.ietab.statusbar", true);
   document.getElementById('handleurl').checked = this.getBoolPref("coral.ietab.handleUrlBar", false);
   document.getElementById('alwaysnew').checked = this.getBoolPref("coral.ietab.alwaysNewTab", false);
   document.getElementById('focustab').checked  = this.getBoolPref("coral.ietab.focustab", true);
   document.getElementById('showprompt').checked  = this.getBoolPref("coral.ietab.showprompt", true);
   document.getElementById('adblock').checked = this.getBoolPref("coral.ietab.adblock", false);

   //context
   document.getElementById('pagelink.embed').checked = this.getBoolPref("coral.ietab.pagelink", true);
   document.getElementById('tabsmenu.embed').checked = this.getBoolPref("coral.ietab.tabsmenu", true);
   document.getElementById('bookmark.embed').checked = this.getBoolPref("coral.ietab.bookmark", true);

   document.getElementById('pagelink.extapp').checked = this.getBoolPref("coral.ietab.pagelink.extapp", true);
   document.getElementById('tabsmenu.extapp').checked = this.getBoolPref("coral.ietab.tabsmenu.extapp", true);
   document.getElementById('bookmark.extapp').checked = this.getBoolPref("coral.ietab.bookmark.extapp", true);

   document.getElementById('pagelink.icon').checked = this.getBoolPref("coral.ietab.icon.pagelink", false);
   document.getElementById('tabsmenu.icon').checked = this.getBoolPref("coral.ietab.icon.tabsmenu", false);
   document.getElementById('bookmark.icon').checked = this.getBoolPref("coral.ietab.icon.bookmark", false);

   //external
   var path = this.getStrPref("coral.ietab.extAppPath", "");
   document.getElementById('pathbox').value = (path == "" ? this.IExploreExePath : path);
   document.getElementById('parambox').value = this.getStrPref("coral.ietab.extAppParam", "%1");
   document.getElementById('ctrlclick').checked = this.getBoolPref("coral.ietab.ctrlclick", true);

   //fill urlbox
   var newurl = (window.arguments ? window.arguments[0] : ""); //get CurrentTab's URL
   document.getElementById('urlbox').value = ( this.startsWith(newurl,"about:") ? "" : newurl);
   document.getElementById('urlbox').select();

   //updateStatus
   this.updateDialogPositions();
   this.updateDialogAllStatus();
   this.updateApplyButton(false);
}

IeTab.prototype.updateApplyButton = function(e) {
   document.getElementById("myExtra1").disabled = !e;
}

IeTab.prototype.init = function() {
   this.initDialog();
   this.addEventListenerByTagName("checkbox", "command", this.updateApplyButton);
   this.addEventListener("rulesListChilds", "DOMAttrModified", this.updateApplyButton);
   this.addEventListener("rulesListChilds", "DOMNodeInserted", this.updateApplyButton);
   this.addEventListener("rulesListChilds", "DOMNodeRemoved", this.updateApplyButton);
   this.addEventListener("parambox", "input", this.updateApplyButton);
   this.addEventListener("toolsmenu", "command", this.updateToolsMenuStatus);
}

IeTab.prototype.destory = function() {
   this.removeEventListenerByTagName("checkbox", "command", this.updateApplyButton);
   this.removeEventListener("rulesListChilds", "DOMAttrModified", this.updateApplyButton);
   this.removeEventListener("rulesListChilds", "DOMNodeInserted", this.updateApplyButton);
   this.removeEventListener("rulesListChilds", "DOMNodeRemoved", this.updateApplyButton);
   this.removeEventListener("parambox", "input", this.updateApplyButton);
   this.removeEventListener("toolsmenu", "command", this.updateToolsMenuStatus);
}

IeTab.prototype.updateInterface = function() {
   var statusbar = this.getBoolPref("coral.ietab.statusbar", true);
   var icon = (window.arguments ? window.arguments[1] : null); //get status-bar icon handle
   if (icon) this.setAttributeHidden(icon, statusbar);
}

IeTab.prototype.setOptions = function() {
   //filter
   this.setBoolPref("coral.ietab.filter", true);
   this.setStrPref("coral.ietab.rulelist", this.getRuleListString());

   //general
   var toolsmenu = document.getElementById('toolsmenu').checked;
   this.setBoolPref("coral.ietab.toolsmenu", toolsmenu);
   this.setBoolPref("coral.ietab.toolsmenu.icon", document.getElementById('toolsmenu.icon').checked);

   var statusbar = document.getElementById('statusbar').checked;
   this.setBoolPref("coral.ietab.statusbar", statusbar);

   this.setBoolPref("coral.ietab.handleUrlBar", document.getElementById('handleurl').checked);
   this.setBoolPref("coral.ietab.alwaysNewTab", document.getElementById('alwaysnew').checked);
   this.setBoolPref("coral.ietab.focustab", document.getElementById('focustab').checked);
   this.setBoolPref("coral.ietab.showprompt", document.getElementById('showprompt').checked);
   this.setBoolPref("coral.ietab.adblock", document.getElementById('adblock').checked);

   //context (ietab)
   this.setBoolPref("coral.ietab.pagelink", document.getElementById('pagelink.embed').checked);
   this.setBoolPref("coral.ietab.tabsmenu", document.getElementById('tabsmenu.embed').checked);
   this.setBoolPref("coral.ietab.bookmark", document.getElementById('bookmark.embed').checked);

   //context (extapp)
   this.setBoolPref("coral.ietab.pagelink.extapp", document.getElementById('pagelink.extapp').checked);
   this.setBoolPref("coral.ietab.tabsmenu.extapp", document.getElementById('tabsmenu.extapp').checked);
   this.setBoolPref("coral.ietab.bookmark.extapp", document.getElementById('bookmark.extapp').checked);

   //showicon
   this.setBoolPref("coral.ietab.icon.pagelink", document.getElementById('pagelink.icon').checked);
   this.setBoolPref("coral.ietab.icon.tabsmenu", document.getElementById('tabsmenu.icon').checked);
   this.setBoolPref("coral.ietab.icon.bookmark", document.getElementById('bookmark.icon').checked);

   //external
   var path = document.getElementById('pathbox').value;
   this.setStrPref("coral.ietab.extAppPath", (path == this.IExploreExePath ? "" : path));

   var param = document.getElementById('parambox').value;
   this.setStrPref("coral.ietab.extAppParam", this.trim(param).split(/\s+/).join(" "));

   this.setBoolPref("coral.ietab.ctrlclick", document.getElementById('ctrlclick').checked);

   //update UI
   this.updateApplyButton(false);
   this.updateInterface();
}

IeTab.prototype.setAttributeHidden = function(obj, isHidden) {
   if (!obj) return;
   if (isHidden){
      obj.removeAttribute("hidden");
   }else{
      obj.setAttribute("hidden", true);
   }
}

IeTab.prototype.getRuleListString = function() {
	var list = [];
	var rules = document.getElementById('rulesList');
	var count = rules.view.rowCount;
	for (var i=0; i<count; i++) {
		var rule = rules.view.getCellText(i, rules.columns['col-rules']);
		if(rule!="")
		{
			var flags = 0;
			
			if(rules.view.getCellValue(i, rules.columns['col-enabled'])=="false") flags |= 0x8000;			// The highest bit == 1 means disabled
			if(rules.view.getCellValue(i, rules.columns['col-sync-cookies'])=="true") flags |= IETAB_SYNC_COOKIES;
			if(rules.view.getCellValue(i, rules.columns['col-sync-useragent'])=="true") flags |= IETAB_SYNC_USER_AGENT;
			if(rules.view.getCellValue(i, rules.columns['col-switchback'])=="true") flags |= IETAB_AUTO_SWITCH_BACK;
			
			list.push(flags+","+rule);
		}
	}
	return nsJSON.encode(list);
}

IeTab.prototype.updateToolsMenuStatus = function() {
   document.getElementById("toolsmenu.icon").disabled = !document.getElementById("toolsmenu").checked;
}

IeTab.prototype.updateAddButtonStatus = function() {
   var addbtn = document.getElementById('addbtn');
   var urlbox = document.getElementById('urlbox');
   addbtn.disabled = (this.trim(urlbox.value).length < 1);
}

IeTab.prototype.updateDialogAllStatus = function() {
   this.updateAddButtonStatus();
   this.updateToolsMenuStatus();
}

IeTab.prototype.updateDialogPositions = function() {
   var em = [document.getElementById('tabsmenu.embed'),
             document.getElementById('pagelink.embed'),
             document.getElementById('bookmark.embed')]
   var ex = [document.getElementById('tabsmenu.extapp'),
             document.getElementById('pagelink.extapp'),
             document.getElementById('bookmark.extapp')]
   var emMax = Math.max(em[0].boxObject.width, em[1].boxObject.width, em[2].boxObject.width);
   var exMax = Math.max(ex[0].boxObject.width, ex[1].boxObject.width, ex[2].boxObject.width);
   for (var i=0 ; i<em.length ; i++) em[i].width = emMax;
   for (var i=0 ; i<ex.length ; i++) ex[i].width = exMax;
}

IeTab.prototype.findRule = function(value) {
   var ruleList = document.getElementById('rulesList');
   var count = ruleList.view.rowCount;
   for (var i=0; i<count; i++) {
      var rule = ruleList.view.getCellText(i, ruleList.columns['col-rules']);
      if (rule == value) return i;
   }
   return -1;
}

IeTab.prototype.addNewURL = function() {
   var ruleList = document.getElementById('rulesList');
   var urlbox = document.getElementById('urlbox');
   var rule = this.trim(urlbox.value);
   if (rule != "") {
      if ((rule != "about:blank") && (rule.indexOf("://") < 0)) {
         rule = (/^[A-Za-z]:/.test(rule) ? "file:///"+rule.replace(/\\/g,"/") : rule);
         if (/^file:\/\/.*/.test(rule)) rule = encodeURI(rule);
      }
      if (!/^\/(.*)\/$/.exec(rule)) rule = rule.replace(/\/$/, "/*");
      rule = rule.replace(/\s/g, "%20");
      var idx = this.findRule(rule);
      if (idx == -1) {
      	idx = this.addFilterRule("0,"+rule);
      	
      	urlbox.value = "";
      }
      ruleList.view.selection.select(idx);
      ruleList.boxObject.ensureRowIsVisible(idx);
   }
   ruleList.focus();
   this.updateAddButtonStatus();
}

IeTab.prototype.delSelected = function() {
	var ruleList = document.getElementById('rulesList');
	if (ruleList) {
		var rules = document.getElementById('rulesListChilds');
		if (ruleList.view.selection.count > 0) {
			for (var i=rules.childNodes.length-1 ; i>=0 ; i--) {
				if (ruleList.view.selection.isSelected(i))
					rules.removeChild(rules.childNodes[i]);
			}
		}
	}
}

IeTab.prototype.onClickFilterList = function(e) {
   var ruleList = document.getElementById('rulesList');
   if (!ruleList.disabled && e.button == 0 && e.detail >= 2) {
      if (ruleList.view.selection.count == 1) {
         var urlbox = document.getElementById('urlbox');
         urlbox.value = ruleList.view.getCellText(ruleList.currentIndex, ruleList.columns['col-rules']);
         urlbox.select();
         this.updateAddButtonStatus();
      }
   }
}

IeTab.prototype.modifyTextBoxValue = function(textboxId, newValue) {
   var box = document.getElementById(textboxId);
   if (box.value != newValue) {
      box.value = newValue;
      this.updateApplyButton(true);
   }
}

IeTab.prototype.browseAppPath = function() {
   const nsIFilePicker = Components.interfaces.nsIFilePicker;
   var fp = Components.classes["@mozilla.org/filepicker;1"].createInstance(nsIFilePicker);
   fp.init(window, null, nsIFilePicker.modeOpen);
   fp.appendFilters(nsIFilePicker.filterApps);
   fp.appendFilters(nsIFilePicker.filterAll);
   var rv = fp.show();
   if (rv == nsIFilePicker.returnOK) {
      this.modifyTextBoxValue("pathbox", fp.file.target);
   }
}

IeTab.prototype.resetAppPath = function() {
   this.modifyTextBoxValue("pathbox", this.IExploreExePath);
   this.modifyTextBoxValue("parambox", "%1");
}

IeTab.prototype.saveToFile = function(aList) {
   var fp = Components.classes["@mozilla.org/filepicker;1"].createInstance(Components.interfaces.nsIFilePicker);
   var stream = Components.classes["@mozilla.org/network/file-output-stream;1"].createInstance(Components.interfaces.nsIFileOutputStream);
   var converter = Components.classes["@mozilla.org/intl/converter-output-stream;1"].createInstance(Components.interfaces.nsIConverterOutputStream);

   fp.init(window, null, fp.modeSave);
   fp.defaultExtension = "txt";
   fp.defaultString = "Coral IETab Rules";
   fp.appendFilters(fp.filterText);

   if (fp.show() != fp.returnCancel) {
      try {
         if (fp.file.exists()) fp.file.remove(true);
         fp.file.create(fp.file.NORMAL_FILE_TYPE, 0666);
         stream.init(fp.file, 0x02, 0x200, null);
         converter.init(stream, "UTF-8", 0, 0x0000);

         for (var i = 0; i < aList.length ; i++) {
            aList[i] = aList[i] + "\n";
            converter.writeString(aList[i]);
         }
      } finally {
         converter.close();
         stream.close();
      }
   }
}

IeTab.prototype.loadFromFile = function() {
   var fp = Components.classes["@mozilla.org/filepicker;1"].createInstance(Components.interfaces.nsIFilePicker);
   var stream = Components.classes["@mozilla.org/network/file-input-stream;1"].createInstance(Components.interfaces.nsIFileInputStream);
   var converter = Components.classes["@mozilla.org/intl/converter-input-stream;1"].createInstance(Components.interfaces.nsIConverterInputStream);

   fp.init(window, null, fp.modeOpen);
   fp.defaultExtension = "txt";
   fp.appendFilters(fp.filterText);

   if (fp.show() != fp.returnCancel) {
      try {
         var input = {};
         stream.init(fp.file, 0x01, 0444, null);
         converter.init(stream, "UTF-8", 0, 0x0000);
         converter.readString(stream.available(), input);
         var linebreak = input.value.match(/(((\n+)|(\r+))+)/m)[1];
         return input.value.split(linebreak);
      } finally {
         converter.close();
         stream.close();
      }
   }
   return null;
}

IeTab.prototype.getAllSettings = function(isDefault) {
   var prefservice = Components.classes["@mozilla.org/preferences-service;1"].getService(Components.interfaces.nsIPrefService);
   var prefs = (isDefault ? prefservice.getDefaultBranch("") : prefservice.getBranch("") );
   var preflist = prefs.getChildList("coral.ietab.", {});

   var aList = ["#Coral IETab Preferences"];
   for (var i = 0 ; i < preflist.length ; i++) {
      try {
         var value = null;
         switch (prefs.getPrefType(preflist[i])) {
         case prefs.PREF_BOOL:
            value = prefs.getBoolPref(preflist[i]);
            break;
         case prefs.PREF_INT:
            value = prefs.getIntPref(preflist[i]);
            break;
         case prefs.PREF_STRING:
            value = prefs.getComplexValue(preflist[i], Components.interfaces.nsISupportsString).data;
            break;
         }
         aList.push(preflist[i] + "=" + value);
      } catch (e) {}
   }
   return aList;
}

IeTab.prototype.setAllSettings = function(aList) {
   if (!aList) return;
   if (aList.length == 0) return;
   if (aList[0] != "#Coral IETab Preferences") return;

   var prefservice = Components.classes["@mozilla.org/preferences-service;1"].getService(Components.interfaces.nsIPrefService);
   var prefs = prefservice.getBranch("");

   var aPrefs = [];
   for (var i = 1 ; i < aList.length ; i++){
      var index = aList[i].indexOf("=");
      if (index > 0){
         var name = aList[i].substring(0, index);
         var value = aList[i].substring(index+1, aList[i].length);
         if (this.startsWith(name, "coral.ietab.")) aPrefs.push([name, value]);
      }
   }
   for (var i = 0 ; i < aPrefs.length ; i++) {
      try {
         var name = aPrefs[i][0];
         var value = aPrefs[i][1];
         switch (prefs.getPrefType(name)) {
         case prefs.PREF_BOOL:
            prefs.setBoolPref(name, /true/i.test(value));
            break;
         case prefs.PREF_INT:
            prefs.setIntPref(name, value);
            break;
         case prefs.PREF_STRING:
            if (value.indexOf('"') == 0) value = value.substring(1, value.length-1);
            var sString = Components.classes["@mozilla.org/supports-string;1"].createInstance(Components.interfaces.nsISupportsString);
            sString.data = value;
            prefs.setComplexValue(name, Components.interfaces.nsISupportsString, sString);
            break;
         }
      } catch (e) {}
   }
}

IeTab.prototype.exportSettings = function() {
   var aList = this.getAllSettings();
   if (aList) this.saveToFile(aList);
}

IeTab.prototype.importSettings = function() {
   var aList = this.loadFromFile();
   if (aList) {
      this.setAllSettings(aList);
      this.initDialog();
      this.updateInterface();
   }
}

IeTab.prototype.restoreDefault = function() {
	if (confirm(GetLocalizedString("confirm_reset"))) {
		var aTemp = this.getAllSettings(false);
		var aDefault = this.getAllSettings(true);
		this.setAllSettings(aDefault);
		this.initDialog();
		this.setAllSettings(aTemp);
		this.updateApplyButton(true);
	}
}

IeTab.prototype.importSettingsFromIetab = function() {
   var ruleList = this.getPrefFilterListFromOldIETab();
   if ( ruleList ) {
		for (var i = 0; i < ruleList.length; i++) {
			if (ruleList[i]) {
				this.addFilterRule('0,'+ruleList[i]);
			}
		}
	}
	else {
			alert(GetLocalizedString("no_ietab_settings"));
	}
}

function cmd_selectAll() {
	var tree = document.getElementById('rulesList');
	if (tree) {
		tree.view.selection.selectAll();
	}
}

function cmd_copyToClipboard() {
	var ruleList = document.getElementById('rulesList');
	if (ruleList) {
		var rules = document.getElementById('rulesListChilds');
		var list = [];
		if (ruleList.view.selection.count > 0) {
			for (var i=rules.childNodes.length-1 ; i>=0 ; i--) {
				if (ruleList.view.selection.isSelected(i))
					list.push(rules.childNodes[i].firstChild.firstChild.getAttribute('label'));
			}
		}

		var clipboardHelper = Components.classes["@mozilla.org/widget/clipboardhelper;1"].getService(Components.interfaces.nsIClipboardHelper);
		clipboardHelper.copyString(list.toString());
	}
}

function cmd_pasteFromClipboard() {
	var clipboard = Components.classes["@mozilla.org/widget/clipboard;1"].getService(Components.interfaces.nsIClipboard);
	var transferable = Components.classes["@mozilla.org/widget/transferable;1"].createInstance(Components.interfaces.nsITransferable);
	transferable.addDataFlavor("text/unicode");

	try {
		clipboard.getData(transferable, clipboard.kGlobalClipboard);
	}
	catch (e) {
		return;
	}

	var data = {};
	transferable.getTransferData("text/unicode", data, {});

	try {
		data = data.value.QueryInterface(Components.interfaces.nsISupportsString).data;
	}
	catch (e) {
		return;
	}

	for each (var ruleStr in data.split(','))
	{
		gIeTab.addFilterRule('0,'+ruleStr);
	}
}

function cmd_startEditing() {
	var ruleList = document.getElementById('rulesList');
	if (ruleList) {
		var rules = document.getElementById('rulesListChilds');
		if (ruleList.view.selection.count > 0) {
			for (var i=rules.childNodes.length-1 ; i>=0 ; i--) {
				if (ruleList.view.selection.isSelected(i)) {
					ruleList.startEditing(i, ruleList.columns[0]);
					break;
				}
			}
		}
	}
}

function cmd_moveRulesUp() {
	var ruleList = document.getElementById('rulesList');
	if (ruleList) {
		var rules = document.getElementById('rulesListChilds');
		var treeSelection = ruleList.view.selection;
		if (treeSelection.count > 0) {
			// insertBefore method will clear our selection, so we have to firstly record all our selections,
			// then use another loop to perform moving.
			var list = [];
			for (var i = 0; i < treeSelection.getRangeCount(); i++) {
				var start = {}, end = {};
				treeSelection.getRangeAt(i, start, end);
				for (var j = start.value; j <= end.value; j++) {
					list.push(j);
				}
			}
			for (var i = 0; i < list.length; i++) {
				var j = list[i];
				if ( j > 0 ) {
					rules.insertBefore(rules.childNodes[j], rules.childNodes[j-1]);
					ruleList.view.selection.toggleSelect(j-1);
				}
			}
		}
	}
}

function cmd_moveRulesDown() {
	var ruleList = document.getElementById('rulesList');
	if (ruleList) {
		var rules = document.getElementById('rulesListChilds');
		if (ruleList.view.selection.count > 0) {
			for (var i=rules.childNodes.length-1; i>=0 ; i--) {
				if (ruleList.view.selection.isSelected(i)) {
					// Moving rules down is relatively simpler, just call insertBefore will work
					if ( i < rules.childNodes.length-1 ) rules.insertBefore(rules.childNodes[i+1], rules.childNodes[i]);
				}
			}
		}
	}
}

function enableSelectedRule(enable) {
	var ruleList = document.getElementById('rulesList');
	if (ruleList) {
		var rules = document.getElementById('rulesListChilds');
		if (ruleList.view.selection.count > 0) {
			for (var i=rules.childNodes.length-1 ; i>=0 ; i--) {
				if (ruleList.view.selection.isSelected(i)) {
					rules.childNodes[i].firstChild.childNodes[1].setAttribute('value', enable);
				}
			}
		}
	}
}

var gIeTab = new IeTab();
