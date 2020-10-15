//this code iterates over all html <device>'s to prepare them for their desired actions

var intervals = new Array;

window.addEventListener("load", myInit, true);
function myInit(){ 
  load_devices();
  fix_device_name_width();
 };

function load_devices(){
  let devices = document.getElementsByTagName("device");
  for (let device of devices){
    let id     = device.attributes.getNamedItem("device_id");
    let action = device.attributes.getNamedItem("device_action");
    let delay  = device.attributes.getNamedItem("device_update_delay");
    let name   = device.attributes.getNamedItem("device_name");

    if (name == null) {
      let device_name = document.createElement("strong");
      device_name.classList.add("device_name");
      device_name.appendChild(document.createTextNode(id.value + "\xa0\xa0\xa0\xa0")) ; 
      device.appendChild(device_name);
      console.log("id was used insted of name!");
    }
    else{
      let device_name = document.createElement("strong");
      device_name.classList.add("device_name");
      device_name.appendChild(document.createTextNode(name.value + "\xa0\xa0\xa0\xa0"))  
      device.appendChild(device_name);
    }

    id.value = id.value.replace(/\ /g, "/");
    console.log("id after transformation: '" + id.value + "'");

    let device_delay = 500;
    if (delay != null) device_delay=Number(delay.value);

    let reading = document.createElement("span");
    if (action != null) switch (action.value) {
      case "r_d":
        reading.classList.add("reading");
        device.appendChild(reading);
        updateElement(reading, id.value , device_delay);
        console.log("update: '" + id.value + "' with delay: " + device_delay)
        break;
      case "r_a":
        reading.classList.add("reading");
        device.appendChild(reading);
        updateElement(reading, id.value+"/analog" , device_delay)
        console.log("update: '" + id.value+"/analog" + "' with delay: " + device_delay)
        break;

      case "r_s":
        reading.classList.add("reading");
        device.appendChild(reading);
        updateElement(reading, id.value+"/state" , device_delay);
        console.log("update: '" + id.value+"/state" + "' with delay: " + device_delay)
        break;

      case "ctr":
        let turn_on = document.createElement("a");
        turn_on.innerHTML = "Turn On";
        turn_on.classList.add("button");
        turn_on.onclick = function () {
          console.log("PUT " + id.value+"/high")
          httpGetAsync(id.value+"/high", null ,"PUT")
        }
        device.appendChild(turn_on);

        device.appendChild(document.createTextNode("\xa0\xa0\xa0\xa0"));

        let turn_off = document.createElement("a");
        turn_off.innerHTML = "Turn Off";
        turn_off.classList.add("button");
        turn_off.onclick = function () {
          console.log("PUT " + id.value+"/low")
          httpGetAsync(id.value+"/low", null ,"PUT")
        }
        device.appendChild(turn_off);
        break;
    
      default:
        break;
    }


  }
}

function fix_device_name_width(){
  let min_width=0;
  for (let device_name of document.getElementsByClassName("device_name")){
    min_width =  device_name.offsetWidth>min_width? device_name.offsetWidth : min_width; 
  }
  console.log(min_width);
  var sheet = document.createElement('style');
  sheet.innerHTML = ".device_name {display:inline-block; min-width : " + String(min_width) + "px;}";
  document.body.appendChild(sheet);
}

///////////////////////////////////////////////////////////////////////////////////////
function updateElement (element, request, interval = 500, method = "GET"){
  function loader1 (Http_response){
    element.innerHTML = "is " +  Http_response.responseText;
  }
  function loader1Fetch (){
    httpGetAsync(request, loader1, method);
  }
  intervals.push(setInterval(loader1Fetch,interval));
}

function httpGetAsync(theUrl, callback, method = "GET")
{
  var xmlHttp = new XMLHttpRequest();
  xmlHttp.onreadystatechange = function() { 
    if (xmlHttp.readyState == 4 && callback != null)
        callback(xmlHttp);
  }
  xmlHttp.open(method, theUrl, true); // true for asynchronous 
  xmlHttp.send(null);
}