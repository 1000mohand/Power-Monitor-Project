//this code is a template for http request using java"script"

//LED
updateElement("led-built-in_r", "LED");
updateElement("s1_r", "sensor/36");

///////////////////////////////////////////////////////////////////////////////////////
function updateElement (id, request, interval = 50){
  function loader1 (Http_response){
    document.getElementById(id).innerHTML = "is " + Http_response.responseText;
  }
  function loader1Fetch (){
    httpGetAsync(request, loader1);
  }
  setInterval(loader1Fetch,interval);
}

function httpGetAsync(theUrl, callback, request = "GET")
{
  var xmlHttp = new XMLHttpRequest();
  xmlHttp.onreadystatechange = function() { 
    if (xmlHttp.readyState == 4)
        callback(xmlHttp);
  }
  xmlHttp.open(request, theUrl, true); // true for asynchronous 
  xmlHttp.send(null);
}
