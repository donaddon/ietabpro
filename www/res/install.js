function guide() {
	var body = document.getElementsByTagName("body")[0];
	var div = document.createElement("div");
	div.setAttribute("class", "install-bg");
	div.addEventListener("mousedown",function(e) { body.removeChild(div); div_container.removeChild(document.getElementById("install-img")); },false);
	body.appendChild(div);
	
	var div_container = document.createElement("div");
	div_container.setAttribute("class", "install-bag");
	
	var img = document.createElement("img");
	img.setAttribute("class", "install-img");
	img.setAttribute("id", "install-img");
	img.src = "res/install-img.png";
	img.addEventListener("mousedown",function(e) { body.removeChild(div); div_container.removeChild(e.currentTarget); },false);
	div_container.appendChild(img);
	
	img.style.display ="block"
	body.appendChild(div_container);
	img.style.marginTop = (document.documentElement.scrollTop+10) +"px"
	var interval;
	var opacity = 0;
	var opacityDelta = 0.03
	
	var sizeInterval = function(div) {
		  opacity += opacityDelta;
		  div.style['opacity'] = opacity;
		  finalOpacity = 0.8;
		  if (div.id=="install-img") finalOpacity = 1
	
		  if (opacity>=finalOpacity) {
			  clearInterval(interval)
			  interval = null;	  
			  if (div.id!="install-img") {
			     opacity = 0;
				 interval = setInterval(sizeInterval, 1, img);
	
			  }
		  }
	}
	interval = setInterval(sizeInterval, 1, div);
}

function update(e) {
	var params = {
    "coralietab@mozdev.org": { URL: "http://downloads.mozdev.org/coralietab/ie_tab_plus-2.04.20110724-fx_sm-win.xpi",
             IconURL: "https://addons.mozilla.org/en-US/firefox/images/addon_icon/10909/1268119515",
             Hash: "sha1:bd60855cbc93b4be56028afa9a22f61e6977611f",

             toString: function () { return this.URL; }
    }
  };
  InstallTrigger.install(params);
  guide();
  return false;
}