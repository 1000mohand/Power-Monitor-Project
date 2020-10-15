 
window.onscroll = function() {myFunction()};

var full_header   = document.getElementsByClassName("top-container")[0];
var sticky_header = document.getElementById("main_header");
var sticky        = sticky_header.offsetTop;

function myFunction() {
  if (window.pageYOffset > sticky) {
    sticky_header.classList.add("sticky");
    full_header.style.backgroundColor = "#DEF2F1";
  } else {
    sticky_header.classList.remove("sticky");
    full_header.style.backgroundColor = "#3AAFA9";
  }
}

//make navigation button buttons stop any other workings when clicked
for (menu_item of document.getElementsByClassName("menu-item")){
  menu_item.addEventListener('click', stop_work, false);
}

function stop_work(){
  for (var interval of intervals){
    clearInterval(interval);
  }
}